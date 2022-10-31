modules := src/frontend src/middle src/backend
objects := src/frontend/*.o src/middle/*.o  src/backend/*.o 
includes := -I./src/frontend/include -I./src/middle/include -I./src/backend/include

.PHONY: all $(modules) clean

all: $(modules) main

main: $(modules)
	g++ -std=c++11 -O2 -o main src/main.cpp $(objects) $(includes)

$(modules):
	$(MAKE) --directory=$@

clean:
	for d in $(modules);				\
		do					\
			$(MAKE) --directory=$$d clean;	\
		done;					\
	rm -rf main 

run:
	./main

zip:
	rm -rf tmp/*; \
	cp src/frontend/src/*.cpp tmp/ ; \
	cp src/frontend/include/*.h tmp/ ; \
	cp src/middle/src/*.cpp tmp/ ; \
	cp src/middle/include/*.h tmp/ ; \
	cp src/backend/src/*.cpp tmp/ ; \
	cp src/backend/include/*.h tmp/ ; \
	cp src/main.cpp tmp/ ; \
	zip -q -r 1.zip tmp; \
	mv 1.zip /mnt/c/Users/wangxuezhu/Desktop