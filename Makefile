# Copyright 2014, Kurt Zeilenga. All rights reserved.
#
# Permission to use, copy, modify, and/or distribute this software
# for any purpose with or without fee is hereby granted, provided
# that the above copyright notice and this permission notice appear
# in all copies.
#
# Provided "as is" without warranty of any kind.


CPPFLAGS += `xml2-config --cflags`
XMLLIB = `xml2-config --libs`

all: ftm-strings ftm-test ftm-export ftm-import

clean:
	-rm -f *.o ftm-import ftm-export ftm-strings ftm-test

ftm-import: ftm-import.o ftm-common.o
	$(CXX) $(CFLAGS) $(LDFLAGS) $^ $(XMLLIB) -o $@

ftm-export: ftm-export.o ftm-common.o
	$(CXX) $(CFLAGS) $(LDFLAGS) $^ -o $@

ftm-strings: ftm-strings.o ftm-common.o
	$(CXX) $(CFLAGS) $(LDFLAGS) $^ -o $@

ftm-test: ftm-test.o ftm-common.o
	$(CXX) $(CFLAGS) $(LDFLAGS) $^ -o $@

ftm-export.o: ftm.h
ftm-import.o: ftm.h
ftm-strings.o: ftm.h
ftm-test.o: ftm.h
