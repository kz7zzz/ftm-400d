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

static Channel * decodeChannel(
    const unsigned char * c,
    const unsigned char * s,
	bool home = false)
{
    if (!home && !(c[0] & 0x80U)) return NULL; /* not programmed */

	auto_ptr<Channel> chn(new Channel);

    chn->name = data2str(s, Channel::STRING_SIZE);
    chn->skip = c[0] & 0x20U;
    chn->band = (c[0] & 0x07U) + 1;

	chn->mode = (c[1] & 0x10U) >> 4; // probably more bits here to consider

	chn->freq += 100 * 1000 * (c[2] & 0x0FU);
    chn->freq += 10 * 1000 * ((c[3] & 0xF0U)>>4);
    chn->freq += 1000 * (c[3] & 0x0FU);
    chn->freq += 100 * ((c[4] & 0xF0U)>>4);
    chn->freq += 10 * (c[4] & 0x0FU);
    if (c[2] & 0x80U) chn->freq += 5;

#if 0
    // not sure about this
    string def = " ";
    switch (c[5] & 0x03U) {
    case 0x06U: def = "T"; break;
    case 0x07U: def = "B"; break;
    }
#endif

	int offset = 600;
	if (chn->freq >= 400000) offset=5000;

    switch (c[1] & 0x03U) {
    case 0x02U: chn->offset = -offset; break;
    case 0x03U: chn->offset = +offset; break;
    }


	if (c[5] & 0x10U) {
        chn->tone = (c[9] & 0x1fU) + 1;
	}

	chn->dcs = c[10] & 0x0fU;
	chn->power = (c[9] & 0xc0U) >> 6;

	return chn.release();
}

void channel2xml(
	const Channel * chn)
{
    cout << TAB << "<channel";

	if (chn->cname.length()) {
		cout << " name=\"" << chn->cname << "\"";
	}

	if (chn->bank>0) {
		cout << " bank=\"" << chn->bank << "\"";
	}

	if (chn->slot>0) {
		cout << " slot=\"" << chn->slot << "\"";
	}

	cout << ">" << endl;

	if (chn->band) {
    	cout << TAB TAB << "<band>" << bands[chn->band] << "</band>" << endl;
	}

    cout << TAB TAB << "<frequency>" << chn->freq / 1000 << "."
		<< setfill('0') << setw(3) << chn->freq % 1000 << "</frequency>" << endl;

    if (chn->offset) {
		int offset = abs(chn->offset);
		cout << TAB TAB <<
			"<offset>" << (chn->offset > 0 ? "+" : "-")
				<< offset / 1000 << "."
				<< setfill('0') << setw(3) << offset % 1000 << "</offset>" << endl;
	}

    if (chn->tone) {
		cout << TAB TAB << "<tone>" << tones[chn->tone-1] << "</tone>" << endl;
	}

    cout << TAB TAB << "<mode>" << modes[chn->mode] << "</mode>" << endl;
    cout << TAB TAB << "<power>" << powers[chn->power] << "</power>" << endl;

    if (chn->dcs) {
		cout << TAB TAB << "<dcs>" << dcsCodes[chn->dcs] << "</dcs>" << endl;
	}

    if (chn->name.length()) {
		cout << TAB TAB << "<name>" << xmlsafe(chn->name) << "</name>" << endl;
	}

    if (chn->skip) {
		cout << TAB TAB << "<skip>true</skip>" << endl;
	}

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
    cout << "<channels xmlns=\"" SCHEMA_NS_URI "\"" << endl
	    << TAB "xmlns:ext=\"" SCHEMA_EXT_NS_URI "\"" << endl
	    << TAB "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"" << endl
	    << TAB "xsi:schemaLocation=\"" SCHEMA_NS_URI " " SCHEMA_LOC_URI "\"" ">" << endl;

    for (size_t i=0; i<Channel::NCHANNELS; ++i) {
	    unsigned char * d = &data[Channel::CHANNEL_TOP_OFFSET + (i * Channel::CHANNEL_SIZE)];
	    unsigned char * s = &data[Channel::CHANNEL_TOP_STRING_OFFSET + (i * Channel::STRING_SIZE)];
	    auto_ptr<Channel> chn(decodeChannel(d, s));
		if (chn.get()) {
			chn->bank = 1;
			chn->slot = i+1;
			channel2xml(chn.get());
		}
    }

    for (size_t i=0; i<Channel::NCHANNELS; ++i) { // bottom
	    unsigned char * d = &data[Channel::CHANNEL_BOT_OFFSET + (i * Channel::CHANNEL_SIZE)];
	    unsigned char * s = &data[Channel::CHANNEL_BOT_STRING_OFFSET + (i * Channel::STRING_SIZE)];
	    auto_ptr<Channel> chn(decodeChannel(d, s));
		if (chn.get()) {
			chn->bank = 2;
			chn->slot = i+1;
		    channel2xml(chn.get());
		}
    }

    {
	    unsigned char * d = &data[Channel::HOME_OFFSET];
	    unsigned char * s = &data[Channel::HOME_STRING_OFFSET];
	    auto_ptr<Channel> chn(decodeChannel(d, s));
		if (chn.get()) {
			chn->cname = "Home";
			channel2xml(chn.get());
		}
    }

    for (size_t i=0; i<Channel::NPCHANNELS; ++i) {
	    unsigned char * d = &data[Channel::PCHANNEL_OFFSET + (i * Channel::CHANNEL_SIZE)];
	    unsigned char * s = &data[Channel::PCHANNEL_STRING_OFFSET + (i * Channel::STRING_SIZE)];
	    auto_ptr<Channel> chn(decodeChannel(d, s));
		if (chn.get()) {
			chn->cname = pchannels[i];
			channel2xml(chn.get());
		}
	}

    cout  << "</channels>" << endl;
	return EXIT_SUCCESS;
}
