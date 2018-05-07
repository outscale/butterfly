# Description

```
+-----------+
|           |-----------[ VM 1 ] (vni 42)
| Butterfly |
|           |-----------[ VM 2 ] (vni 42)
+-----------+

```

This scenario test security group without rules.

Initial setup:
- VM1 configured on vni 42 with security group sg-1
- VM2 configured on vni 42 with security group sg-2
- Add VM1's IP to sg-1 members
- sg-2 has a rule: allow all sg-1 members on UDP port 8000

Test that:
- UDP communication on port 8000 VM1 -> VM2 is OK
- UDP communication on port 8000 VM2 -> VM1 is KO

Change setup:
- Remove VM1's IP from sg-1 members

Test that:
- UDP communication on port 8000 VM1 -> VM2 is KO
- UDP communication on port 8000 VM2 -> VM1 is KO

Change setup:
- Add VM1's IP from sg-1 members

Test that:
- UDP communication on port 8000 VM1 -> VM2 is OK
- UDP communication on port 8000 VM2 -> VM1 is KO
