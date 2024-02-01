---
- Name: Mark Jia
- NSID: mij623
- StuN: 11271998
---

# part A

## Q1 ( 4 )

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

Detailed calculation see the spread sheet (below if you viewing pdf)

## Q2 ( 4 )

What is the minimum bandwidth needed to achieve a data rate of `B bits/sec` 
if the signal is transmitted using 
 - NRZ,
 - MLT-3, and 
 - Manchester encoding, 
 respectively? Explain your answer.

### A2.1
use $w$ to represent `bandwidth` to avoid confusion
(since $B$ is used as the data rate (in bits/sec))

Nyquist:
 B = $2w log_2 V$ bits/sec
 w: bandwidth
 V: discrete levels

#### NRZ
binary -> 2 states: V=2

$$
\begin{align}
B &= 2w log_2 V \\
  &= 2w \times 1 \\
  &= 2w\\
w &= \frac{B}{2}
\end{align}
$$

minimum bandwidth needs to be $\frac{B}{2}$ Hz

#### MLT-3

Has three possible states => V=3

To encode any 1 bit (information is 1 or 0), 
    it could be one of three states (-1, 0, 1).

Hence, to encoding B bits (2^B of information), 
it would have (3^B) different encodings in MLT-3.

So B bits as the data would mean $\frac{3^B}{2^B}\times B$ information sent
    through physical layer, which, $log_2{3^B}=B\times log_2 3$ bits
$$
\begin{align}
& log_2 3 \times B &= 2w log_2 3 \\
& w &= \frac{B}{2}
\end{align}
$$

The minimum bandwidth needs to be $\frac{B}{2}$ Hz

#### Manchester

Has 50% coding effieiency (the other half is for clocking info)
Hence, to send B bits $=2^B$ information, requires an encoding of 2B bits of
    data sent through physical.

$$
\begin{align}
& 2\times B &= 2w log_2 2\\
&   w &= \frac{B}{log_2 2}\\
&   w &= B
\end{align}
$$

Hence, the minimum bandwidth needs to be $B$ Hz.

Ten signals, each requiring 4000 Hz, are multplexed onto a channel using FDM. What is the minimum bandwidth required for the multiplexed channel? Assume that the guard bands are 400 Hz wide.

