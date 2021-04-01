############################################################
CXX = g++
CXXFLAGS = -g -Wall -Wextra -pedantic -lcurl -lboost_filesystem -lboost_system -std=c++11
EXC = build
OBJ = obj
############################################################

OUTPUT_NAME = execute_crawl

all: $(OBJ)/crawl.o
	$(CXX) $(CXXFLAGS) -o $(EXC)/$(OUTPUT_NAME) $(OBJ)/crawl.o

$(OBJ)/crawl.o: src/crawl.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c src/crawl.cpp

clean:
	$(RM) $(OBJ)/crawl.o

run: $(EXC)/$(OUTPUT_NAME)
	./$(EXC)/$(OUTPUT_NAME)
