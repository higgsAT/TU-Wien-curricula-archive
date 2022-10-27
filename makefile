############################################################
CXX = g++
CXXFLAGS = -g -Wall -Wextra -pedantic -lboost_system -std=c++17
EXC = build
OBJ = obj
############################################################

OUTPUT_NAME = execute_crawl

all: $(OBJ)/crawl.o
	$(CXX) $(CXXFLAGS) -o $(EXC)/$(OUTPUT_NAME) $(OBJ)/crawl.o -lboost_filesystem -lcurl

$(OBJ)/crawl.o: src/crawl.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c src/crawl.cpp -lboost_filesystem -lcurl

clean:
	$(RM) $(OBJ)/crawl.o

run: $(EXC)/$(OUTPUT_NAME)
	./$(EXC)/$(OUTPUT_NAME)
