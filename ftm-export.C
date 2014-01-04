// Copyright 2014, Kurt Zeilenga. All rights reserved.
//
// Permission to use, copy, modify, and/or distribute this software
// for any purpose with or without fee is hereby granted, provided
// that the above copyright notice and this permission notice appear
// in all copies.
//
// Provided "as is" without warranty of any kind.

//
// Yaesu FTM 400dr memory dump to XML program
//

#include <stdlib.h>
#include <memory>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>

#include "ftm.h"

using namespace std;

// symbols supported.  (note: # is used as a fill character here (not the radio))
static const char d2a[] =
    "0123456789ABCDEF"
    "GHIJKLMNOPQRSTUV"
    "WXYZabcdefghijkl"
    "mnopqrstuvwxyz|\""
    "#$#&'()*,-./:;##"
    "##?@############"
    "#### ###########";

static const char * tones[] = {
    "67.0", "69.3", "71.9", "74.4", "77.0", "79.7", "82.5", "85.4",
    "88.5", "91.5", "94.8", "97.4", "100.0", "103.5", "107.2", "110.9",

    "114.8", "118.8", "123.0", "127.3", "131.8", "136.5", "141.3", "146.2",
    "151.4", "156.7", "162.2", "167.9", "173.8", "179.9", "186.2", "192.8",

    "203.5", "206.5", "210.7", "218.1", "225.7", "229.1", "233.6", "241.8", 
    "250.3", "254.1"
};

static const char * dcsCodes[] = {
    NULL /* no DCS */,
    "023", "025", "026", "031", "032", "043", "047", "051",
    "054", "065", "071", "072", "073", "074", "114", "115",
    "116", "125", "131", "132", "134", "143", "152", "155",
    "156", "162", "165", "172", "174", "205", "223", "226",
    "243", "244", "245", "251", "261", "263", "265", "271",
    "306", "311", "315", "331", "343", "345", "351", "364",
    "365", "371", "411", "412", "413", "423", "431", "432",
    "445", "464", "465", "466", "503", "506", "516", "532",
    "546", "565", "606", "612", "624", "627", "631", "632",
    "654", "662", "664", "703", "712", "723", "731", "732",
    "734", "743", "754"
};

string int2str(int i) {
    ostringstream os;
    os << i;
    return os.str();
}

string data2str(
    const unsigned char * s,
    size_t len)
{
    string str;

    for (size_t i=0; i<len; ++i) {
	if (s[i] == STRING_FILL) {
	    break;

	} else if (s[i] == STRING_ZERO) {
	    str += "0";

	} else if (s[i] < sizeof(d2a)) {
	    str.append(1, d2a[s[i]]);

	} else {
	    str += "[" + int2str(s[i]) + "]";
	}
    }

    return str;
}

string xmlsafe(const string in) {
    string out;
    out.reserve(in.length());

    for (size_t i=0; i<in.length(); i++) {
        switch(in[i]) {
	case '&': // must be escaped
	   out.append("&amp;");
	   break;
	   
	case '<': // must be escaped
	   out.append("&lt;");
	   break;

	case '>': // should be escaped
	   out.append("&gt;");
	   break;

	default:
	    out.append(1, in[i]);
	}
    }
    return out;
}

void dumpChannel(
    size_t bank,
    size_t slot,
    const unsigned char * c,
    const unsigned char * s)
{
    string station = data2str(s, STRING_SIZE);

    if (!(c[0] & 0x80U)) return; /* not programmed */

    bool unknown = c[0] & 0x20U;

    string band;
    switch (c[0] & 0x07U) {
    case 0x00U: band = "VHF+"; break;
    case 0x01U: band = "VHF"; break;
    case 0x02U: band = "G1"; break;
    case 0x03U: band = "UHF"; break;
    case 0x04U: band = "G2"; break;
    case 0x05U: band = "U1"; break;
    case 0x06U: band = "U2"; break;
    case 0x07U: band = "U3"; break;
    }

    string offset;
    switch (c[1] & 0x03U) {
    case 0x01U: offset = "unknown"; break;
    case 0x02U: offset = "-"; break;
    case 0x03U: offset = "+"; break;
    }

    string mode = "FM";
    switch (c[1] & 0x10U) { // probably more bits here to consider
    case 0x10U: mode= "AM"; break;
    }

    unsigned int mhz = 0;
    mhz += 100 * (c[2] & 0x0FU);
    mhz += 10 * ((c[3] & 0xF0U)>>4);
    mhz += c[3] & 0x0FU;

    unsigned int khz = 0;
    khz += 100 * ((c[4] & 0xF0U)>>4);
    khz += 10 * (c[4] & 0x0FU);
    if (c[2] & 0x80U) khz += 5;

    // not sure about this
    string def = " ";
    switch (c[5] & 0x03U) {
    case 0x06U: def = "T"; break;
    case 0x07U: def = "B"; break;
    }

    bool hasTone = c[5] & 0x10U;
    unsigned int tone = c[9] & 0x1fU;

    string power("high");
    switch (c[9] & 0xc0U) {
    case 0x80U: power="low"; break;
    case 0x40U: power="medium"; break;
    case 0xc0U: power="unknown"; break;
    }
    
    unsigned int dcs = c[10] & 0x0fU;

    //
    // TODO: separate datafile decoding from XML encoding
    //
    cout << TAB << "<channel bank=\"" << bank <<
		"\" slot=\"" << slot << "\">" << endl;
    cout << TAB TAB << "<band>" << band << "</band>" << endl;
    cout << TAB TAB << "<frequency>" << mhz << "." << khz << "</frequency>" << endl;
    if (offset.length()) cout << TAB TAB << "<offset>" << offset << "</offset>" << endl;
    if (hasTone) cout << TAB TAB << "<tone>" << tones[tone] << "</tone>" << endl;
    cout << TAB TAB << "<mode>" << mode << "</mode>" << endl;
    cout << TAB TAB << "<power>" << power << "</power>" << endl;
    if (dcs) cout << TAB TAB << "<dcs>" << dcsCodes[dcs] << "</dcs>" << endl;
    if (station.length()) cout << TAB TAB << "<name>" << xmlsafe(station) << "</name>" << endl;
    if (unknown) cout << TAB TAB << "<unknown>true</unknown>" << endl;
    cout << TAB << "</channel>" << endl;
}

int main(int argc, char *argv[])
{
    const char * datafile = argv[1];

    ifstream is;
    off_t datalen;

    is.open(datafile, ios::binary);
    if (!is.is_open()) {
	    cerr << "Failed to open " << datafile << " for input." << endl;
	    exit(1);
    }

    is.seekg(0, ios::end);
    datalen = is.tellg();
    vector<unsigned char> data(datalen);

    is.seekg(0, ios::beg);
    is.read(reinterpret_cast<char *>(&data[0]), datalen);
    is.close();

    if (datalen != 25600) {
	    cerr << "Bad data length: " << datalen << endl;
	    // press on anyways (might crash if short)
    }

    cout << XML_DECL << endl;
    cout << "<channels xmlns=\"" CHANNEL_NS_URI "\"" << endl
	    << TAB "xmlns:ext=\"http://boolean.net/ftm400dr/ext/0\"" << endl
	    << TAB "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"" << endl
	    << TAB "xsi:schemaLocation=\"http://boolean.net/ftm400dr/0 channels.xsd\"" ">" << endl;

    for (size_t i=0; i<NCHANNELS; ++i) {
	    unsigned char * d = &data[CHANNEL_TOP_OFFSET + (i * CHANNEL_SIZE)];
	    unsigned char * s = &data[STRING_TOP_OFFSET + (i * STRING_SIZE)];
	    dumpChannel(1, i+1, d, s);
    }

    for (size_t i=0; i<NCHANNELS; ++i) { // bottom
	    unsigned char * d = &data[CHANNEL_BOT_OFFSET + (i * CHANNEL_SIZE)];
	    unsigned char * s = &data[STRING_BOT_OFFSET + (i * STRING_SIZE)];
	    dumpChannel(2, i+1, d, s);
    }

    cout  << "</channels>" << endl;
}
