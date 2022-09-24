all: lexer.o analyser.o node.o

lexer.o: ./src/lexer.cpp
	g++ -I./include ./src/lexer.cpp -c 

analyser.o: ./src/analyser.cpp
	g++ -I./include ./src/analyser.cpp -c 

node.o: ./src/node.cpp
	g++ -I./include ./src/node.cpp -c 

clean:
	rm -rf *.o