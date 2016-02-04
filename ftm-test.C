// Copyright 2014-2016, Kurt Zeilenga. All rights reserved.
//
// Permission to use, copy, modify, and/or distribute this software
// for any purpose with or without fee is hereby granted, provided
// that the above copyright notice and this permission notice appear
// in all copies.
//
// Provided "as is" without warranty of any kind.

//
// Functional tests
//

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

using namespace std;

#include "ftm.h"

int main(int argc, char *argv[])
{
	unsigned x;

	for (x = 0; x < 256; x++) {
		if (a2d[x] && (d2a[a2d[x]] != x)) {
			cerr << "ASCII: 0x" << std::hex << x
				<< " -> YCHAR 0x" << (unsigned)a2d[x]
				<< " reversed to 0x" << (unsigned)d2a[a2d[x]] << endl;
		}
	}

	for (x = 0; x < 256; x++) {
		if (d2a[x] && (a2d[d2a[x]] != x)) {
			cerr << "YCHAR: 0x" << std::hex << x
				<< " -> ASCII 0x" << (unsigned)d2a[x]
				<< " reversed to 0x" << (unsigned)a2d[d2a[x]] << endl;
		}
	}

	std::string str(
		"abcdefghijklmnopqrstuvwxyz"
		"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
		"`0123456789-=[]\\;',./"
		"~!@#$%^&*()_+{}|:\"<>?"
		"\u2191" "\u2193" "\u2192" "\u2190" "\u00B1" "\u266a"
		" "
		"\u00A5" "\u2219"
	);

	const unsigned char data[] = 
		"\x24\x25\x26\x27\x28\x29\x2a\x2b\x2c\x2d\x2e\x2f\x30\x31\x32\x33"
		"\x34\x35\x36\x37\x38\x39\x3a\x3b\x3c\x3d\x0a\x0b\x0c\x0d\x0e\x0f"
		"\x10\x11\x12\x13\x14\x15\x16\x17\x18\x19\x1a\x1b\x1c\x1d\x1e\x1f"
		"\x20\x21\x22\x23\x59\xbf\x01\x02\x03\x04\x05\x06\x07\x08\x09\x4a"
		"\x50\x54\x56\x55\x4e\x44\x49\x4b\x4c\x5d\x3e\x53\x40\x41\x42\x57"
		"\x43\x47\x45\x46\x58\x48\x5a\x5c\x5b\x4d\x3f\x4f\x51\x52"
		"\x5e" "\x5f" "\x60" "\x61" "\x62" "\x63"
		"\x64"
		"\xb7" "\xb8"
		;

	size_t datalen=sizeof(data)-1;

	vector<unsigned char> dataOut(datalen);
	size_t len = str2data(str, &dataOut[0]);

	if (len != datalen) {
		cerr << "str2data: expected " << datalen << " got " << len << endl;

	} else if (memcmp(data, dataOut.data(), len)) {
		cerr << "str2data: mismatch" << endl;
		for(int i=0; i < len; i++) {
			cout << hex << setw(2) << setfill('0') << (unsigned) dataOut[i];
		}
		cout << endl;
	}

	std::string strout(data2str(data, datalen));
	if (strout.length() != str.length()) {
		cerr << "data2str: expected " << str.length()<< " got " << strout.length() << endl;
	} else if (str != strout) {
		cerr << "data2str: mismatch" << endl;
	}
}
