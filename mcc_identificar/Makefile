# The compiler to use
CC = g++

# Directories for Includes and Common classes
IDIR = ../include/
CDIR = ../commons/

# Compiler options: agregamos -fopenmp para habilitar OpenMP en los .o
CFLAGS = -Wall -O3 -std=c++11 -fopenmp -I$(IDIR)

# Flags de enlazado (linker)
LDFLAGS = -fopenmp

# Sources and Common classes sources
SOURCES = main_omp_13k.cpp MCC.cpp Cylinder.cpp
CSOURCES = $(CDIR)Fingerprint.cpp $(CDIR)Functions.cpp $(CDIR)Minutia.cpp $(CDIR)GrahamScanConvexHull.cpp $(CDIR)File19794.cpp $(CDIR)Munkres.cpp

# Objects
OBJECTS = $(SOURCES:.cpp=.o)
COBJECTS = $(CSOURCES:.cpp=.o)

# Name of the executable
EXECUTABLE = mcc_matching_omp_13k

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS) $(COBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(COBJECTS) $(LDFLAGS) -o $@

.cpp.o:
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(COBJECTS) $(EXECUTABLE)
