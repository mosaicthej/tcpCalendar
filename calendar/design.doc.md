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


## Client

**Disclaimer**: 
We assume the user is an *intelligent* human being with the ability 
    to NOT break the application in every possible way.
Therefore, the error handling is minimal.
All user input is valid, and will result in valid action. 
It's just a matter of "doing what the user REALLY thinks they want to do".

