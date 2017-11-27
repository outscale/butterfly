# Description

```
+-----------+
|           |-----------[ VM 1 ] (vni 42)
| Butterfly |
|           |-----------[ VM 2 ] (vni 42)
+-----------+

```

Initial setup:
- VM1 configured on vni 42 without security group
- VM2 configured on vni 42 without security group

Test that:
- TCP communication on port 4454 VM2 -> VM1 KO
- TCP communication on port 4454 VM1 -> VM2 KO

Change setup:
- Add sg-1 to VM1 and VM2 without rule

Loop to 3:

Test that:
- TCP communication on port 4454 VM2 -> VM1 KO
- TCP communication on port 4454 VM1 -> VM2 KO

Change setup:
- Add rule to sg-1: allow all hosts on TCP port 4454

Test that:
- TCP communication on port 4454 VM2 -> VM1 OK
- TCP communication on port 4454 VM1 -> VM2 OK

Change setup:
- Delete rule to sg-1: allow all hosts on UDP port 4454

Test that:
- TCP communication on port 4454 VM2 -> VM1 KO
- TCP communication on port 4454 VM1 -> VM2 KO

Change setup:
- Add rule to sg-1: allow all hosts on UDP port 4454

Test that:
- UDP communication on port 4454 VM2 -> VM1 OK
- UDP communication on port 4454 VM1 -> VM2 OK

Change setup:
- Delete sg-1

Test that:
- TCP communication on port 4454 VM2 -> VM1 KO
- TCP communication on port 4454 VM1 -> VM2 KO
- UDP communication on port 4454 VM2 -> VM1 OK
- UDP communication on port 4454 VM1 -> VM2 OK
