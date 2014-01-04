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
		unknown(false)
	{
	}
};

static const int NCHANNELS = 500;
static const unsigned int CHANNEL_TOP_OFFSET = 0x0200U;
static const unsigned int CHANNEL_BOT_OFFSET = 0x2260U;
static const unsigned int CHANNEL_SIZE = 0x0010U;
static const unsigned int STRING_TOP_OFFSET = 0x42c0U;
static const unsigned int STRING_SIZE = 0x0008U;
static const unsigned int STRING_BOT_OFFSET = 0x52f0U;
static const unsigned char STRING_FILL = 0xcaU;
static const unsigned char STRING_ZERO = 191U;

#define XML_DECL "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
#define CHANNEL_NS_URI "http://boolean.net/ftm400dr/0"
#define TAB "\t"

#endif
