.PHONY: all debug

OBJ = main.cpp main.h deltaproject.h

COMMAND = export LD_LIBRARY_PATH=/home/ilya/Qt/5.9/gcc_64/lib:$LD_LIBRARY_PATH;\
	g++ -Wall -Werror -std=c++14 -o sim main.cpp -l:libdelpro.so.1.0.0

all: $(OBJ)
	$(COMMAND)

debug: $(OBJ)
	$(COMMAND) -g
