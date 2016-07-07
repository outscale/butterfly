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
and make a bunch of network tests (tcp and udp). Update the security
group and make a bunch of network tests (tcp and udp). Stop the two vm
and restart them, update security group and make a bunch of network
tests (tcp and udp).
