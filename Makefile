
CPP=g++
CXXFLAGS=-std=c++17 -pedantic -Wall -Wextra -g -I ./include

all:
	$(CPP) $(CXXFLAGS) src/main.cc src/inet.cc src/signals.cc


testi: clean
	$(CPP) $(CXXFLAGS) src/testi.cc -o testi

clean:
	rm -rf testi a.out
