// Copyright 2014, Kurt Zeilenga. All rights reserved.
//
// Permission to use, copy, modify, and/or distribute this software
// for any purpose with or without fee is hereby granted, provided
// that the above copyright notice and this permission notice appear
// in all copies.
//
// Provided "as is" without warranty of any kind.

#ifndef __FTM_H__
#define __FTM_H__

class Channel
{
public:
	std::string memname;
	int bank;
	int slot;

	int band;
	int rx;		// in khz
	int tx;		// in khz
	int duplex; // 0, -1, +1
	int offset; // in khz
	int sql;
	int tone;
	int dcs;
	int mode;
	int power;
	std::string tag;
	int scan;

	Channel() :
		bank(0),
		slot(0),
		band(0),
		rx(0),
		tx(0),
		duplex(0),
		offset(0),
		sql(0),
		tone(0),
		dcs(0),
		mode(0),
		power(0),
		scan(0)
	{
	}

	static const int NBANKS = 2;
	static const int NCHANNELS = 500;

	static const unsigned int HOME_OFFSET = 0x00b0U;
	static const unsigned int HOME_TAG_OFFSET = 0x01b0U;

	static const unsigned int CHANNEL_SIZE = 0x0010U;
	static const unsigned int TAG_SIZE = 0x0008U;
	static const unsigned char TAG_FILL = 0xcaU;

	static const unsigned int CHANNEL_TOP_OFFSET = 0x0200U;
	static const unsigned int CHANNEL_TOP_TAG_OFFSET = 0x42c0U;
	static const unsigned int CHANNEL_BOT_OFFSET = 0x2260U;
	static const unsigned int CHANNEL_BOT_TAG_OFFSET = 0x52f0U;

	static const int NPCHANNELS = 18;
	static const unsigned int PCHANNEL_OFFSET = 0x2140U;
	static const unsigned int PCHANNEL_TAG_OFFSET = 0x5260U;
};

extern const char * scans[];
extern const char * modes[];
extern const char * powers[];
extern const char * sqls[];
extern const char * tones[];
extern const char * dcsCodes[];
extern const unsigned char d2a[];
extern const unsigned char a2d[];
extern const char * utf8[];
extern const char * bands[];
extern const char * pchannels[];

std::string int2str(int i);
size_t str2data(const std::string & str, unsigned char * sbuf);
std::string data2str(const unsigned char * s, size_t len);
std::string xmlsafe(const std::string & in);

#define XML_DECL "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
#define SCHEMA_NS_URI "http://boolean.net/ftm400dr/0"
#define SCHEMA_LOC_URI "https://raw.github.com/kz7zzz/ftm-400d/master/channels.xsd"
#define SCHEMA_EXT_NS_URI "http://boolean.net/ftm400dr/ext/0"
#define TAB "\t"

#endif
