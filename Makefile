OUT?=ddb_bs2b.so

CFLAGS+=-std=c99 -fPIC -Wall -I./libbs2b-3.1.0/src

BS2B_SOURCES=\
libbs2b-3.1.0/src/bs2b.c

SOURCES=bs2b.c $(BS2B_SOURCES)

OBJECTS=$(SOURCES:.c=.o)

all: $(SOURCES) $(OUT)

$(OUT): $(OBJECTS)
	$(CC) $(CFLAGS) -shared -lm $(OBJECTS) -o $@ $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) $< -c -o $@

clean:
	rm $(OBJECTS) $(OUT)
