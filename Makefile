CC=mpicc
CFLAGS=-c 
LDFLAGS=-lm
SOURCES=main.c
OBJECTS=$(SOURCES:.cpp=.o)
EXECUTABLE=pflock

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.cpp.o:
	$(CC) $(CFLAGS) $< -o $@
