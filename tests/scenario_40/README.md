# Description

This scenario will check single IP update through NicUpdateReq.

Initial setup:
- VM1 starts with IPv4 a1 and Ipv6 A1
- VM2 starts with Ipv4 b1

Test that:
- ping a1 --> b1 OK

Change setup
- Update VM2 ip: VM2 has now only Ipv4 b2

Test that:
- ping a1 --> b1 KO
- ping a1 --> b2 OK

Change setup
- Update VM2 ip: VM2 has now only Ipv4 b1

Test that:
- ping a1 --> b1 OK
- ping a1 --> b2 KO

Change setup
- Update VM2 ip: VM2 has now only Ipv6 B1

Test that:
- ping a1 --> b1 KO
- ping a1 --> b2 KO
- ping A1 --> B1 OK

Change setup
- Update VM2 ip: VM2 has now only Ipv4 b1

Test that:
- ping a1 --> b1 OK
- ping a1 --> b2 KO
- ping A1 --> B1 KO

Change setup
- Update VM2 ip: VM2 has now only Ipv6 B2

Test that:
- ping a1 --> b1 KO
- ping a1 --> b2 KO
- ping A1 --> B1 KO
- ping A1 --> B2 OK

Change setup
- Update VM2 ip: VM2 has now only Ipv6 B1

Test that:
- ping a1 --> b1 KO
- ping a1 --> b2 KO
- ping A1 --> B1 OK
- ping A1 --> B2 KO
