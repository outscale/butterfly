# Description

```
+-----------+
|           |-----------[ VM 1 ] (vni 42)
| Butterfly |
|           |-----------[ VM 2 ] (vni 42)
+-----------+

```

This scenario test UDP communication between VM on one butterfly.

Initial setup:
- 1 butterfly
- VM1 configured on vni 42 with security group sg-1
- VM2 configured on vni 42 with security group sg-1
- sg-1 is full open

Test that
- UDP communication VM2 -> VM1 is OK
- UDP communication VM1 -> VM2 is OK
