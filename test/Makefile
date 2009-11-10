

CPPFLAGS:=-I../build/usr/include/ $(CPPFLAGS)
LDFLAGS=`pkg-config --libs cppunit` -L../build/usr/lib/ -lnitro

TESTS=tests/node.o tests/error.o tests/types.o tests/xml.o tests/device.o tests/userdevice.o tests/scripts.o

run: test userdevice.so
	LD_LIBRARY_PATH=../build/usr/lib ./test

mem: test
	LD_LIBRARY_PATH=../build/usr/lib valgrind --leak-check=full --show-reachable=yes ./test

test: $(TESTS) test.o
	g++ -o test test.o $(TESTS) $(LDFLAGS)

userdevice.so: userdevice.cpp
	g++ $(CPPFLAGS) -fPIC -o userdevice.so -shared userdevice.cpp

clean:
	rm *.o tests/*.o test *.so
