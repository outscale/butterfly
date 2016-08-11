# Description

```
+-----------+
|           |-----------[ VM 1 ] (vni 42)
| Butterfly |
|           |-----------[ VM 2 ] (vni 42)
+-----------+

```

This test start a butterfly server with two nic.
Add and remove security group and rule.
Make a bunch of network tests (tcp and udp).
Those operations are done several times.

