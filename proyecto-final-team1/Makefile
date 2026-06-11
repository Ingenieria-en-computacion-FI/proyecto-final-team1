CC = gcc
CFLAGS = -Wall -Wextra -Iinclude
SOURCES = $(wildcard src/*.c) $(wildcard *.c)
TARGET = ensamblador

all: $(TARGET)

$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $(SOURCES) -o $(TARGET)

clean:
	rm -f $(TARGET) *.obj *.o salida.obj
