# Description

```
+-----------+
|           |-----------[ VM 1 ] (vni 42)
| Butterfly |-----------[ VM 2 ] (vni 42)
|           |-----------[ VM 3 ] (vni 42)
+-----------+

```

This scenario just connects three virtual machines on the same butterfly
on the same network (vni) vm1 and vm2 opened on Port 8000, vm3 opened on
port 9000 and make a bunch of network tests (tcp and udp).
Delete and Update the security group on the same port for the three vm
and make a bunch of network tests (tcp and udp).