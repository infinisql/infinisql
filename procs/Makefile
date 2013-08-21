CPP := $(CXX)
CPPFLAGS := $(CMDLINE) -Wall -Wno-deprecated -ggdb -L$(PREFIX)/lib -I$(PREFIX)/include -shared -fPIC -rdynamic -O3 -finline-functions -std=c++11 -mcx16
LIBS := -linfinisql

%.so: %.cc $(PREFIX)/lib/libinfinisql.so
	$(CPP) $(CPPFLAGS) $(LIBS) -o $@ $<

clean:
	- rm *.so

