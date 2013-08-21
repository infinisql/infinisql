August 19, 2013

Refer to http://www.infinisql.org for information about InfiniSQL (tm).

but briefly:

autoreconf --force --install
./configure --prefix=/some/where (/usr/local probably not a good choice)
make
make install (sudo if need be)
cd procs
./makem.sh /some/where (sudo if need be)

