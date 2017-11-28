# Description

```
+-----------+
|           |-----------[ VM 1 ] (vni 42)
| Butterfly |
|           |-----------[ VM 2 ] (vni 42)
+-----------+

```

This scenario test that network communication is OK on a protocol and port opened, is KO on others protocol and port.

Initial setup:
- VM1 configured on vni 42 with security group sg-1
- VM2 configured on vni 42 with security group sg-1
- Add one rule to sg-1 allowing UDP communication on port 5554

Test that:
- UDP communication on port 5554 VM1 -> VM2 OK
- UDP communication on port 5554 VM2 -> VM1 OK
- TCP communication on port 4445 VM2 -> VM1 KO
- TCP communication on port 5554 VM2 -> VM1 KO
- TCP communication on port 1002 VM1 -> VM2 KO
- UDP communication on port 1234 VM2 -> VM1 KO
- TCP communication on port 5895 VM1 -> VM2 KO
- UDP communication on port 5553 VM2 -> VM1 KO
- UDP communication on port 5555 VM1 -> VM2 KO
