# Description

```
+-----------+
|           |-----------[ VM 1 ] (vni 42)
| Butterfly |
|           |-----------[ VM 2 ] (vni 42)
+-----------+

```

- This test start a butterfly server with two nic and
the same security group but no rules and make a bunch
of network tests (ping, tcp and udp).

- Add sg rules full open and make a bunch of network
tests (ping, tcp and udp).

- Remove sg rules full open and make a bunch of network
tests (ping, tcp and udp).

- Add sg rules opened on a specific port and make a bunch
of network tests (ping, tcp and udp).

