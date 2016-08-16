# Description

```
                      +-----------+      +-----------+
(vni 42) [ VM 1 ]-----|           |      |           |-----[ VM 2 ] (vni 42)
                      | Butterfly +======+ Butterfly |
                      |           |      |           |
                      +-----------+      +-----------+

```

This scenario connects two virtual machines through two butterfly.
Each VM are in the same network (VNI) and can ping.
