all: myfind

myfind: main.cpp
	g++ -o myfind main.cpp

clean:
	rm -f myfind
