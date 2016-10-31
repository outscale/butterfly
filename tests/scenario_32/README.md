# Description

```
                        +-----------+      +-----------+
  (vni 42) [ VM 1 ]-----|           |      |           |-----[ VM 2 ] (vni 42)
                        | Butterfly +======+ Butterfly |
                        |           |      |           |
                        +-----------+      +-----------+
```

This scenario that DHCP is open by default

Initial setup:
- 2 butterfly
- VM1 configured on vni 42 with security group sg-1
- VM1 has it's IP statically configured and runs a DHCP server
- VM2 configured on vni 42 with security group sg-1
- VM2 has it's IP configured using DHCP (client)

Test that:
- Ping VM2 -> VM1 is KO

Change setup:
- Open ICMP on sg1

Test that:
- Ping VM2 -> VM1 is OK

