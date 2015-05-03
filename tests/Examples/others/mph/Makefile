HEADERS = ../src/cmph.h
INCLUDE_DIRS = -I ../src
LIBS = ../src/.libs/libcmph.a

default: sparrow_cmp_test

sparrow_cmp_test.o: sparrow_cmp_test.cpp $(HEADERS)
	clang++ -O3 -c sparrow_cmp_test.cpp -o sparrow_cmp_test.o $(INCLUDE_DIRS)

sparrow_cmp_test: sparrow_cmp_test.o
	clang++ sparrow_cmp_test.o $(LIBS) -o sparrow_cmp_test -lm -L /usr/lib -L /usr/lib/i386-linux-gnu

clean:
	-rm -f sparrow_cmp_test.o
	-rm -f sparrow_cmp_test