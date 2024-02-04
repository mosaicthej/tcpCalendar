---
Definitely not GPT generated, but:
 - I hope this design doc finds you well.
---

# TCP Calendar

## Overview

This is a simple calendar application that allows users do things with events.
The application is built using the TCP protocol.

The key is to demonstrate learning of TCP and socket programming, 
and simple protocol design.

The construction of the protocol should center around the following:
- Simple, easy to understand, easy to implement
- Minimalize the payload size over network. Do not send unnecessary data.

Consists of three main components:
- Client
- Server

## Database

Would only have 1 entity: `event`, with 6 fields:
- `username`  
- `eventname` 
- `date`      
- `startTime` 
- `endTime`   
- `valid`

For the scope and purpose of this assignment, it should be enogh.

This table is pretty normalized as well.

No need to think about the database and serialization/deserialization, 
on the server's side, it's a in-memory database, implemented using a 
simple linked list.

Per the assignment spec, each user would have 1 list, and the list should 
be sorted by order of `date` and `startTime`.

Also, store the lists in a linked list, and have the size able to be
dynamically adjusted.

Insert, update, delete and select operations are all in `O(m+n)` time,
where as `m` be the number of existing users, and `n` be the number of
events in the user's list.

## Client

The client is used to interact between the user and the server.

The client's responsibility is to:
- take user input
- validate user input
- parse user input into a request structure (abstract)
- pack the request structure into a request payload (128 bits)
- send the request payload to the server via TCP connection
- receive the response payload from the server.
- unpack the response payload into a response structure (abstract)
- print out the response to the user

## Server

The server is used to receive the request and send the response.

Be more detailed:
1. create a socket
2. bind the socket to a port
3. listen for incoming connections
4. accept incoming connections
5. receive the request payload from the client
6. unpack the request payload into a request structure (abstract)
7. process the request (database things)
8. pack the response structure into a response payload (104 bits)
9. send the response payload to the client
10. close the connection

3 types of server:

### Single-threaded server

**Approach Based on `Select()`**: A single-threaded server is used, 
but the server can handle multiple clients (and open sockets) with 
the `select()` system call.

The implementation will be similar to the multi-person chat server example
from *Beej's Guide to Network Programming*. 
In the sense that, we will use keeping track of all open sockets from 
`listen()` on `sockfd` to `new_fd` from `accept()` in a `fd_set` and
use `select()` to check for activity on the sockets.

The difference will be that, instead of forwarding the message to all
sockets, we will process the request, then send the response to only
the socket that sent the request.

Also, the connection will be closed after the response is sent.

Since it's single threaded, there is no need for synchronization and 
locks on the in-memory database.

### Multi-threaded server

Threads will be created from *posix threads* to handle each client.

The server will act as the main thread, and only `listen()` and `accept()`
connections. 

Once a connection is accepted, a new thread will be created, given the 
`new_fd` as parameter, and will handle the request and response by itself.
main thread will continue to `accept()` new connections and create children.

On the child thread's side, it will handle the request, do the database
operation (synchonization is required, using mutex on the user's list),
then send the response, and close the connection. Once the connection is
closed, the child thread will terminate and join back to the main thread.

The list for each user will have an additional `mutex` field to synchronize.

### Multi-process server

Similar to the multi-threaded server, but instead of creating threads,
we will create processes using `fork()`.

Because the memory would be different from the multi-threading case,
we need to first use `mmap()` to make the linked list a shared memory.

In the child process, it will handle the request, check the mutex, do the
database operation, send the response, and close the connection, finally
terminate.

I do not plan to use `munmap()` to unmap the shared memory, because I'm
not sure if that would nuke the map for sibling processes as well? As
man pages says that:
> The region is also automatically unmapped  when  the  process is terminated.

So I'll just let the OS handle it.

## Common code

The protocol handling and the database operations would be common to all
3 types of servers, consider to extract them into a separate module.

**Disclaimer**: 
We assume the user is an *intelligent* human being with the ability 
    to NOT break the application in every possible way.
Therefore, the error handling is minimal.
All user input is valid, and will result in valid action.
It's just a matter of "doing what the user REALLY thinks they want to do".
