# Description

```
+-----------+
|           |-----------[ VM 1 ] (vni 42)
| Butterfly |
|           |-----------[ VM 2 ] (vni 42)
+-----------+

```

This scenario just connects two virtual machines(vm1 without security
group, vm2 full open) on the same butterfly on the same network (vni)
and make a bunch of network tests (tcp and udp)
