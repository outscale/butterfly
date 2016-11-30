#Description

```
+-----------+
|           |-----------[ VM 1 ] (vni 42)
| Butterfly |
|           |-----------[ VM 2 ] (vni 42)
+-----------+

```

This scenario test VNIC connectivity when deleted VNIC or VM

Initial setup:
- 1 butterfly
- VM1 configured on vni 42 with security group sg-1
- VM2 configured on vni 42 with security group sg-1
- sg-1 has one rule full opened

Test that:
- ping communication VM1 -> VM2 is OK
- ping communication VM2 -> VM1 is OK

Change setup:
-Remove NIC1
-Remove NIC2

Test that:
- ping communication VM1 -> VM2 is KO
- ping communication VM1 -> VM2 is KO

Change setup:
- Re-Add NIC1
- Re-Add NIC2

Test that:
- ping communication VM1 -> VM2 is KO
- ping communication VM1 -> VM2 is KO

Change setup:
- Stopping VM1
- Stopping VM2
- restarting VM1
- restarting VM2

Test that:
- ping communication VM1 -> VM2 is OK
- ping communication VM2 -> VM1 is OK
