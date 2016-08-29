# Description

```
+-----------+
|           |-----------[ VM 1 ] (vni 42)
| Butterfly |
|           |-----------[ VM 2 ] (vni 42)
+-----------+

```

- This test start a butterfly server with two nic opened
on a specific.

- Remove/add security group and make a bunch of network tests
(tcp and udp).

- Those operations are done 10 times.

