# ---
# - Name: Mark Jia
# - NSID: mij623
# - StuN: 11271998
# ---

# the list library is borrowed from John Miller from 332 Fa23
# this makefile is incomplete, don't run it
CC = gcc
CFLAGS = -g -Wall -std=c99 -pedantic -Wextra -Werror
LDFLAGS = -g
LDLIBLIST = -L. -llist

ARCHIVE = ar -r -c -s

binaries = forkserv pthreadserv selectserv mycal

.PHONY: all clean
all: $(binaries) liblist.a

mycal: client.o protocolc.o
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBLIST)

client.o: client/client.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o client.o -c client/client.c -I.

forkserv: forkserv.o protocols.o 
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBLIST)

forkserv.o: server/forkserv.c protocols.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -o forkserv.o -c server/forkserv.c -I.

pthreadserv: pthreadserv.o protocols.o protocols.h
	$(CC) $(LDFLAGS) -o $@ $^ -lpthread $(LDLIBLIST)

pthreadserv.o: server/pthreadserv.c protocols.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -o pthreadserv.o -c server/pthreadserv.c -I.

selectserv: selectserv.o protocols.o 
	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBLIST)

selectserv.o: server/selectserv.c protocols.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -o selectserv.o -c server/selectserv.c -I.

protocols.o: protocols.c protocols.h
	$(CC) $(CFLAGS) $(CPPFLAGS) -o protocols.o -c protocols.c -I.

liblist.a: list.h list_adders.o list_movers.o list_removers.o
	$(ARCHIVE) liblist.a list_adders.o list_movers.o list_removers.o

list_adders.o: list.h list_adders.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o list_adders.o -c list_adders.c -I.

list_movers.o: list.h list_movers.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o list_movers.o -c list_movers.c -I.

list_removers.o: list.h list_removers.c
	$(CC) $(CFLAGS) $(CPPFLAGS) -o list_removers.o -c list_removers.c -I.

clean:
	rm -f $(binaries) *.o *.a
