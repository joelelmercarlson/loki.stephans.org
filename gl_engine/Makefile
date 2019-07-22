PRJ_DIR = $(HOME)/project/gl_engine
SRC = ./src
INC = ./include
BIN = ./bin
PROG = $(BIN)/gl_game
CXX = g++
SDLFLAGS = `sdl-config --cflags --libs`
CXXFLAGS = -g -Wall $(SDLFLAGS) -I$(INC) -lGL -lX11
COMPILE=$(CXX) $(CXXFLAGS)
OBJFILES := $(patsubst %.cpp,%.o,$(wildcard $(SRC)/*.cpp))

all: gl_game 

gl_game: $(OBJFILES)
	cd $(PRJ_DIR); \
	mkdir -p $(BIN); \
	rm -f $(PROG); \
	$(COMPILE) \
	-o $(PROG) $(OBJFILES)

%.o: %.cpp
	$(COMPILE) -o $@ -c $<

clean:
	rm -f $(PROG) $(PROG1) $(SRC)/*.o
