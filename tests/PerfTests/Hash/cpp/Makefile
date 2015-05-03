
PROGRAM=TestHashPerfCpp
DEBUG_FLAGS=
CPP_FLAGS=-std=c++11 $(DEBUG_FLAGS)
OPT_LEVEL=-O2
OPT_FLAGS=-std-compile-opts -std-link-opts $(OPT_LEVEL)
LIBS=-lc -lstdc++
LLVM_DIR=../../../../.out/Release/llvm
GCC=gcc
RUN_ARGS=

OPTS=-O2

$(PROGRAM).out:  $(PROGRAM).o
	$(GCC) -o $(PROGRAM).out $(PROGRAM).o $(LIBS) $(OPT_LEVEL) $(DEBUG_FLAGS)

$(PROGRAM).o: $(PROGRAM).cpp
	$(GCC) -o $(PROGRAM).o $(PROGRAM).cpp -c $(OPT_LEVEL) $(CPP_FLAGS)

$(PROGRAM)-alt.ll: $(PROGRAM).cpp
	$(GCC) -o $(PROGRAM)-alt.ll $(PROGRAM).cpp -c -S -emit-llvm $(CPP_FLAGS)

$(PROGRAM)-alt.opt.bc: $(PROGRAM)-alt.ll
	$(LLVM_DIR)/opt -o $(PROGRAM)-alt.opt.bc $(PROGRAM)-alt.ll $(OPT_FLAGS)

$(PROGRAM)-alt.opt.ll: $(PROGRAM)-alt.opt.bc
	$(LLVM_DIR)/llvm-dis -o $(PROGRAM)-alt.opt.ll $(PROGRAM)-alt.opt.bc

$(PROGRAM)-alt.opt.o: $(PROGRAM)-alt.opt.bc
	$(LLVM_DIR)/llc -o $(PROGRAM)-alt.opt.o $(PROGRAM)-alt.opt.bc --filetype=obj

$(PROGRAM)-alt.out: $(PROGRAM)-alt.opt.o
	$(GCC) -o $(PROGRAM)-alt.out $(PROGRAM)-alt.opt.o -O0 $(DEBUG_FLAGS) $(LIBS)

all: $(PROGRAM).out $(PROGRAM)-alt.out $(PROGRAM)-alt.opt.ll

run: $(PROGRAM).out
	./$(PROGRAM).out $(RUN_ARGS)

run-alt: $(PROGRAM)-alt.out $(PROGRAM)-alt.opt.ll
	./$(PROGRAM)-alt.out $(RUN_ARGS)

clean:
	rm -f *~
	rm -f *.ll
	rm -f *.bc
	rm -f *.o
	rm -f *.out
