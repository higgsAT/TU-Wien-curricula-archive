############################################################
CXX = g++
CXXFLAGS = -g -Wall -Wextra -pedantic -lcurl -lboost_filesystem -lboost_system -std=c++11
############################################################

OUTPUT_NAME = execute_crawl

all: crawl.o
	$(CXX) $(CXXFLAGS) -o $(OUTPUT_NAME) crawl.o

crawl.o: src/crawl.cpp
	$(CXX) $(CXXFLAGS) -c src/crawl.cpp

clean:
	$(RM) crawl.o

run: $(OUTPUT_NAME)
	./$(OUTPUT_NAME)

valgrind: $(OUTPUT_NAME)
	valgrind --tool=memcheck --track-origins=yes --leak-check=yes --num-callers=20 --track-fds=yes --suppressions=minimal.supp ./$(OUTPUT_NAME)
