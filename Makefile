OUT?=ddb_bs2b.so

BS2B_LIBS?=-lbs2b
CFLAGS+=-std=c99 -fPIC -Wall -shared -lm

SOURCES=bs2b.c

OBJECTS=$(SOURCES:.c=.o)

all: $(SOURCES) $(OUT)

$(OUT): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(BS2B_LIBS) $(LDFLAGS)

.c.o:
	$(CC) $(CFLAGS) $< -c -o $@

clean:
	rm $(OBJECTS) $(OUT)
