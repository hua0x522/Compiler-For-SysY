all: generate.o tools.o ir.o
	
generate.o: ./src/generate.cpp
	g++ -I./include -I../frontend/include ./src/generate.cpp -c 

tools.o: ./src/tools.cpp
	g++ -I./include -I../frontend/include ./src/tools.cpp -c 

ir.o: ./src/ir.cpp
	g++ -I./include -I../frontend/include ./src/ir.cpp -c 

clean:
	rm -rf *.o