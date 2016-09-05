# Description

```
+-----------+
|           |-----------[ VM 1 ] (vni 42)
| Butterfly |
|           |-----------[ VM 2 ] (vni 42)
+-----------+

```

- This test start a butterfly server with two nic A (SG1) and B (SG2) with different sg.
- Add A's IP as a member of SG1
- Add B's IP as a member of SG2
- SG1 rule: Allow all members of SG1 to access UDP port 8000
- SG2 rule: Allow all members of SG2 to access UDP port 9000
- Make a bunch of network tests (tcp and udp).

- Add A's IP as a member of SG2
- Add B's IP as a member of SG1
- Make a bunch of network tests (tcp and udp).

- remove A's IP from members of SG2
- remove B's IP from members of SG1
- Make a bunch of network tests (tcp and udp).
