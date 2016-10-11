# Description

```
+-----------+
|           |-----------[ VM 1 ] (vni 42)
| Butterfly |
|           |-----------[ VM 2 ] (vni 42)
+-----------+

```

This scenario test ping communication through two butterfly.

Initial setup:
- VM1 configured on vni 42 with security group sg-1
- VM2 configured on vni 42 with security group sg-1
- sg-1 has one rule full opened

Test that:
- ping communication VM1 -> VM2 is OK
- ping communication VM2 -> VM1 is OK

Change step:
- Stopping VM1
- restarting VM1

Test that:
- ping communication VM1 -> VM2 is OK
- ping communication VM2 -> VM1 is OK

Change step:
- Stopping VM1
- restarting VM1

Test that:
- ping communication VM1 -> VM2 is OK
- ping communication VM2 -> VM1 is OK
