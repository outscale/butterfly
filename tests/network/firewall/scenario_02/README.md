# Description

```
+-----------+
|           |-----------[ VM 1 ] (vni 42) (sg-1)
| Butterfly |-----------[ VN 2 ] (vni 42) (sg-1)
|           |-----------[ VM 3 ] (vni 42) (sg-2)
+-----------+

```

This scenario test rules based on members of security groups.

Initial setup:
- 1 butterfly
- VM1 configured on vni 42 with security group sg-1
- VM2 configured on vni 42 with security group sg-1
- VM3 configured on vni 42 with security group sg-2
- sg-1 and sg-2 have no rules configured
- Add VM1's ip to SG1 members
- Add VM2's ip to SG1 members
- Add VM3's ip to SG2 members
- Add one rule to sg-1 allowing sg-2 members on UDP port 8000

Test that:
- UDP communication on port 8000 VM3 -> VM1 OK
- UDP communication on port 8000 VM3 -> VM2 OK
- UDP communication on port 8000 VM1 -> VM3 KO
- UDP communication on port 8000 VM2 -> VM3 KO
- UDP communication on port 8000 VM1 -> VM2 KO
- UDP communication on port 8000 VM2 -> VM1 KO

Change setup:
- Add new rule to sg-1 allowing sg-1 members on UDP port 8000

Test that:
- UDP communication on port 8000 VM3 -> VM1 OK
- UDP communication on port 8000 VM3 -> VM2 OK
- UDP communication on port 8000 VM1 -> VM3 KO
- UDP communication on port 8000 VM2 -> VM3 KO
- UDP communication on port 8000 VM1 -> VM2 OK
- UDP communication on port 8000 VM2 -> VM1 OK

Change setup:
- Remove SG from VM2
- Remove VM2's IP of sg-1 members

Test that:
- UDP communication on port 8000 VM3 -> VM1 OK
- UDP communication on port 8000 VM3 -> VM2 KO
- UDP communication on port 8000 VM1 -> VM3 KO
- UDP communication on port 8000 VM2 -> VM3 KO
- UDP communication on port 8000 VM1 -> VM2 KO
- UDP communication on port 8000 VM2 -> VM1 KO