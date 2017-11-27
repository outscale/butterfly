# Description

```
                        +-----------+      +-----------+
  (vni 42) [ VM 1 ]-----|           |      |           |-----[ VM 2 ] (vni 42)
                        | Butterfly +======+ Butterfly |
                        |           |      |           |
                        +-----------+      +-----------+
```

This scenario check if exotic protocol filtering like sctp are managed by Butterfly.

Initial setup:
- 2 butterfly
- VM1 configured on vni 42 with security group sg-1
- VM2 configured on vni 42 with security group sg-1
- sg-1 has no rules

Test that:
- Test SCTP connection VM1 -> VM2: KO
- Test SCTP connection VM2 -> VM1: KO

Change setup:
- Add rule to sg-1 allowing all protocols (-1) from all hosts

Test that:
- Test SCTP connection VM1 -> VM2: OK
- Test SCTP connection VM2 -> VM1: OK

Change setup:
- Remove rule to sg-1 allowing all protocols (-1) from all hosts

Test that:
- Test SCTP connection VM1 -> VM2: KO
- Test SCTP connection VM2 -> VM1: KO
