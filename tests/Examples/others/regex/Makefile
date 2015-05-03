all: dfa0 dfa1 nfa

dfa0: dfa0.o
	gcc -Wall -O3 -o dfa0 dfa0.o

dfa1: dfa1.o
	gcc -Wall -O3 -o dfa1 dfa1.o

nfa: nfa.o
	gcc -Wall -O3 -o nfa nfa.o

dfa0.o: dfa0.c
	gcc -Wall -O3 -c dfa0.c

dfa1.o: dfa1.c
	gcc -Wall -O3 -c dfa1.c

nfa.o: nfa.c
	gcc -Wall -O3 -c nfa.c

clean:
	rm -f *~ *.o dfa0 dfa1 nfa

