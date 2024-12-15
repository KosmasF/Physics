CCX = gcc
CFLAGS = -g -Wall -Wextra -D_REENTRANT	

CPP_SOURCES = $(wildcard *.cpp)
CPP_OBJECTS = $(patsubst %.cpp,%.o,$(CPP_SOURCES))
C_HEADERS = $(wildcard *.h)


LDFLAGS = -L. -L/usr/lib -L../Chess/OpenCL -L../Chess/NeuralNetwork -L../Discrete-Fourier-Transform
LDLIBS = -lm -lSDL2 -lSDL2_ttf -lOpenCL -l:libgpu.o -l:libNeuralNetwork.o -l:Map.o
INC = -I. -I/usr/include/SDL2 -I../Chess/OpenCL -I../Chess/NeuralNetwork

EXECUTABLE = main

$(EXECUTABLE) : $(CPP_OBJECTS)
	$(CXX) $(CFLAGS) $(LDFLAGS) $(LDLIBS) -o $(EXECUTABLE) $(CPP_OBJECTS)

$(CPP_OBJECTS) : %.o : %.cpp
	$(CXX) $(INC) $(CFLAGS) -c $^ -o $@

clean:
	rm -f *o $(EXECUTABLE)
