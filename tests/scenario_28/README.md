# Description

```
+-----------+
|           |-----------[ VM 1 ] (vni 42)
| Butterfly |
|           |-----------[ VM 2 ] (vni 42)
+-----------+

```

This scenario just connects two virtual machines with the same
security groups no rule in the SG on the same butterfly.

- Start the butterfly server and the two virtual machines;
- Tested that VM1 and VM2 can't send the UDP/TCP message and can't
make ping test between them;

- Add a rule in security groupe full opened;
- Tested that VM1 and VM2 can send the UDP/TCP message and can make
ping test between them;

- Remove this rule;
- Tested that VM1 and VM2 can't send the UDP/TCP message and can't
make ping test between them;

- Add a new rule in SG listening on the port 8000 for only TCP message;
- Tested that VM1 and VM2 can't send the UDP message and
make ping test but can send TCP message between them;

- Remove this rule;
- Add a new rule in SG listening on the port 8000 for only UDP message;
- Tested that VM1 and VM2 can't send the TCP message and
make ping test but can send UDP message between them;

- Stop the two virtual machines;
- Stop the butterfly server.
