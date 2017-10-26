# Description

```
+-----------+
|           |-----------[ VM 1 ] (vni 42)
| Butterfly |-----------[ VM 2 ] (vni 42)
|           |-----------[ VM 3 ] (vni 42)
+-----------+

```

Initial setup:
- VM1 configured on vni 42 with security group sg-1
- VM2 configured on vni 42 with security group sg-1
- VM3 configured on vni 42 with security group sg-1
- sg-1 doesn't have a rule

Test that:
- UDP communication on port 8000 VM3 -> VM1 KO
- UDP communication on port 8000 VM3 -> VM2 KO

Change setup:
- Add rule to sg-1: allow all hosts on UDP port 8000

Test that:
- UDP communication on port 8000 VM3 -> VM1 OK
- UDP communication on port 8000 VM3 -> VM2 OK

Change setup:
- Delete rule to sg-1: allow all hosts on UDP port 8000

Test that:
- UDP communication on port 8000 VM3 -> VM1 KO
- UDP communication on port 8000 VM3 -> VM2 KO

Change setup:
- Add rule to sg-1: allow all hosts on UDP port 9000

Test that:
- UDP communication on port 9000 VM3 -> VM1 OK
- UDP communication on port 9000 VM3 -> VM2 OK

Change setup:
- Delete rule to sg-1: allow all hosts on UDP port 9000
- Add rule to sg-1: allow all hosts on TCP port 8000

Test that:
- TCP communication on port 8000 VM3 -> VM1 OK
- TCP communication on port 8000 VM3 -> VM2 OK

Change setup:
- Delete rule to sg-1: allow all hosts on TCP port 8000

Test that:
- TCP communication on port 8000 VM3 -> VM1 KO
- TCP communication on port 8000 VM3 -> VM2 KO

Change setup:
- Add rule to sg-1: allow all hosts on TCP port 9000

Test that:
- TCP communication on port 9000 VM3 -> VM1 OK
- TCP communication on port 9000 VM3 -> VM2 OK
