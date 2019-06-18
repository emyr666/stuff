all: map1 map2 map3

map1: map1.cpp
	g++ -o map1 -O2 -Wall -Weffc++ -std=c++14 map1.cpp -lpthread

map2: map2.cpp
	g++ -o map2 -O2 -Wall -Weffc++ -std=c++14 map2.cpp -lpthread

map3: map3.cpp
	g++ -o map3 -O2 -Wall -Weffc++ -std=c++14 map3.cpp -lpthread

clean:
	touch tmp.o
	rm *.o
	rm map1
	rm map2
	rm map3
