# Description

```
                      +-----------+      +-----------+
(vni 42) [ VM 1 ]-----|           |      |           |-----[ VM 3 ] (vni 42)
                      | Butterfly +======+ Butterfly |
(vni 42) [ VM 2 ]-----|           |      |           |-----[ VM 4 ] (vni 42)
                      +-----------+      +-----------+

```

This scenario connects four virtual machines through two butterfly.
Each VM are in the same network (VNI) and can ping.
