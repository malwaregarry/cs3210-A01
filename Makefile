CXX=g++
CXXFLAGS:=-Wall -Werror -Wextra -pedantic -std=c++17 -fopenmp
RELEASEFLAGS:=-O3
DEBUGFLAGS:=-g

.PHONY: all clean
all: submission

submission: main.o
	$(CXX) $(CXXFLAGS) $(RELEASEFLAGS) -o troons $^

main.o: main.cc
	$(CXX) $(CXXFLAGS) $(RELEASEFLAGS) -c $^

clean:
	$(RM) *.o troons

debug: main.cc
	$(CXX) $(CXXFLAGS) $(DEBUGFLAGS) -D DEBUG -o troons main.cc util.cc

bonus: main_bonus.o
	$(CXX) $(CXXFLAGS) $(RELEASEFLAGS) -o troons_bonus $^

main_bonus.o: main_bonus.cc
	$(CXX) $(CXXFLAGS) $(RELEASEFLAGS) -c $^