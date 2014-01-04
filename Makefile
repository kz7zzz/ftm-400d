CC = $(CXX)
CPPFLAGS += `xml2-config --cflags`
LDFLAGS += `xml2-config --libs`

all: ftm-import ftm-export ftm-strings

ftm-import: ftm-import.o
ftm-export: ftm-export.o
ftm-strings: ftm-strings.o

ftm-export.o: ftm.h
ftm-import.o: ftm.h
ftm-strings.o: ftm.h

clean:
	-rm -f *.o ftm-import ftm-export ftm-strings
