# The compiler to use.
CC = g++

# Directories for Includes and Common clases
IDIR =../include/
CDIR =../commons/

# Compiler options
CFLAGS= -Wall -O3 -std=c++11 -I$(IDIR)

# Sources and Common clases sources
SOURCES= benchmark_descriptor.cpp MCC.cpp Cylinder.cpp
CSOURCES= $(CDIR)Fingerprint.cpp $(CDIR)Functions.cpp $(CDIR)Minutia.cpp $(CDIR)GrahamScanConvexHull.cpp $(CDIR)File19794.cpp $(CDIR)Munkres.cpp

# Objects
OBJECTS=$(SOURCES:.cpp=.o)
COBJECTS=$(CSOURCES:.cpp=.o)

# Name of the executable
EXECUTABLE=benchmark_descriptor

all: $(SOURCES) $(CSOURCES) $(EXECUTABLE)


$(EXECUTABLE): $(OBJECTS) $(COBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) $(COBJECTS) -o $@

.cpp.o:
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(COBJECTS) $(EXECUTABLE)
