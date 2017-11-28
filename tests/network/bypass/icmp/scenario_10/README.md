# Description

```
                        +-----------+      +-----------+
  (vni 42) [ VM 1 ]-----|           |      |           |-----[ VM 2 ] (vni 42)
                        | Butterfly +======+ Butterfly |
                        |           |      |           |
                        +-----------+      +-----------+
```

Test internal switch add / removing

Initial setup:
- 2 butterfly
- VM1 configured on vni 42 with security group sg-1
- VM1 on Butterfly1
- VM2 configured on vni 42 with security group sg-1
- VM2 on Butterfly2
- Open ICMP for sg-1

Test that:
- Ping VM2 -> VM1 is OK

Change setup:
- Create a vnic 3 on Butterfly1
- Start VM3

Test that:
- Ping VM2 -> VM1 is OK
- Ping VM2 -> VM3 is OK
- Ping VM1 -> VM3 is OK

Change setup:
- Stop VM3
- Delete vnic 3 on Butterfly1

Test that:
- Ping VM2 -> VM1 is OK

Change setup:
- Create a vnic 3 on Butterfly1
- Create a vnic 4 on Butterfly1
- Start VM3

Test that:
- Ping VM2 -> VM1 is OK
- Ping VM2 -> VM3 is OK
- Ping VM1 -> VM3 is OK

Change setup:
- Stop VM3
- Delete vnic 3 on Butterfly1
- Delete vnic 4 on Butterfly1

Test that:
- Ping VM2 -> VM1 is OK

Note: we run this scenario several times in different configurations
- With and without trace option
- With and without bypass_filtering option
- With a mix of both
