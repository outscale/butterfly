# Description

This scenario try to create a NIC without IP and to add one through NicUpdateReq
- VM1 has 1 Ipv4 a1
- Nic of VM2 is created without IP
- VM2 is launched and configured with Ipv4 b1

1. Test that:
- ping a1 -> b1 KO

2. Change setup:
- Add Ipv4 b1 to VM2 in butterfly using NicUpdateReq

3. Test that:
- ping a1 -> b1 OK

4. Change setup:
- Remove Ipv4 b1 from VM2 in butterfly using NicUpdateReq

5. Loop to 1. ten times
