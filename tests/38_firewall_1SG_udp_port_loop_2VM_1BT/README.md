# Description

```
+-----------+
|           |-----------[ VM 1 ] (vni 42)
| Butterfly |
|           |-----------[ VM 2 ] (vni 42)
+-----------+

```

This scenario test that deleting a SG block all traffic,
and adding it again allow traffic.

Initial setup:
- VM1 configured on vni 42 with security group sg-1
- VM2 configured on vni 42 with security group sg-1
- sg-1 has no rules configured

Loop to 10:

Test that:
- UDP communication VM1 -> VM2 is KO
- UDP communication VM2 -> VM1 is KO

Change setup:
- Add rule to sg-1: allow all hosts on UDP port 6000

Test that:
- UDP communication VM1 -> VM2 is OK
- UDP communication VM2 -> VM1 is OK

Change setup:
- delete sg-1

Test that:
- TCP communication VM1 -> VM2 is KO
- TCP communication VM2 -> VM1 is KO

Change setup:
- Add rule to sg-1: allow all hosts on TCP port 6000

Test that:
- UDP communication VM1 -> VM2 is OK
- UDP communication VM2 -> VM1 is OK

Change setup:
- delete sg-1
