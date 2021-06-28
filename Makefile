CXX = g++
CFLAGS = -std=c++11 -O3 -Wall -Wextra
LEDAROOT=./LEDA_lib/LEDA
.PHONY: all clean
OBJS = main.o parser.o

all:	als
als:	$(OBJS)
	$(CXX) -o $@ $^ $(CFLAGS) -I$(LEDAROOT)/incl -L$(LEDAROOT) -lG -lL -lm
main.o:		main.cpp parser.hpp
	$(CXX) -c main.cpp $(CFLAGS) -I$(LEDAROOT)/incl -L$(LEDAROOT) -lG -lL -lm
parser.o:	parser.cpp parser.hpp
	$(CXX) -c parser.cpp $(CFLAGS) -I$(LEDAROOT)/incl -L$(LEDAROOT) -lG -lL -lm
clean:
	$(RM) *.o

