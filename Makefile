
CPP=g++
CXXFLAGS=-std=c++17 -pedantic -Wall -Wextra -g -I ./include

all:
	$(CPP) $(CXXFLAGS) src/main.cc src/inet.cc src/signals.cc

