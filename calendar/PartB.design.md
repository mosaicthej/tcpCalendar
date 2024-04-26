---
- Name: Mark Jia
- NSID: mij623
- StuN: 11271998

- Definitely not GPT generated, but:
 - I hope this design doc finds you well. 🤖
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
4. accept incoming connections (needs to track number of connections)
5. receive the request payload from the client
6. unpack the request payload into a request structure (abstract)
7. process the request (database things)
8. pack the response structure into a response payload (104 bits)
9. send the response payload to the client
10. close the connection

3 types of server:

#### Maximum connections

The `MAX_CONN` would be different for each type of server.

Except the `select()` based server, which has way smaller limit
due to `FD_SETSIZE` (1024). Other servers limit would be hard
to determine.

3 factors that limits the number of connections:
1. Maximum socket file descriptors:
    - check via `getrlimit(RLIMIT_NOFILE, &rlim)`
2. Maximum threads (pthreads):
    - from `cat /proc/sys/kernel/threads-max`,
        - on my vm, it's 256214
        - and on `tux` or `trux` cluster, it's 4194304.
    - from `cat /proc/sys/kernel/pid_max`, 
        - on my vm, it's 32768, 
        - and on `tux` or `trux` cluster, it's 41man 94304.
> EAGAIN A  system-imposed  limit  on  the number of threads was encountered. 
> There are a number of limits that may trigger this  error:  the  RLIMIT_NPROC
> soft  resource limit  (set  via  setrlimit(2)), which limits the number of 
> processes and threads for a real user ID, was reached; the kernel's system-wide
> limit on the number  of processes  and  threads, /proc/sys/kernel/threads-max, 
> was reached (see proc(5)); or the  maximum  number  of  PIDs,  
> /proc/sys/kernel/pid_max,  was  reached  (see proc(5)).
3. Maximum memory...

Similar problem with the max number of users, as the memory and 
number of mutexes are limited.

So I guess is just to not set a hard limit at all, and do error 
handling for each cases, and checks `EAGAIN`, `ENFILE`, `EMFILE`, `ENOMEM` for each, and handle them properly.

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
locks on the in-memory database. However, I'll include the lock in the
user's list anyways for all to keep consistency (1 data structure design
for all 3 types of servers).

#### Some notes on the consistency of semantics

With `select()`, although there's no need for synchronization, but due to
the underlying implementation of `select()` (I didn't read the source code,
but it's either checking on cyclic queue or hardware interrupt), for example,
if the two request,
```
./mycal ... add ... newEvent
./mycal ... getall
```
arrive at the same time, the output will be undetermined, the `getall` could
either include the result of *newEvent* or not. And might be different behavior
between each execution.

There are 2 ways:
1. introduce a priority to the server (or notion of precedence), that the modify
    operation will be processed first, then the read operation.

    I'm not sure if `select()` can do this.

2. Just say on the documentation here that this application is **eventually
    consistent**. If the user is not confident with the result, just re-run
    the command and it should have the updated result.

#### Maximum connections

Such server can hold up to `MAX_CONN = 1024` connections, due to the
limitation of `FD_SETSIZE` (1024) on the `select()` system call.

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

## Utilities

The client and server will have their own utilities to handle the parts where
not directly related to the TCP connection.

### Client utilities

**strstructc**: between string and data structure for client.
- Validate the input.
- Parse the command line arguments to data structure (`request_t`).
- Format and `response_t` data structure to strings that is ready to output.

**payloadstructc**: between payload and data structure for client.
- Validate the payload received from the server. (check for invalid response)
- Pack the data structure into a payload.
- Unpack the payload into a data structure.

```c
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    char[5] username;
    char[10] eventname;
    uint16_t daysSinceEpoch;
    uint16_t startTime;
    uint16_t endTime;
    bool isADD;
} request_t;

typedef struct {
    uint32_t opcode : 1;
    uint32_t clTime : 21;
    uint32_t date   : 16;
    uint32_t uname  : 30; // Represented as an integer here, but will need special handling
    uint64_t ename  : 60; // Same as above
} reqpayload_t;

typedef struct {
    bool success;
    bool hasBody;
    int date;
    int startTime;
    int endTime;
    uint8_t numNext;
} response_t;

int parse_args(int argc, char* argv[], request_t* req);
int reqToPayload(request_t* req, char* buf);
int format_response(response_t* res, char* buf);
int streamToRes(char* buf, response_t* res);

/* local functions */
uint8_t charTo6bit(char c);
char bit6ToChar(uint8_t b);
uint32_t timeToClTime(uint16_t startTime, uint16_t endTime);
int clTimeToTime(uint32_t clTime, uint16_t* startTime, uint16_t* endTime);
```

Request handling:

user input (`char*[]`) -> client -> `parse_args()` -> client (`request_t`) 
-> `reqToPayload()` -> client (`char[16]`) -> TCP -> server

Response handling:

TCP -> client (`char*[13]`) -> streamToRes() -> client (`response_t`)
-> format_response() -> client (`char*`) -> output

### Server utilities

**payloadstructs**: between payload and data structure for server.
- Validate the payload received from the client. (check for invalid request)
- Unpack the payload into a data structure (`request_t`).
- Pack the `response_t` data structure into a payload.

**dbops**: database operations.
- lock and unlock the mutex (`mutex`, `read_lock`, `write_lock`)
- select, insert, update, delete
- create new user list

```c
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    char[5] username;
    char[10] eventname;
    uint16_t daysSinceEpoch;
    uint16_t startTime;
    uint16_t deltaTime;
    bool isADD;
} request_t;

typedef struct {
    bool success;
    bool hasBody;
    uint16_t daysSinceEpoch;
    uint16_t startTime;
    uint16_t endTime;
    int endTime;
    uint8_t numNext;
} response_t;


```
