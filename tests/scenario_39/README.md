# Description

This scenario will check that we can have several IP on an interface:
- VM1 has 2 IPv4 (a1, a2) and 2 Ipv6(A1, A2)
- VM3 has 2 IPv4 (b1, b2) and 2 Ipv6(B1, B2)

Then test ping: 
- a1 -> b1
- a1 -> b2
- a2 -> b1
- a2 -> b2
- A1 -> B1
- A1 -> B2
- A2 -> B1
- A2 -> B2
