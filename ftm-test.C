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

using namespace std;

#include "ftm.h"

int main(int argc, char *argv[])
{
    unsigned x = 0;
	for (unsigned x = 0; x < 256; x++) {
		if (a2d[x] && (d2a[a2d[x]] != x)) {
		    cerr << "ASCII: " << std::hex << x << "->" << a2d[x] << " reversed to " << d2a[a2d[x]] << endl;
		}
	}

	// zero is a special case
	for (x = 1; x < 256; x++) {
		if (d2a[x] != '#' && (a2d[d2a[x]] != x)) {
		    cerr << "YCHAR: " << std::hex << x << "->" << d2a[x] << " reversed to " << a2d[d2a[x]] << endl;
		}
	}
}
