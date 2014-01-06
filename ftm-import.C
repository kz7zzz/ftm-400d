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
#include <memory>

#include <libxml/parser.h>
#include <libxml/tree.h>

#include <assert.h>
#include <string.h>

#include "ftm.h"

using namespace std;

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
    "734", "743", "754", NULL
};

// symbols supported.  (note: # is used as a fill character here (not the radio))
static const char d2a[] =
    "0123456789ABCDEF"
    "GHIJKLMNOPQRSTUV"
    "WXYZabcdefghijkl"
    "mnopqrstuvwxyz|\""
    "#$#&'()*,-./:;##"
    "##?@############"
    "#### ###########";

static const unsigned char a2d[] = {
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

static void str2data(const std::string & str, unsigned char * sbuf) {
	for (int i=0; i < str.length(); i++) {
		sbuf[i] = a2d[str[i]];
		assert(sbuf[i]);

		if (d2a[sbuf[i]] != str[i]) {
			cerr << "Error: '" << str[i] << "' -> " << (int) a2d[str[i]] << " -> '" << d2a[a2d[str[i]]] << "'" << endl;
		}

		assert(d2a[sbuf[i]] == str[i]);
	}
}

static void encodeChannel(const Channel * c, unsigned char * dbuf, unsigned char * sbuf) {
	memset(sbuf, STRING_FILL, STRING_SIZE);
	memset(dbuf, 0, CHANNEL_SIZE);

	dbuf[5]  = 0x07U;
	dbuf[11] = (c->bank < 2) ? 0x8fU : 0x0fU;
	dbuf[13] = 0x64U;

	if (!c) return;

    str2data(c->name, sbuf);

	if (c->unknown) {
		dbuf[0] |= 0x20U;
	}

	if (c->band) {
		dbuf[0] |= 0x07U & (c->band-1);

	} else if (c->freq > 400*1000) {
		dbuf[0] |= 0x03U; /* UHF */

	} else {
		dbuf[0] |= 0x01U; /* VHF */
	}

	unsigned x = c->freq;
	unsigned rem = x % 10;
	if (rem >=  5) {
		dbuf[2] |= 0x80U;		// 5Hz
	}
	x /= 10;
	rem = x % 10;
	dbuf[4] |= 0x0F & rem;		// 10Hz
	x /= 10;
	rem = x % 10;
	dbuf[4] |= 0xF0 & (rem<<4); // 100Hz
	x /= 10;
	rem = x % 10;
	dbuf[3] |= 0x0F & rem;		// Mhz
	x /= 10;
	rem = x % 10;
	dbuf[3] |= 0xF0 & (rem<<4); // 10Mhz
	x /= 10;
	rem = x % 10;
	dbuf[2] |= 0x0F & rem;		// 100Mhz

	if (c->offset > 0) {
		dbuf[1] |= 0x03U; // +
	} else if (c->offset < 0) {
		dbuf[1] |= 0x02U; // -
	}

	if (c->mode) {
		dbuf[1] |= 0x10U; /* AM */
	}
	/* else FM */

	if (c->tone) {
		dbuf[5] |= 0x10U;
		dbuf[9] |= 0x1fU & (c->tone-1);
	}

	if (c->dcs) {
		dbuf[10] |= 0x1fU & c->dcs;
	}

	if (c->power == 1) {
		dbuf[9] |= 0x40U; /* medium */
	} else if (c->power) {
		dbuf[9] |= 0x80U; /* low */
	}
	/* else high */

	dbuf[0] |= 0x80U; // programmed 
}
 
static Channel * parseChannel(xmlDoc * doc, xmlNs * ns, xmlNode * node) {
	auto_ptr<Channel> c(new Channel);
	const char * str;

	str = (const char *) xmlGetProp(node, (const xmlChar *)"bank");
	if (str) {
		c->bank = strtol(str, NULL, 10);
	}

	str = (const char *) xmlGetProp(node, (const xmlChar *)"slot");
	if (str) {
		c->slot = strtol(str, NULL, 10);
	}

	cout << "Channel: " << c->bank << "/" << c->slot << endl;

	for (xmlNode * cur = node->xmlChildrenNode; cur; cur = cur->next) {
	    if (cur->type != XML_ELEMENT_NODE) {
			continue;
		}
		if (cur->ns != ns) {
			cerr << TAB "Skipping " << cur->name << " ns=" << (cur->ns ? (const char *)cur->ns->href : "<empty>") << endl;
			continue;
		}

		const char * str = (const char *) xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);

		cout << TAB << cur->name << TAB << (str ? (const char *)str : "nil") << endl;

		if (!strcmp((const char *)cur->name, "band")) {
			if (!strcmp(str, "VHF+")) {
			    c->band = 1;
			} else if (!strcmp(str, "VHF")) {
			    c->band = 2;
			} else if (!strcmp(str, "G1")) {
			    c->band = 3;
			} else if (!strcmp(str, "UHF")) {
			    c->band = 4;
			} else if (!strcmp(str, "G2")) {
			    c->band = 5;
			} else if (!strcmp(str, "U1")) {
			    c->band = 6;
			} else if (!strcmp(str, "U2")) {
			    c->band = 7;
			} else if (!strcmp(str, "U3")) {
			    c->band = 8;
			}
			cout << TAB "band=" << c->band << endl;

		} else if (!strcmp((const char *)cur->name, "frequency")) {
			char *p = NULL;
			long l = strtol(str, &p, 10);
			if (p && *p == '.') {
				const static int m[3] = {100, 10, 1};
				c->freq = l*1000;
				l = 0;
				p++;
				for (int i=0; i<3; ++i) {
					if (!p[i]) break;
					c->freq += m[i] * (p[i] - '0');
				}

		    } else {
				c->freq = l;
			}

			cout << TAB "freq=" << c->freq << endl;

		} else if (!strcmp((const char *)cur->name, "offset")) {
			if (!strcmp(str, "+")) {
				c->offset = +600;
			} else if (!strcmp(str, "-")) {
				c->offset = -600;
			}
			cout << TAB "offset=" << c->offset << endl;

		} else if (!strcmp((const char *)cur->name, "tone")) {
			long l = strtol(str, NULL, 10);
			switch (l) {
			case 67: c->tone = 1; break;
			case 69: c->tone = 2; break;
			case 71: c->tone = 3; break;
			case 74: c->tone = 4; break;
			case 77: c->tone = 5; break;
			case 79: c->tone = 6; break;
			case 82: c->tone = 7; break;
			case 85: c->tone = 8; break;
			case 88: c->tone = 9; break;
			case 91: c->tone = 10; break;
			case 94: c->tone = 11; break;
			case 97: c->tone = 12; break;
			case 100: c->tone = 13; break;
			case 103: c->tone = 14; break;
			case 107: c->tone = 15; break;
			case 110: c->tone = 16; break;
			case 114: c->tone = 17; break;
			case 118: c->tone = 18; break;
			case 123: c->tone = 19; break;
			case 127: c->tone = 20; break;
			case 131: c->tone = 21; break;
			case 136: c->tone = 22; break;
			case 141: c->tone = 23; break;
			case 146: c->tone = 24; break;
			case 151: c->tone = 25; break;
			case 156: c->tone = 26; break;
			case 162: c->tone = 27; break;
			case 167: c->tone = 28; break;
			case 173: c->tone = 29; break;
			case 179: c->tone = 30; break;
			case 186: c->tone = 31; break;
			case 192: c->tone = 32; break;
			case 203: c->tone = 33; break;
			case 206: c->tone = 34; break;
			case 210: c->tone = 35; break;
			case 218: c->tone = 36; break;
			case 225: c->tone = 37; break;
			case 229: c->tone = 38; break;
			case 233: c->tone = 39; break;
			case 241: c->tone = 40; break;
			case 250: c->tone = 41; break;
			case 254: c->tone = 42; break;
			default:
				cerr << TAB "Bad tone: " << str << endl;
			}

			if (c->tone) {
				cout << TAB "tone=" << tones[c->tone-1] << " (" << c->tone << ")" << endl;
				assert(!strncmp(str, tones[c->tone-1], strlen(str)));
			}

		} else if (!strcmp((const char *)cur->name, "dcs")) {
		    for (int i=1; dcsCodes[i]; i++) {
				if (!strcmp((const char *)str, dcsCodes[i])) {
					c->dcs = i;
					break;
				}
			}

			if (!c->dcs) {
				cerr << TAB "Bad DCS: " << str << endl;
			}

		} else if (!strcmp((const char *)cur->name, "mode")) {
			if (!strcmp(str, "AM")) {
			    c->mode = 1;
			}
			cout << TAB "mode=" << c->mode << endl;

		} else if (!strcmp((const char *)cur->name, "power")) {
			if (!strcmp(str, "high")) {
			    c->power = 0;
			} else if (!strcmp(str, "medium")) {
			    c->power = 1;
			} else if (!strcmp(str, "low")) {
			    c->power = 2;
			}
			cout << TAB "power=" << c->power << endl;

		} else if (!strcmp((const char *)cur->name, "name") && str) {
			c->name = str;
			if (c->name.length() > 8) c->name.resize(8);
			cout << TAB "name=" << c->name << endl;

		} else if (!strcmp((const char *)cur->name, "unknown")) {
			c->unknown = !strcmp((const char *)str, "true");
		}
    }

    return c.release();
}

static void processDoc(xmlDoc * doc, unsigned char * data) {
    xmlNode * root = xmlDocGetRootElement(doc);
	xmlNs * ns = NULL;
    if (root->ns) {
		if (strcmp((const char*)root->ns->href, CHANNEL_NS_URI)) {
		    cerr << "Bad NS URI: " << root->ns->href << endl;
		    return;
		}
		ns = root->ns;
	}

	if (strcmp((const char *)root->name, "channels")) {
	    cerr << "Bad root element: " << root->ns->href << endl;
	    return;
	}

	int n=101;
	for (xmlNode * node = root->children; node; node = node->next) {
	    if (node->type != XML_ELEMENT_NODE) {
			continue;
		}
		if (node->ns != ns) {
			cerr << "Skipping " << node->name << " ns=" << (node->ns ? (const char *)node->ns->href : "<empty>") << endl;
			continue;
		}
		if (strcmp((const char *)node->name, "channel")) {
			cerr << "Skipping " << node->name << " ns=" << (node->ns ? (const char *)node->ns->href : "<empty>") << endl;
			continue;
		}

	    Channel * chn = parseChannel(doc, ns, node);
		if (!chn) {
			cerr << "Could not parse channel" << endl;
		}

		vector<unsigned char> cdata(CHANNEL_SIZE);
		vector<unsigned char> sdata(STRING_SIZE);

		int slot = chn->slot ? chn->slot : n++;
		slot--;

        unsigned char * d;
        unsigned char * s;

		if (chn->bank < 2) {
            d = &data[CHANNEL_TOP_OFFSET + (slot * CHANNEL_SIZE)];
            s = &data[STRING_TOP_OFFSET + (slot * STRING_SIZE)];
		} else {
            d = &data[CHANNEL_BOT_OFFSET + (slot * CHANNEL_SIZE)];
            s = &data[STRING_BOT_OFFSET + (slot * STRING_SIZE)];
		}

		encodeChannel(chn, d, s);

		delete chn;
	}
}

int main(int argc, char *argv[])
{
    const char * xmlfile = argv[1];
    const char * infile = argv[2];
    const char * outfile = argv[3];

	if (argc != 4) {
		cerr << "expects three arguments: xml indata outdata" << endl;
		return 1;
	}

    ifstream is;

    is.open(xmlfile, ios::binary);
    if (!is.is_open()) {
	    cerr << "Failed to open " << xmlfile << " for input." << endl;
	    return 1;
    }

    is.seekg(0, ios::end);
    off_t xmllen = is.tellg();
    vector<char> xml(xmllen);

    is.seekg(0, ios::beg);
    is.read((&xml[0]), xmllen);
    is.close();

    is.open(infile, ios::binary);
    if (!is.is_open()) {
        cerr << "Failed to open " << infile << " for input." << endl;
        exit(1);
    }

    is.seekg(0, ios::end);
    off_t datalen = is.tellg();
    vector<unsigned char> input(datalen);

    is.seekg(0, ios::beg);
    is.read(reinterpret_cast<char *>(&input[0]), datalen);
    is.close();

    if (datalen != 25600) { 
		cerr << "bad input data length: " << datalen << endl;
		return 1;
	}

	vector<unsigned char> output(input);

    xmlDoc * doc = xmlReadMemory(&xml[0], xmllen, xmlfile, NULL, 0);
    if (!doc) {
	    cerr << "Failed to parse " << xmlfile << endl;
	    return 1;
    }

    processDoc(doc, &output[0]);
	xmlFreeDoc(doc);

    ofstream os;
	os.open(outfile, ios::binary);

    if (!os.is_open()) {
        cerr << "Failed to open " << outfile << endl;
        return 1;
    }

	os.write(reinterpret_cast<char *>(&output[0]), output.size());
	os.close();

	xmlCleanupParser();
	return 0;
}
