# Description

```
+-----------+
|           |-----------[ VM 1 ] (vni 42)
| Butterfly |
|           |-----------[ VM 2 ] (vni 42)
+-----------+

```

This scenario just connects two virtual machines with security group
full opened on the same butterfly on the same network (vni).

- Start the butterfly server and the two virtual machines;

- Tested that VM1 and VM2 can send the UDP message between them;
- Checked every time if the UDP test failed or no;
Those operations are done 50 times.

- Tested that VM1 and VM2 can send the TCP message between them;
- Checked every time if the TCP test failed or no;
Those operations are done 50 times.

- Stop the two virtual machines;
- Stop the butterfly server.
