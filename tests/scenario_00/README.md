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

Test that:
- ping communication VM1 -> VM2 is KO
- ping communication VM2 -> VM1 is KO

Change setup:
- sg-1 allow all protocols from any hosts

Test that:
- ping communication VM1 -> VM2 is OK
- ping communication VM2 -> VM1 is OK

Change setup:
- sg-1 remove rule allowing all protocols from any hosts

Test that:
- ping communication VM1 -> VM2 is KO
- ping communication VM2 -> VM1 is KO

Change setup:
- sg-1 add rule allowing only ICMP from all hosts

Test that:
- ping communication VM1 -> VM2 is OK
- ping communication VM2 -> VM1 is OK

Change setup:
- sg-1 remove rule allowing only ICMP from all hosts

Test that:
- ping communication VM1 -> VM2 is KO
- ping communication VM2 -> VM1 is KO

