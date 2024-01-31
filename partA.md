# part A

## Q1 (4 marks) 

Consider two nodes connected by a single dedicated link within an 
    OCN (on-chip network), 
    SAN (system/storage area network), 
    LAN (local area network), 
    or WAN (wide-area network), 
and suppose that we wish to transmit a single 100 byte 
(including header) packet from one node to the other. 

Calculate the total delay and the percentage of delay which is 
propagation delay for each network, assuming the following:
- the link data rate is 6 Gbps;
- there is no queueing delay;
- the total node processing delay 
  (not overlapped with any other component of delay) is 
  `x + (0.8 nanoseconds/byte)`, 
  where x (the portion of this delay that is independent of packet size) is 
  - 0 microseconds for the OCN, 
  - 0.6 microseconds for the SAN, 
  - 6 microseconds for the LAN, 
  - 60 microseconds for the WAN; 
- the link distances are 
    0.5 cm, 5m, 5000m, and 5000 km, 
    for the OCN, SAN, LAN, and WAN, respectively, 
    with the speed of signal propagation in each case equal to 
    200,000 km/s (approximately 2/3 of the speed of light in a vacuum).

### Solution 1

| Type | Total Delay (s) | Percent Delay that is propagation |
|-|-|-|
| OCN | 2.13583 E-8 | 0.12% |
| SAN | 4.63333 E-8 | 53.96%|
| LAN | 2.50213 E-5 | 99.91%|
| WAN | 0.02500     | 100%  |



