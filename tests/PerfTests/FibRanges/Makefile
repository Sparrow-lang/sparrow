
GCC=clang
OPT_LEVEL=-O2
C_FLAGS=
C_LIBS=-lc
CPP_FLAGS=-std=c++11
CPP_LIBS=-lc -lstdc++

SPARROWC=~/work/sparrow/dev/.out/Release/SparrowCompiler

all: fib_op.cpp.out fib_op.c.out fib_op.spr.out fib_op_base.spr.out

%.spr.out: %.spr
	$(SPARROWC) -o $@ $< $(OPT_LEVEL) $(DEBUG_FLAGS)

%.c.out: %.c
	$(GCC) -o $<.o $< -c $(OPT_LEVEL) $(C_FLAGS)
	$(GCC) -o $@ $<.o $(C_LIBS) $(OPT_LEVEL) $(DEBUG_FLAGS)

%.cpp.out: %.cpp
	$(GCC) -o $<.o $< -c $(OPT_LEVEL) $(CPP_FLAGS)
	$(GCC) -o $@ $<.o $(CPP_LIBS) $(OPT_LEVEL) $(DEBUG_FLAGS)

%.py.out: %.py
	cp $< $@
	chmod +x $@ 

clean:
	rm -f *.o
	rm -f *.out
	rm -f *.llvm
