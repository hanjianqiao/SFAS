all:
	g++ src/sfas.cpp -o sfas -std=c++0x

clean:
	rm sfas
