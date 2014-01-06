// Copyright 2014, Kurt Zeilenga. All rights reserved.
//
// Permission to use, copy, modify, and/or distribute this software
// for any purpose with or without fee is hereby granted, provided
// that the above copyright notice and this permission notice appear
// in all copies.
//
// Provided "as is" without warranty of any kind.

//
// Yaesu FTM 400dr memory sting dump
//

#include <stdlib.h>
#include <memory>
#include <iostream>
#include <fstream>
#include <vector>

#include "ftm.h"

using namespace std;

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
	std::vector<unsigned char> data(datalen);

	is.seekg(0, ios::beg);
	is.read(reinterpret_cast<char *>(&data[0]), datalen);
	is.close();

	if (datalen != 25600) {
		cout << "Bad data len: " << datalen << endl;
	}

	ssize_t pos = -1;
	std::string s;

	// strings
	for (size_t i=0; i<datalen; ++i) {
		unsigned char c = data[i];
		if ((i < 0x0200 || i >= 0x42B0) && (d2a[c] == c || c < 127)) {
			s.append(1, d2a[c]);

		} else {
			if (s.length() > 2) {
				cout << std::hex << pos+1;
				cout << ": " << s << endl;
			}
			s.erase();
			pos = i;
		}
	}

	if (s.length() > 3) {
		cout << std::hex << pos+1;
		cout << ": " << s << endl;
	}
}
