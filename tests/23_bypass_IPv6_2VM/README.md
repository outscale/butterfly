# Description

```
+-----------+
|           |-----------[ VM 1 ] (vni 42)
| Butterfly |
|           |-----------[ VM 2 ] (vni 42)
+-----------+

```

This scenario test IPv6 pass through an other VM

Initial setup:
- VM1 configured on vni 42 with security group sg-1
- VM2 configured on vni 42 with security group sg-2
- sg-1 has no rules

Test that:
- ping6 communication VM1 -> VM2 is KO
- Ping6 communication VM2 -> VM1 is KO

Change setup:
- Add rule to sg-1 allowing IPv6 address of VM1 on all protocols
- Add rule to sg-1 allowing IPv6 address of VM2 on all protocols

Test that:
- ping6 communication VM1 -> VM2 is OK
- ping6 communication VM2 -> VM1 is OK

Change setup:
- Remove rule from sg-1 allowing IPv6 address of VM1 on all protocols
- Remove rule from sg-1 allowing IPv6 address of VM2 on all protocols 

Test that:
- ping6 communication VM1 -> VM2 is KO
- ping6 communication VM2 -> VM1 is KO

Change setup:
- Add rule to sg-1 allowing IPv6 address of VM1 on TCP port 8000

Test that:
- TCPv6 communication on port 8000 VM1 -> VM2 is OK
- TCPv6 communication on port 8000 VM2 -> VM1 is KO

Change setup:
- Remove rule from sg-1 allowing IPv6 address of VM1 on TCP port 8000
- Add rule to sg-1 allowing IPv6 address of VM1 on UDP port 8000

Test that:
- UDPv6 communication on port 8000 VM1 -> VM2 is OK
- UDPv6 communication on port 8000 VM2 -> VM1 is KO

Change_setup:
- Remove sg-1

Test that:
- ping6 communication VM1 -> VM2 is KO
- ping6 communication VM2 -> VM1 is KO
- TCPv6 communication on port 8000 VM1 -> VM2 is KO
- TCPv6 communication on port 8000 VM2 -> VM1 is KO
- UDPv6 communication on port 8000 VM1 -> VM2 is KO
- UDPv6 communication on port 8000 VM2 -> VM1 is KO
