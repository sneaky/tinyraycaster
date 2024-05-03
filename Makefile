EXE = tinyraycaster 
CC = g++
OBJDIR = obj
SRC = $(wildcard *.cpp)
HDR = $(wildcard *.h)
LIBS = -Wall --std=c++11
OUT = *.ppm

$(EXE): $(SRC)
	$(CC) $(SRC) -o $(EXE) $(LIBS)

clean:
	rm -rf $(EXE) $(OUT)

debug: $(SRC) $(HDR)
	$(CC) $(SRC) -g -o $(EXE) $(LIBS)

testing: $(SRC) $(HDR)
	$(CC) $(SRC) -g -fsanitize=address -o $(EXE) $(LIBS)
