# Description

```
+-----------+
|           |-----------[ VM 1 ] (vni 42)
| Butterfly |
|           |-----------[ VM 2 ] (vni 42)
+-----------+

```

Initial setup:
- VM1 configured on vni 42 with security group sg-1
- VM2 configured on vni 42 with security group sg-1
- sg-1 doesn't have a rule

Test that:
- UDP communication on port 5554 VM1 -> VM2 KO
- UDP communication on port 5554 VM2 -> VM1 KO
- TCP communication on port 4445 VM2 -> VM1 KO
- TCP communication on port 4445 VM1 -> VM2 KO

Change setup:
- Add a rule in sg-1 full open

Test that:
- UDP communication on port 4445 VM2 -> VM1 OK
- UDP communication on port 5554 VM1 -> VM2 OK
- TCP communication on port 5554 VM1 -> VM2 OK
- TCP communication on port 3485 VM2 -> VM1 OK
