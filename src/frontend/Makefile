all: lexer.o analyser.o node.o check.o table.o

lexer.o: ./src/lexer.cpp
	g++ -I./include ./src/lexer.cpp -c 

analyser.o: ./src/analyser.cpp
	g++ -I./include ./src/analyser.cpp -c 

node.o: ./src/node.cpp
	g++ -I./include ./src/node.cpp -c 

check.o: ./src/check.cpp
	g++ -I./include ./src/check.cpp -c 

table.o: ./src/table.cpp
	g++ -I./include ./src/table.cpp -c 

clean:
	rm -rf *.o