SERVER=server
SUBSCRIBER=subscriber

SERVER_SOURCES=server.c server_utils.c common_utils.c
SUB_SOURCES=subscriber.c subscriber_utils.c common_utils.c

LIBPATHS=.
INCPATHS=include
CFLAGS=-c -Wall -g
CC=gcc

SERV_OBJECTS=$(SERVER_SOURCES:.c=.o)
SUB_OBJECTS=$(SUB_SOURCES:.c=.o)
INCFLAGS=$(foreach TMP,$(INCPATHS),-I$(TMP))
LIBFLAGS=$(foreach TMP,$(LIBPATHS),-L$(TMP))
VPATH=src

SERV_BINARY=$(SERVER)
SUB_BINARY=$(SUBSCRIBER)

all: $(SERVER_SOURCES) $(SUB_SOURCES) $(SERV_BINARY) $(SUB_BINARY)

$(SERV_BINARY): $(SERV_OBJECTS)
	$(CC) $(LIBFLAGS) $(SERV_OBJECTS) $(LDFLAGS) -o $@
$(SUB_BINARY): $(SUB_OBJECTS)
	$(CC) $(LIBFLAGS) $(SUB_OBJECTS) $(LDFLAGS) -o $@

.c.o:
	$(CC) $(INCFLAGS) $(CFLAGS) -fPIC $< -o $@

clean:
	rm -f $(SUB_OBJECTS) $(SERV_OBJECTS) $(SUB_BINARY) $(SERV_BINARY)
