LUAINC = -I/usr/include/lua5.1
LIBXML = -I/usr/include/libxml2


all: testlib.exe genhtml.so 

testlib.exe: libForLua.c main.c
	gcc -g -std=c99 libForLua.c $(LUAINC) $(LIBXML) main.c -llua5.1 -lxml2 -o testlib.exe


genhtml.so: libForLua.c
	gcc -g -std=c99 libForLua.c $(LIBXML) $(LUAINC) -fpic -c -o genhtml.o
	gcc -O -std=c99 -shared -fpic -llua5.1 -lxml2 -o genhtml.so genhtml.o

run:
	lua manovich.lua


clean:
	rm *~ *.o *.so *.exe
