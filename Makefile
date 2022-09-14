modules := src/frontend
objects := src/frontend/*.o
includes := -I./src/frontend/include

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
	zip -q -r 1.zip src; \
	mv 1.zip /mnt/c/Users/wangxuezhu/Desktop