EXE=./run
CC=g++
STD=-std=c++20
CXXFLAGS=-Wall -Wextra -pedantic -Wconversion
RAYFLAGS=`pkg-config --cflags --libs raylib`

all: clean main

main: main.cpp
	$(CC) $(CXXFLAGS) -o $(EXE) main.cpp $(RAYFLAGS) $(OBJS) $(STD)

%.o: %.cpp
	$(CC) $(CXXFLAGS) -c $< -o $@ $(RAYFLAGS)

clean:
	rm -rf $(EXE) *.o *.i *.asm test
