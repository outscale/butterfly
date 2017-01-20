# Description

This scenario invert IP between two VM and test connectivity.

1. Initial setup:
- VM1 has IPv4 a and Ipv6 A
- VM2 has IPv4 b and Ipv6 B

2. Test that:
- ping @VM1 a -> b OK
- ping @VM1 A -> B OK

3. Change setup:
- Update VM1 Ips so VM1 has ony Ipv4 b and Ipv6 B
- Update VM1 Ips so VM1 has ony Ipv4 a and Ipv6 A

4. Test that:
- ping @VM1 b -> a OK
- ping @VM1 B -> A OK

5. Change setup:
- Update VM1 Ips so VM1 has ony Ipv4 a and Ipv6 A
- Update VM1 Ips so VM1 has ony Ipv4 b and Ipv6 B

6. Loop to 2. ten times

