# Description

```
                        +-----------+      +-----------+
  (vni 42) [ VM 1 ]-----|           |      |           |-----[ VM 3 ] (vni 1337)
                        | Butterfly +======+ Butterfly |
(vni 1337) [ VM 2 ]-----|           |      |           |-----[ VM 4 ] (vni 42)
                        +-----------+      +-----------+

```

This scenario test ping communication between VM
on different VNI through two butterfly.

Initial setup:
- VM1 configured on vni 42 with security group sg-1
- VM2 configured on vni 1337 with security group sg-1
- VM3 configured on vni 1337 with security group sg-1
- VM4 configured on vni 42 with security group sg-1
- sg-1 has one rule full opened

Test that:
- ping communication VM1 -> VM2 is KO
- ping communication VM1 -> VM3 is KO
- ping communication VM1 -> VM4 is OK
- ping communication VM4 -> VM1 is OK
- ping communication VM4 -> VM2 is KO
- ping communication VM4 -> VM3 is KO
