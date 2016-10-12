# Description

```
+-----------+
|           |-----------[ VM 1 ] (vni 42)
| Butterfly |
|           |-----------[ VM 2 ] (vni 42)
+-----------+

```

This scenario test cumulates security group rules.

Initial setup:
- 1 butterfly
- VM1 configured on vni 42 with security group sg-1
- VM2 configured on vni 42 without security group
- sg-1 has a rule allow all hosts on UDP port 8000
- sg-2 has a rule allow all hosts on UDP port 9000

Test that:
- UDP communication on port 8000 VM2 -> VM1 OK
- UDP communication on port 9000 VM2 -> VM1 KO

Change setup:
- Add sg-1 and sg-2 to VM1

Test that:
- UDP communication on port 8000 VM2 -> VM1 OK
- UDP communication on port 9000 VM2 -> VM1 OK

Change setup:
- Remove sg-1 and sg-2 from VM1
- Add sg-2 to VM1

Test that:
- UDP communication on port 8000 VM2 -> VM1 KO
- UDP communication on port 9000 VM2 -> VM1 OK

Change setup:
- Remove sg-2 from VM1

Test that:
- UDP communication on port 8000 VM2 -> VM1 OK
- UDP communication on port 9000 VM2 -> VM1 OK
