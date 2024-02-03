---
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
- Database

## Protocol

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

| field | len | bits field |
|-------|-----|------------|
| opcode| 1   | [0]        |
| clTime| 21  | [1-21]     |
| date  | 16  | [22-37]    |
| uname | 30  | [38-67]    |
| ename | 60  | [68-127]   |

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

| charType | ordinals | encode formula  | decode formula  |
|----------|----------|-----------------|-----------------|
| digit 0-9| 0-9      | x = c - '0'     | c = x + '0'     |
| upper A-Z| 10-35    | x = c - 'A' + 10| c = x + 'A' - 10|
| lower a-z| 36-61    | x = c - 'a' + 36| c = x + 'a' - 36|
| spcl `-` | 62       | x = 62          | if x == 62      |
| spcl `_` | 63       | x = 63          | if x == 63      |

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


## Client

**Disclaimer**: 
We assume the user is an *intelligent* human being with the ability 
    to NOT break the application in every possible way.
Therefore, the error handling is minimal.
All user input is valid, and will result in valid action. 
It's just a matter of "doing what the user REALLY thinks they want to do".

