# Description

```
+-----------+
|           |-----------[ VM 1 ] (vni 42)
| Butterfly |
|           |-----------[ VM 2 ] (vni 42)
+-----------+

```

This scenario just connects two virtual machines on the same butterfly
on the same network (vni), open udp protocol on a specific port and
make a bunch of network tests (udp, tcp)
