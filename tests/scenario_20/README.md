# Description

```
+-----------+
|           |-----------[ VM 1 ] (vni 42)
| Butterfly |-----------[ VM 2 ] (vni 42)
|           |-----------[ VM 3 ] (vni 42)
+-----------+

```

This scenario just connects three virtual machines with the same SG on the same butterfly
- Add a rule in SG listening on the port 8000
- Make a bunch of network tests (tcp and udp)
- Remove this rule
- Make a bunch of network tests (tcp and udp)
- Add a new rule in SG listening on the port 9000
- Make a bunch of network tests (tcp and udp)
