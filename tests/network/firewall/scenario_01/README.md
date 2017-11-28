# Description

```
+-----------+
|           |-----------[ VM 1 ] (vni 42)
| Butterfly |
|           |-----------[ VM 2 ] (vni 42)
+-----------+

```

This scenario test adding and deleting security groups members.

Initial setup:
- VM1 configured on vni 42 with security group sg-1
- VM2 configured on vni 42 with security group sg-2
- sg-1 and sg-2 have no rules configured
- Add VM1's IP as a member of SG1
- Add VM2's IP as a member of SG2
- Add one rule to sg-1 allowing sg-1 members on UDP port 8000
- Add one rule to sg-2 allowing sg-2 members on UDP port 9000

Test that:
- UDP communication on port 9000 VM1 -> VM2 is KO
- UDP communication on port 8000 VM2 -> VM1 is KO

Change setup:
- Add VM2's IP as a member of SG1
- Add VM1's IP as a member of SG2

Test that:
- UDP communication on port 9000 VM1 -> VM2 is OK
- UDP communication on port 8000 VM2 -> VM1 is OK

Change setup:
- Remove VM2's IP from SG1 members
- Remove VM1's IP from SG2 members

Test that:
- UDP communication on port 9000 VM1 -> VM2 is KO
- UDP communication on port 8000 VM2 -> VM1 is KO
