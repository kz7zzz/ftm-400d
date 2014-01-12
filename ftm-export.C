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

	chn->tag = data2str(s, Channel::TAG_SIZE);
	chn->scan = (c[0] & 0x60U) >> 5;

	chn->band = (c[0] & 0x07U) + 1;

	chn->mode = (c[1] & 0x70U) >> 4;

	chn->rx += 100 * 1000 * (c[2] & 0x0fU);
	chn->rx += 10 * 1000 * ((c[3] & 0xf0U)>>4);
	chn->rx += 1000 * (c[3] & 0x0fU);
	chn->rx += 100 * ((c[4] & 0xf0U)>>4);
	chn->rx += 10 * (c[4] & 0x0fU);
	if (c[2] & 0x80U) chn->rx += 5;

	switch (c[1] & 0x07U) {
	case 0x02U: chn->offset = -1; break;
	case 0x03U: chn->offset = +1; break;
	case 0x04U: // separate transmit frequency
	    chn->tx += 100 * 1000 * (c[6] & 0x0fU);
	    chn->tx += 10 * 1000 * ((c[7] & 0xf0U)>>4);
	    chn->tx += 1000 * (c[7] & 0x0fU);
	    chn->tx += 100 * ((c[8] & 0xf0U)>>4);
	    chn->tx += 10 * (c[8] & 0x0fU);
	    if (c[6] & 0x80U) chn->tx += 5;
	}

	// c[5] & 0x0FU ?
	chn->sql = (c[5] & 0xf0U) >> 4;
	chn->tone = c[9] & 0x1fU;
	chn->dcs = c[10] & 0x0fU;

	// c[11] & 0x80U seems only set for bank 1
	// c[11] & 0x0fU seems always set
	// c[12]/c[13] is offset size in 50mhz steps (see ftm-import.C)

	chn->power = (c[9] & 0xc0U) >> 6;

	return chn.release();
}

void channel2xml(
	const Channel * chn)
{
	cout << TAB << "<channel";

	if (chn->memname.length()) {
		cout << " name=\"" << chn->memname << "\"";
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

	cout << TAB TAB << "<frequency>" << chn->rx / 1000 << "."
		 << setfill('0') << setw(3) << chn->rx % 1000 << "</frequency>" << endl;

	if (chn->tx) {
		cout << TAB TAB << "<txFrequency>" << chn->tx / 1000 << "."
			<< setfill('0') << setw(3) << chn->tx % 1000 << "</txFrequency>" << endl;

	} else if (chn->offset) {
		if (chn->offset > 0) {
			cout << TAB TAB << "<offset>+</offset>" << endl;
		} else {
			cout << TAB TAB << "<offset>-</offset>" << endl;
		}
	}

	if (chn->sql) {
	    cout << TAB TAB << "<sql>" << sqls[chn->sql] << "</sql>" << endl;
	    cout << TAB TAB << "<tone>" << tones[chn->tone] << "</tone>" << endl;
	}

	if (chn->dcs) {
		cout << TAB TAB << "<dcs>" << dcsCodes[chn->dcs] << "</dcs>" << endl;
	}

	if (chn->mode) {
	    cout << TAB TAB << "<mode>" << modes[chn->mode] << "</mode>" << endl;
    }

	cout << TAB TAB << "<power>" << powers[chn->power] << "</power>" << endl;


	if (chn->tag.length()) {
		std::string safe(xmlsafe(chn->tag));
		cout << TAB TAB << "<tag>" << safe << "</tag>" << endl;
	}

	if (chn->scan) {
		cout << TAB TAB << "<scan>" << scans[chn->scan] << "</scan>" << endl;
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

	// Home channel
	{
		unsigned char * d = &data[Channel::HOME_OFFSET];
		unsigned char * s = &data[Channel::HOME_TAG_OFFSET];
		auto_ptr<Channel> chn(decodeChannel(d, s));
		if (chn.get()) {
			chn->memname = "Home";
			channel2xml(chn.get());
		}
	}

	// Bank 1
	for (size_t i=0; i<Channel::NCHANNELS; ++i) {
		unsigned char * d = &data[Channel::CHANNEL_TOP_OFFSET + (i * Channel::CHANNEL_SIZE)];
		unsigned char * s = &data[Channel::CHANNEL_TOP_TAG_OFFSET + (i * Channel::TAG_SIZE)];
		auto_ptr<Channel> chn(decodeChannel(d, s));
		if (chn.get()) {
			chn->bank = 1;
			chn->slot = i+1;
			channel2xml(chn.get());
		}
	}

	// Bank 2
	for (size_t i=0; i<Channel::NCHANNELS; ++i) { // bottom
		unsigned char * d = &data[Channel::CHANNEL_BOT_OFFSET + (i * Channel::CHANNEL_SIZE)];
		unsigned char * s = &data[Channel::CHANNEL_BOT_TAG_OFFSET + (i * Channel::TAG_SIZE)];
		auto_ptr<Channel> chn(decodeChannel(d, s));
		if (chn.get()) {
			chn->bank = 2;
			chn->slot = i+1;
			channel2xml(chn.get());
		}
	}

	// Programmable memory channels (for scanning)
	for (size_t i=0; i<Channel::NPCHANNELS; ++i) {
		unsigned char * d = &data[Channel::PCHANNEL_OFFSET + (i * Channel::CHANNEL_SIZE)];
		unsigned char * s = &data[Channel::PCHANNEL_TAG_OFFSET + (i * Channel::TAG_SIZE)];
		auto_ptr<Channel> chn(decodeChannel(d, s));
		if (chn.get()) {
			chn->memname = pchannels[i];
			channel2xml(chn.get());
		}
	}

	cout  << "</channels>" << endl;
	return EXIT_SUCCESS;
}
