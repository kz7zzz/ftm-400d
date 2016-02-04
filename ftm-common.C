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

#include <memory>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>

#include <cstdlib>
#include <cstring>
#include <cassert>

#include "ftm.h"

using namespace std;

const char * sqls[] = {
	"noise", "T-TX", "T-TRX", "T-REV",
	"D-TRX", "program", "pager", "D-TX",
	"TT/DR", "DT/TR", NULL, NULL,
	NULL, NULL, NULL, NULL,
};

const char * scans[] = {
	"", "skip", "select", "unknown", NULL
};

// symbols supported.  (note: # is used as a fill character here (not the radio))
const unsigned char d2a[] =
	"\0" "1234567"	   "89ABCDEF"
	"GHIJKLMN"		   "OPQRSTUV"
	"WXYZabcd"		   "efghijkl"
	"mnopqrst"		   "uvwxyz!\""

	"#$%&'()*"		   "+,-./:;<"
	"=>?@[\\]^"		   "_`{|}~\x82\x83"
	"\x84\x85\x86\x87 \0\0\0"
					   "\0\0\0\0\0\0\0\0"
	"\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0"

	"\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0"
	"\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0"
	"\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0"
	"\0\0\0\0\0\0\0\x80\x81\0\0\0\0\0\0" "0"

	"\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0"
	"\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0"
	"\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0"
	"\0\0\0\0\0\0\0\0" "\0\0\0\0\0\0\0\0";

const char * utf8[] = {
	"\u00A5",   // Yen symbol
	"\u2219",	// Dot
	"\u2191",	// UPWARDS ARROW
	"\u2193",	// DOWNWARDS ARROW
	"\u2192",	// RIGHTWARDS ARROW
	"\u2190",	// LEFTWARDS ARROW
	"\u00B1",	// PLUS-MINUS SIGN
	"\u266A",	// EIGHTH NOTE
	NULL
};

const unsigned char utf8data[] = {
	0xb7U, 0xb8U, 0x5eU, 0x5fU, 0x60U, 0x61U, 0x62U, 0x63U
};

const unsigned char a2d[] = {
	   0,   0,   0,   0,   0,   0,   0,   0,	0,   0,   0,   0,   0,   0,   0,   0,
	   0,   0,   0,   0,   0,   0,   0,   0,	0,   0,   0,   0,   0,   0,   0,   0,
	0x64,0x3e,0x3f,0x40,0x41,0x42,0x43,0x44, 0x45,0x46,0x47,0x48,0x49,0x4a,0x4b,0x4c,
	0xbf,0x01,0x02,0x03,0x04,0x05,0x06,0x07, 0x08,0x09,0x4d,0x4e,0x4f,0x50,0x51,0x52,

	0x53,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10, 0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,
	0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,0x20, 0x21,0x22,0x23,0x54,0x55,0x56,0x57,0x58,
	0x59,0x24,0x25,0x26,0x27,0x28,0x29,0x2a, 0x2b,0x2c,0x2d,0x2e,0x2f,0x30,0x31,0x32,
	0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3a, 0x3b,0x3c,0x3d,0x5a,0x5b,0x5c,0x5d,   0,

	0xb7,0xb8,0x5e,0x5f,0x60,0x61,0x62,0x63,	0,   0,   0,   0,   0,   0,   0,   0,
	   0,   0,   0,   0,   0,   0,   0,   0,	0,   0,   0,   0,   0,   0,   0,   0,
	   0,   0,   0,   0,   0,   0,   0,   0,	0,   0,   0,   0,   0,   0,   0,   0,
	   0,   0,   0,   0,   0,   0,   0,   0,	0,   0,   0,   0,   0,   0,   0,   0,

	   0,   0,   0,   0,   0,   0,   0,   0,	0,   0,   0,   0,   0,   0,   0,   0,
	   0,   0,   0,   0,   0,   0,   0,   0,	0,   0,   0,   0,   0,   0,   0,   0,
	   0,   0,   0,   0,   0,   0,   0,   0,	0,   0,   0,   0,   0,   0,   0,   0,
	   0,   0,   0,   0,   0,   0,   0,   0,	0,   0,   0,   0,   0,   0,   0,   0,
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

const char * modes[] = {
	"auto", "AM", "FM", "narrow FM"
};

const char * powers[] = {
	"high", "medium", "low", "unknown"
};

const char * bands[] = {
	"", "Air", "VHF", "Gen1", "UHF", "Gen2", "Unknown", "Unknown", "Unknown", NULL
};

const char * pchannels[] = {
	"P1U", "P1L", "P2U", "P2L", "P3U", "P3L", "P4U", "P4L", "P5U", "P5L",
	"P6U", "P6L", "P7U", "P7L", "P8U", "P8L", "P9U", "P9L"
};

string int2str(int i) {
	ostringstream os;
	os << i;
	return os.str();
}

string int2hex(unsigned i) {
	ostringstream os;
	os << std::hex << i;
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
		unsigned char d = s[i];
		if (d == Channel::TAG_FILL) {
			break;

		} else {
			unsigned a = d2a[d];

			if (!a) {
				str += "[0x" + int2hex(d) + "]";

			} else if (a < 128) {
				str.append(1, a);

			} else {
				str.append(utf8[a - 128]);
			}
		}
	}

	return str;
}

int utf8len(const char * s) {
	const unsigned char * u = reinterpret_cast<const unsigned char *>(s);
	
	if (*u < 0x80) return 1;

	int i;
	for (i=1; i<6; i++) {
		if ((u[i] & 0xc0U) != 0x80U) break;
	}

	return i;
}

size_t str2data(const std::string & str, unsigned char * sbuf) {
	size_t len;
	size_t j=0;
	for (int i=0; i < str.length(); i+=len) {
		unsigned char a = str[i];
		len = 1;

		if (a < 128) {
			sbuf[j++] = a2d[a];

		} else {
			len = utf8len(&str.c_str()[i]);
			int x;
			for (x=0; utf8[x]; x++) {
				int ulen = strlen(utf8[x]);
				if (len == ulen && !memcmp(&str.c_str()[i], utf8[x], len)) {
					break;
				}
			}
			if (utf8[x]) {
				sbuf[j++] = utf8data[x];

			} else {
				sbuf[j++] = a2d['?']; // unrecognized
			}
		}
	}
	return j;
}

