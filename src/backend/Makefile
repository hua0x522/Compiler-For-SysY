all: translate.o mips.o

translate.o: ./src/translate.cpp
	g++ -I./include -I../middle/include ./src/translate.cpp -c

mips.o: ./src/mips.cpp
	g++ -I./include -I../middle/include ./src/mips.cpp -c

clean:
	rm -rf *.o