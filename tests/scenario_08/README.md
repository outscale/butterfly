# Description

```
+-----------+
|           |-----------[ VM 1 ] (vni 42)
| Butterfly |
|           |-----------[ VM 2 ] (vni 1337)
+-----------+

```

This scenario test UDP communication between VM with different VNI
on one butterfly.

Initial setup:
- 1 butterfly
- VM1 configured on vni 42 with security group sg-1
- VM2 configured on vni 1337 with security group sg-1
- sg-1 is full open

Test that
- UDP communication VM2 -> VM1 is KO
- UDP communication VM1 -> VM2 is KO
