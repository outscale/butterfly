# Description

```
                        +-----------+      +-----------+
  (vni 42) [ VM 1 ]-----|           |      |           |-----[ VM 3 ] (vni 1337)
                        | Butterfly +======+ Butterfly |
(vni 1337) [ VM 2 ]-----|           |      |           |-----[ VM 4 ] (vni 42)
                        +-----------+      +-----------+

```

This scenario connects four virtual machines through two butterfly.
VM 1 & VM 4 are on the same VNI (42).
VM 2 & VM 3 are on the same VNI (1337).
