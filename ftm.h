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
	int bank;
	int slot;
	int band;
	int freq; // in hz
	int offset; // in hz
	int tone;
	int dcs;
	int mode;
	int power;
	std::string name;
	bool unknown;
	bool skip;

	Channel() :
		bank(0),
		slot(0),
	    band(0),
		freq(0),
		offset(0),
		tone(0),
		dcs(0),
		mode(0),
		power(0),
		unknown(false),
		skip(false)
	{
	}

	static const int NBANKS = 2;
	static const int NCHANNELS = 500;

	static const unsigned int HOME_OFFSET = 0x00b0U;
	static const unsigned int HOME_STRING_OFFSET = 0x01b0U;

	static const unsigned int CHANNEL_SIZE = 0x0010U;
	static const unsigned int STRING_SIZE = 0x0008U;
	static const unsigned char STRING_FILL = 0xcaU;

	static const unsigned int CHANNEL_TOP_OFFSET = 0x0200U;
	static const unsigned int CHANNEL_TOP_STRING_OFFSET = 0x42c0U;
	static const unsigned int CHANNEL_BOT_OFFSET = 0x2260U;
	static const unsigned int CHANNEL_BOT_STRING_OFFSET = 0x52f0U;

	static const int NPCHANNELS = 18;
	static const unsigned int PCHANNEL_TOP_OFFSET = 0x2140U;
	static const unsigned int PCHANNEL_TOP_STRING_OFFSET = 0x5260U;
};

extern const char * tones[];
extern const char * dcsCodes[];
extern const unsigned char d2a[];
extern const unsigned char a2d[];
extern const char * utf8[];

std::string int2str(int i);
size_t str2data(const std::string & str, unsigned char * sbuf);
std::string data2str(const unsigned char * s, size_t len);
std::string xmlsafe(const std::string & in);

#define XML_DECL "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
#define CHANNEL_NS_URI "http://boolean.net/ftm400dr/0"
#define CHANNEL_EXT_NS_URI "http://boolean.net/ftm400dr/ext/0"
#define TAB "\t"

#endif
