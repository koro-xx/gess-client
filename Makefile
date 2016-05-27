TARGET= gess

CC= gcc
CFLAGS= -Wall -W -Wno-unused

LIBS= -lm -lallegro -lallegro_font -lallegro_memfile -lallegro_ttf -lallegro_primitives -lircclient
LDFLAGS= -L/usr/lib 

INCLUDE= -I. -I./src/widgetz -I/usr/include/allegro5 -I/usr/include/libircclient
SOURCES= $(wildcard src/*.c) $(wildcard src/widgetz/src/*.c) $(wildcard src/widgetz/src/widgets/*.c)
HEADERS= $(wildcard src/*.h)
OBJECTS= $(SOURCES:.c=.o)

.PHONY: clean all default

default: $(TARGET)
all: default

%.o: %.c $(HEADERS)
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $@ $< 

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) $(CFLAGS) $(LDFLAGS) $(LIBS) -o $@

clean:
	find . -name "*.o" -exec rm -v {} \;
#	-rm -v $(TARGET)


