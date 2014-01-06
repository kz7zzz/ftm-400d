CPPFLAGS += `xml2-config --cflags`
XMLLIB += `xml2-config --libs`

all: ftm-import ftm-export ftm-strings

ftm-import: ftm-import.o
	$(CXX) $(CFLAGS) $(LDFLAGS) $^ $(XMLLIB) -o $@

ftm-export: ftm-export.o
	$(CXX) $(CFLAGS) $(LDFLAGS) $^ -o $@

ftm-strings: ftm-strings.o
	$(CXX) $(CFLAGS) $(LDFLAGS) $^ -o $@

ftm-export.o: ftm.h
ftm-import.o: ftm.h
ftm-strings.o: ftm.h

clean:
	-rm -f *.o ftm-import ftm-export ftm-strings
