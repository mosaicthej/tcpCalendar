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



# Protocol Design for TCP Calendar

## Protocol－Request

Trying to achieve a protocol between the client and the server that will
save as much bandwidth as possible. That means we need to pack as much 
information as possible into a single request, and a single response.

To make simple implementation, we set all request to have same size.

I've designed a way to pack the 2 time fields: start and end time,
into a 21 bit field to be accurate to minutes. 

Since all $(24\cdot60)^2=2073600$ possible combinations of the 2 time fields
did not exhaust all patterns of the 21 bit field, some of the patterns
are reserved for special cases (to encode some special actions).

Detailed way to encode and decode the time fields will be explained later.

That might be able to save a bit from `command`, to encode things like
- `REMOVE` which would only require 1 time field,
- `GET1` which would require 1 time field,
- `GET2` which would require no time field.
- `GETALL` which would require no time field.

That way, I'd be able to encode all possible commands:
`ADD`, `UPDATE`, `REMOVE`, `GET1`, `GET2`, `GETALL` into **1 bit**!
(i'm not drunk when i wrote this)

That would become easier to explain once we get to the design of the time fields.

But per my design, each request would only take 128 bits (16 bytes). 
If we have a 128 bit computer, entire request can be put into a single register!
(afaik, there isn't any such computer yet, the best they do is 128 bit SIMD)

| field  | len  | bits field |
| ------ | ---- | ---------- |
| opcode | 1    | `[0]`      |
| clTime | 21   | `[1-21]`   |
| date   | 16   | `[22-37]`  |
| uname  | 30   | `[38-67]`  |
| ename  | 60   | `[68-127]` |

Note that the `len` is in bits.

### Overview of each part

#### Opcode

1 bit field, use fields from `clTime` to encode additional commands.

#### clTime

Calendar Time, 21 bits field, 

Contain both start and end time, accurate to minutes.

since some patterns will never be used as time, we can use them to encode
for commands like `REMOVE`, `GET1`, `GET2`, `GETALL`.

#### Date

16 bits field (this is the result of saving bits from here and there)

16 bits would support a range of 65536 calendar days, which is about 180 years.

More than enough for anyone to use (unless timetravellers)

Set epoch to 1900-01-01, and we can support dates from 1900 to 2080.

#### Uname and Ename

User input fields, each varchar takes 6 bits, so 30 and 60 bits respectively.

### varchars

The username and eventname are encoded as varchars.

Using entire ascii table is such a waste, there are too many characters that
no one would ever ever going to use.
who put emojis or emoticons in their stuff anyway?  ╮(╯▽╰)╭

For fields that are supposed to take user-input, I've decided to use only
[A-Za-z0-9\-\_] characters, which are:
- 10 digits
- 26 uppercase letters
- 26 lowercase letters
- 2 special characters: `-` and `_`

That's 64 characters in total, which can be encoded into 6 bits.

table of conversion:

| charType  | ordinals | encode formula   | decode formula   |
| --------- | -------- | ---------------- | ---------------- |
| digit 0-9 | 0-9      | x = c - '0'      | c = x + '0'      |
| upper A-Z | 10-35    | x = c - 'A' + 10 | c = x + 'A' - 10 |
| lower a-z | 36-61    | x = c - 'a' + 36 | c = x + 'a' - 36 |
| spcl `-`  | 62       | x = 62           | if x == 62       |
| spcl `_`  | 63       | x = 63           | if x == 63       |

That way, we group every 4 characters into 3 bytes,
versus 4 ordinary bytes, would be 25% reduction in size.

#### Uname and Ename

Because I gets to decide, I'll limit:
- `username` to 4 characters
    (original plan was 6, but I changed my mind so date can have 2 extra bits)
- `eventname` to 10 characters.

Yeah, English is a language that is redundant and verbose,
The cardinality of the set of English vocabulary is much wasted,
if you compare it to the cardinality of the set of all possible words.

$|V_{eng}| \ll |V_{all}| = \sum_{n=1}^{N} 64^n$

where $N$ is the maximum length of the word.

(╯°□°）╯︵ ┻━┻) I'm not letting you waste my bandwidth, 
Don't trying to write a novel in the eventname field.

$\sum_{n=1}^{10} 64^n$ is a big number, probably larger than number of 
atoms in the universe (no that's not true). At least larger than how many
differnt event names you can come up with.

Go get yourself a hash table if you can't manage to fit your event names.

### Date

Date is encoded as 16 bits field.

Set epoch to 1900-01-01, we can support dates from 1900 to 2080.

If I have time, 
it should follow the rules of the Gregorian calendar, including leap years.

For now, just assume every month has 30 days.

### clTime

This is the fun part.

At most, we'd need 2 time fields: start and end time. (sometimes just 1 time)

To encode HHMM time, it would need $24\cdot60=1440$ different values,
which, would need 11 bits. 2 such fields would need 22 bits.

But, there is a catch: the `endTime` can only be a time after `startTime`.

Thinking about `deltaTime` instead of `endTime`, then `deltaTime` would have
$\sum_{i=1}^{1440} i = 1440\cdot\frac{1441}{2} = 1037520$ different values,
which, not can fit into 21 bits (and still have some patterns left).

Another thing that makes our life easier is that, since to encode time need
$1440$ different values, which is just a tiny bit more than $2^{10}=1024$,
and, let's say, in case that the `startTime` takes more than 10 bits, 
(more than $1024$ minutes, from 00:00 to 18:56), the max `deltaTime` would be
$1440-1024=416$, which is enough with 9 bits. 
(that's how we save 1 bit to encode more commands)

#### So, here is the plan

(because this way makes the math eaiser (I won't explain what if the althernative))

`startTime` would be encoded in *inversedMinutes*, that mean, a value of $0$ would
mean 23:59, and a value of $1439$ would mean 00:00.

Use the highest (20th) bit to indicate if that *inversedMinutes* exceeds 1024 or not.

So, *inversedMinutes* <=> `[20|(20?8:9)-0]`
(if 20th bit is 1, then 9 bits (8-0) are used, otherwise 10 bits (9-0) are used)

##### **Case if *inversedMinutes* less than 1024**

$$
\begin{align*}
\text{let }& t_0 \text{ be the startTime in minutes} \\
t_0 &= 1439 - \text{inversedMinutes} \\
0 &\leq \text{inversedMinutes} \leq 1023 \\
\therefore 416 &\leq t_0 \leq 1439 \\
\\
\text{let } \Delta_t &\text{ be the deltaTime} \\
\text{knowing that } \Delta_t &\text{ can at most bring the endTime to 1439}\\
\text{and }& \text{ can at least bring the endTime to }t_0 \\
\\
\Delta_{max} &= 1439 - t_0 \\
    &= 1439 - (1439 - \text{inversedMinutes}) \\
    &= \text{inversedMinutes} \\
\Delta_{min} &= 0 \\
\\
\therefore \\
0 \leq \Delta_{min} &\leq \Delta_t \leq \Delta_{max} \leq 1023 \\
\end{align*}
$$

We conclude that, in case that the *inversedMinutes* is less than 1024, it would be sufficient to use 10 bits to encode the `deltaTime`.

if `[20] LOW`, then `deltaTime` <=> `[19-10]`, `startTime` <=> `[9-0]`

##### **Case if *inversedMinutes* exceeds 1024**

In this case, `exceeds` <=> `[20] HIGH` and the rest of the bit would be have
values at most $1439-1024=415$, which is enough to fit into 9 bits `[8-0]`.
Which, leaves room for `deltaTime` to be encoded in 11 bits `[19-9]`.

if `[20] HIGH`, then `deltaTime` <=> `[19-9]`, `startTime` <=> `[8-0]`

*inversedMinutes* <=> `[20|(20?8:9)-0]`
*deltaTime* <=> `[19-(20?9:10)]`

#### Special rules:

there are some patterns that would never be used as time, for example,
when `exceeds` is `HIGH`, then *9-bit low bits field* `[8-0]` would at most 
have a value of $415$, which is less than $512$. Thus, if we set all bits to
some value $k\geq416$, then we can use those pattern to encode special commands
that does not require 2 time fields, and use the bit field `[19-9]` to encode
the single time field.

##### REMOVE encoding
REMOVE date start-time

Need 1 time field.
`exceeds` is `HIGH`, then `[8-0]` set to $504$ `1 1111 1000`

The time field would be encoded in `[19-9]` as numbers of minutes from 00:00.

##### GET1 encoding
GET date start-time

Need 1 time field.
`exceeds` is `HIGH`, then `[8-0]` set to $505$ `1 1111 1001`

The time field would be encoded in `[19-9]` as numbers of minutes from 00:00.

##### GET2 encoding
GET date

Need no time field.
`exceeds` is `HIGH`, then `[8-0]` set to $506$ `1 1111 1010`

The bit field `[19-9]` needs to be set all set to $1$, check this,
if it's not, then it's an error of invalid encoding for GET2.

##### GETALL encoding
GETALL

Need no time field.
`exceeds` is `HIGH`, then `[8-0]` set to $511$ `1 1111 1111`

The bit field `[19-9]` needs to be set all set to $1$, check this,
if it's not, then it's an error of invalid encoding for GETALL.

##### summary

| command | `[20]` | `[19-9]`        | `[8-0]`             |
| ------- | ------ | --------------- | ------------------- |
| REMOVE  | 1      | min-after 00:00 | `1 1111 1000` (504) |
| GET1    | 1      | min-after 00:00 | `1 1111 1001` (505) |
| GET2    | 1      | fill with `1`   | `1 1111 1010` (506) |
| GETALL  | 1      | fill with `1`   | `1 1111 1111` (511) |

to have `[20] HIGH` and `[8-0]` $k\geq 416$, above would be the only
valid encodings, other patterns would be invalid and server should return
an error.

### Opcode

The opcode is a 1 bit field, which is used to encode command types.

- If `opcode` is `0`, then it's an `ADD` command with 2 time fields.
- If `opcode` is `1`, and if the `clTime` is not a special pattern, then
    it's an `UPDATE` command with 2 time fields.
- If `opcode` is `1`, and `clTime` is a special pattern, then it's a special command.

### Summary

Therefore, I define each request to be 128 bits (16 bytes) in length.

Lol, the amount of work to implement this would be making me killing myself 💀

## Protocol－Response

Depending on the command, the server would respond with different payloads of data.

### Single status response

For commands that is analogous to a DML command in SQL, the server would simply
respond with a single status code.

This would be the case for `ADD`, `UPDATE`, `REMOVE` commands.

The status code would be 1 byte (shortest possible payload).

Including the information about what type of command was executed.

### Event data response

For command that is analogous to a DQL command in SQL, the server would respond
with a status code followed by a list of events.

The single status code would have a part of the payload that, if on success, 
notifies the number of events that are returned (size).

Each event would be 60 bits in length, 
use 4 additional bits to pad to 64 bits -> 8 bytes.

Since there are only 4 bits for the size data, the client would at most take 15
events at a time. However, this is not anywhere near being useful.

So the end of the last event's padding would be used to indicate if there are
more events to be fetched. And so on (kind of like a linked list).

Header (read 32 more) -> ... -> 32th event (read 32 more) -> ... -> last event (no more)

So this way the server can send as many events as it wants (and order is preserved).

### Encoding

We pack each response into a 104 bits (13 bytes) payload.

| field      | len  | desc      |
| ---------- | ---- | --------- |
| `[103]`    | 1    | success?  |
| `[102]`    | 1    | has body? |
| `[101-81]` | 21   | clTime    |
| `[80-65]`  | 16   | date      |
| `[64-5]`   | 60   | event     |
| `[4-0]`    | 5    | more?     |

**success?**: 1 bit, 1 on success, 0 on failure.

**has body?**: 1 bit, 1 if there is a body in this response. 0 if not
    0 is used for res to `ADD`, `UPDATE`, `REMOVE` commands.
    In this case, the rest of the payload is required to be 0.

**clTime**: 21 bits, same as in request.

**date**: 16 bits, same as in request.

**event**: 60 bits, same as in request.

**more?**: 5 bits.
    This is useful in the case of `GETALL` command, to indicate if there are
    more events to be fetched.
    These 5 bits can represent up to 31 more events to be fetched, so will
    know how many more bytes to read.
    If there's more than 31 events to be fetched, then the end of the last
    event's padding would be used to indicate to read $n$ more events.

In case of a non-tail event of the response, this field is required be 0.

There is a special case for `ADD` and `UPDATE` commands to handle the case
where the the failure is due to the event has conflicting time with an existing
event. Hence the top 2 bits: `success?` and `has body?` would be `[0|1]`. 
This is a failure with body, and the rest of the payload would be the
conflicting event.

yea... An alternative approach would be first send a header of success and size,
then send the events in the following payload. But that would limit the size
to 127 events, have the it would be quite inconvenient to get the bytes to be
aligned. So I'm happy with this approach where each response is guaranteed to
be 13 bytes in length.
