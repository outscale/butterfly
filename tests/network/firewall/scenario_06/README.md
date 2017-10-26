# Description

```
+-----------+
|           |-----------[ VM 1 ] (vni 42)
| Butterfly |
|           |-----------[ VM 2 ] (vni 42)
+-----------+

```

This scenario test iperf and iperf3 UDP/TCP between two machines on the same butterfly.

Initial setup:
- VM1 configured on vni 42 with security group sg-1
- VM2 configured on vni 42 with security group sg-1
- sg-1 is full open

Test that:
- iperf TCP communication VM1 -> VM2 OK
- iperf TCP communication VM2 -> VM1 OK
- iperf UDP communication VM1 -> VM2 OK
- iperf UDP communication VM2 -> VM1 OK
- iperf3 TCP communication VM1 -> VM2 OK
- iperf3 TCP communication VM2 -> VM1 OK
- iperf3 UDP communication VM1 -> VM2 OK
- iperf3 UDP communication VM2 -> VM1 OK
