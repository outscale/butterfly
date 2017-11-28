# Description

This scenario will check that we can have several IP on an interface:
- VM1 has 3 IPv4 (a1, a2, a3)
- VM2 has 3 IPv4 (b1, b2, b3)

Then test ping: 
- a1 -> b1
- a1 -> b2
- ...
- a3 -> b3
- b1 -> a1
- b1 -> a2
- ...
- b3 -> a3
