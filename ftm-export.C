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

void dumpChannel(
    size_t bank,
    size_t slot,
    const unsigned char * c,
    const unsigned char * s,
	bool home = false)
{
    string station = data2str(s, Channel::STRING_SIZE);

    if (!home && !(c[0] & 0x80U)) return; /* not programmed */

    bool skip = c[0] & 0x20U;

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
    if (skip) cout << TAB TAB << "<skip>true</skip>" << endl;
    cout << TAB << "</channel>" << endl;
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
		cerr << "expected one argument" << endl;
		cerr << "usage: ftm-export MEMFTM400D.dat" << endl;
		return EXIT_FAILURE;
	}

    const char * datafile = argv[1];

    ifstream is;
    off_t datalen;

    is.open(datafile, ios::binary);
    if (!is.is_open()) {
	    cerr << "Failed to open " << datafile << " for input." << endl;
	    return EXIT_FAILURE;
    }

    is.seekg(0, ios::end);
    datalen = is.tellg();
    vector<unsigned char> data(datalen);

    is.seekg(0, ios::beg);
    is.read(reinterpret_cast<char *>(&data[0]), datalen);
    is.close();

    if (datalen != 25600) {
	    cerr << "Bad data length: " << datalen << endl;
		return EXIT_FAILURE;
    }

    cout << XML_DECL << endl;
    cout << "<channels xmlns=\"" CHANNEL_NS_URI "\"" << endl
	    << TAB "xmlns:ext=\"http://boolean.net/ftm400dr/ext/0\"" << endl
	    << TAB "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"" << endl
	    << TAB "xsi:schemaLocation=\"http://boolean.net/ftm400dr/0 channels.xsd\"" ">" << endl;

    for (size_t i=0; i<Channel::NCHANNELS; ++i) {
	    unsigned char * d = &data[Channel::CHANNEL_TOP_OFFSET + (i * Channel::CHANNEL_SIZE)];
	    unsigned char * s = &data[Channel::CHANNEL_TOP_STRING_OFFSET + (i * Channel::STRING_SIZE)];
	    dumpChannel(1, i+1, d, s);
    }

    for (size_t i=0; i<Channel::NCHANNELS; ++i) { // bottom
	    unsigned char * d = &data[Channel::CHANNEL_BOT_OFFSET + (i * Channel::CHANNEL_SIZE)];
	    unsigned char * s = &data[Channel::CHANNEL_BOT_STRING_OFFSET + (i * Channel::STRING_SIZE)];
	    dumpChannel(2, i+1, d, s);
    }

    for (size_t i=0; i<2; ++i) {
	    unsigned char * d = &data[Channel::HOME_OFFSET + (i * Channel::CHANNEL_SIZE)];
	    unsigned char * s = &data[Channel::HOME_STRING_OFFSET + (i * Channel::STRING_SIZE)];
	    dumpChannel(3, i+1, d, s);
    }

    cout  << "</channels>" << endl;
	return EXIT_SUCCESS;
}
