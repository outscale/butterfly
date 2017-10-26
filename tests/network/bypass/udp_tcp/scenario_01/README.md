# Description

```
+-----------+
|           |-----------[ VM 1 ] (vni 42)
| Butterfly |
|           |-----------[ VM 2 ] (vni 42)
+-----------+

```

This scenario test stateful firewall.

Initial setup:
- VM1 configured on vni 42 without security group
- VM2 configured on vni 42 with security group sg-1
- sg-1 is full open

Test that:
- UDP communication on port 5543 VM1 -> VM2 OK
- TCP communication on port 1243 VM1 -> VM2 OK
- UDP communication on port 1254 VM2 -> VM1 KO
- TCP communication on port 1234 VM2 -> VM1 KO
