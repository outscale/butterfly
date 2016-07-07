# Description

```
+-----------+
|           |-----------[ VM 1 ] (vni 42)
| Butterfly |
|           |-----------[ VM 2 ] (vni 42)
+-----------+

```

This scenario just connects two virtual machines on the same butterfly
on the same network (vni) vm1 without security group, vm2 full open
and make a bunch of network tests (tcp and udp).

Add the security group on vm1 and make a bunch of network tests (tcp and udp).
Delete security group and make a bunch of network tests (tcp and udp).
Add security group rule and make a bunch of network tests (tcp and udp).
