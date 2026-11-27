CXX = g++
CXXFLAGS = -g -std=c++17 -O3
WARNINGS = -Wall -Wextra -Wpedantic

main : main.cpp AlgoUtils.cpp DefaultAlgorithm.cpp Knuth.cpp Utils.cpp
	$(CXX) $(WARNINGS) $(CXXFLAGS) -o bullcow main.cpp AlgoUtils.cpp DefaultAlgorithm.cpp Knuth.cpp Utils.cpp

run : bullcow
	./bullcow

clean : 
	rm bullcow
