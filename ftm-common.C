// Copyright 2014, Kurt Zeilenga. All rights reserved.
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

#include <stdlib.h>
#include <memory>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <assert.h>

#include "ftm.h"

using namespace std;

// symbols supported.  (note: # is used as a fill character here (not the radio))
const char d2a[] =
	"0123456789ABCDEF"
	"GHIJKLMNOPQRSTUV"
	"WXYZabcdefghijkl"
	"mnopqrstuvwxyz|\""

	"#$#&'()*,-./:;##"
	"##?@############"
	"#### ###########"
	"################"

	"################"
	"################"
	"################"
	"###############0"

	"################"
	"################"
	"################"
	"################";

const unsigned char a2d[] = {
	 0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,

   100,  0, 63,  0, 65,  0, 67, 68,
	69, 70, 71,  0, 72, 73, 74, 75,
   191,  1,  2,  3,  4,  5,  6,  7,
	 8,  9, 76, 77,  0,  0,  0, 82,

	83, 10, 11, 12, 13, 14, 15, 16,
	17, 18, 19, 20, 21, 22, 23, 24,
	25, 26, 27, 28, 29, 30, 31, 32,
	33, 34, 35, 0, 0, 0, 0, 0,

	 0, 36, 37, 38, 39, 40, 41, 42,
	43, 44, 45, 46, 47, 48, 49, 50,
	51, 52, 53, 54, 55, 56, 57, 58,
	59, 60, 61,  0, 62,  0,  0,  0,

	 0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,

	 0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,

	 0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,

	 0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0,
	 0,  0,  0,  0,  0,  0,  0,  0
};

const char * tones[] = {
	"67.0", "69.3", "71.9", "74.4", "77.0", "79.7", "82.5", "85.4",
	"88.5", "91.5", "94.8", "97.4", "100.0", "103.5", "107.2", "110.9",

	"114.8", "118.8", "123.0", "127.3", "131.8", "136.5", "141.3", "146.2",
	"151.4", "156.7", "162.2", "167.9", "173.8", "179.9", "186.2", "192.8",

	"203.5", "206.5", "210.7", "218.1", "225.7", "229.1", "233.6", "241.8", 
	"250.3", "254.1"
};

const char * dcsCodes[] = {
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
	"734", "743", "754", NULL
};

string int2str(int i) {
	ostringstream os;
	os << i;
	return os.str();
}

string xmlsafe(const string & in) {
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

string data2str(
	const unsigned char * s,
	size_t len)
{
	string str;

	for (size_t i=0; i<len; ++i) {
	    if (s[i] == Channel::STRING_FILL) {
		    break;

		} else if (s[i] != '#') {
			str.append(1, d2a[s[i]]);

		} else {
			str += "[" + int2str(s[i]) + "]";
		}
	}

	return str;
}

void str2data(const std::string & str, unsigned char * sbuf) {
	for (int i=0; i < str.length(); i++) {
		sbuf[i] = a2d[str[i]];
		assert(sbuf[i]);

		if (d2a[sbuf[i]] != str[i]) {
			cerr << "Error: '" << str[i]
				<< "' -> " << (int) a2d[str[i]]
				<< " -> '" << d2a[a2d[str[i]]] << "'" << endl;
		}

		assert(d2a[sbuf[i]] == str[i]);
	}
}

