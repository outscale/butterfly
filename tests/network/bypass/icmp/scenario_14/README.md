# Description

```
+-----------+
|           |-----------[ VM 1 ] (vni 42)
| Butterfly |
|           |-----------[ VM 2 ] (vni 42)
+-----------+

```

This test check that we can delete and add again nics.

Initial setup:
- VM1 configured on vni 42 with security group sg-1
- VM2 configured on vni 42 with security group sg-1
- sg-1 has one rule full opened

Loop to 10:

Test that:
- Add VM1 to butterfly server is OK
- Add VM2 to butterfly server is OK
- ping communication VM1 -> VM2 is OK
- ping communication VM1 -> VM2 is OK
- stop VM1
- stop VM2
- Delete VM1
- Delete VM2
