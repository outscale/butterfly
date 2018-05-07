# Description

```
                      +-----------+      +-----------+
(vni 42) [ VM 1 ]-----|           |      |           |-----[ VM 3 ] (vni 42)
                      | Butterfly +======+ Butterfly |
(vni 42) [ VM 2 ]-----|           |      |           |-----[ VM 4 ] (vni 42)
                      +-----------+      +-----------+

```

This scenario test TCP communication between four VM through two butterfly.

Initial setup:
- 2 butterfly
- VM1 configured on vni 42 with security group sg-1
- VM2 configured on vni 42 with security group sg-1
- VM3 configured on vni 42 with security group sg-1
- VM4 configured on vni 42 with security group sg-1
- sg-1 is full open

Test that:
- TCP communication VM1 -> VM2 is OK
- TCP communication VM1 -> VM3 is OK
- TCP communication VM1 -> VM4 is OK
- TCP communication VM2 -> VM1 is OK
- TCP communication VM2 -> VM3 is OK
- TCP communication VM2 -> VM4 is OK
- TCP communication VM3 -> VM1 is OK
- TCP communication VM3 -> VM2 is OK
- TCP communication VM3 -> VM4 is OK
- TCP communication VM4 -> VM1 is OK
- TCP communication VM4 -> VM2 is OK
- TCP communication VM4 -> VM3 is OK
