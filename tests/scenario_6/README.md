# Description

```
+-----------+
|           |-----------[ VM 1 ] (vni 42)
| Butterfly |
|           |-----------[ VM 2 ] (vni 42)
+-----------+

```

This scenario just connects two virtual machines on the same butterfly
on the same network (vni) and make a bunch of network tests (ping, ...)
This test also check that we can delete and add again nics
