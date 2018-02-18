CC=gcc

all: echos echo

echos: server.c
	$(CC) -o echos server.c
echo: client.c
	$(CC) -o echo client.c

.PHONY: clean

clean:
	rm -f echos echo