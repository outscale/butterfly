# Description

```
+-----------+
|           |-----------[ VM 1 ] (vni 42) (sg-1)
| Butterfly |-----------[ VN 2 ] (vni 42) (sg-1)
|           |-----------[ VM 3 ] (vni 42) (sg-2)
+-----------+

```
This test start a butterfly server with three nic 1,2 and 3.
- nic 1 and 2 have the same sg (sg-1) and (sg-1) for nic 3
- sg1 add a rule allowing members of sg2 and make a bunch of network tests (tcp and udp).
- sg1 add a new rule allowing members of sg1 and make a bunch of network tests (tcp and udp).
- Remove B from sg1 and make a bunch of network tests (tcp and udp).
