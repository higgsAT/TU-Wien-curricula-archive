############################################################
CXX      = g++
CXXFLAGS = -g -Wall -Wextra -pedantic -std=c++17
LIBS     = -lboost_system -lboost_filesystem -lcurl
EXC      = build
OBJ      = obj
############################################################
OUTPUT_NAME = execute_crawl

all: $(EXC)/$(OUTPUT_NAME)

$(EXC)/$(OUTPUT_NAME): $(OBJ)/crawl.o $(OBJ)/mail.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIBS)

$(OBJ)/crawl.o: src/crawl.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ src/crawl.cpp

$(OBJ)/mail.o: src/mail.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ src/mail.cpp

clean:
	$(RM) $(OBJ)/crawl.o $(OBJ)/mail.o

run: $(EXC)/$(OUTPUT_NAME)
	./$(EXC)/$(OUTPUT_NAME)
