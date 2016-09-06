# Description

```
+-----------+
|           |-----------[ VM 1 ] (vni 42)
| Butterfly |
|           |-----------[ VM 2 ] (vni 42)
+-----------+
```
The goal of this test is to check that firewall rules are applied on security
group modification: add / remove rule, and when security group is removed.

Initial setup:
- VM1 configured on vni 42 with security group sg-1
- VM2 configured on vni 42 with security group sg-1
- sg-1 has no rules configured

Test that:
- ping VM1 -> VM2 is KO
- ping VM2 -> VM1 is KO
- TCP VM1 -> VM2 is KO
- TCP VM2 -> VM1 is KO
- UDP VM1 -> VM2 is KO
- UDP VM2 -> VM1 is KO

Change setup:
- sg-1 is now full open

Test that:
- ping VM1 -> VM2 is OK
- ping VM2 -> VM1 is OK
- TCP VM1 -> VM2 is OK
- TCP VM2 -> VM1 is OK
- UDP VM1 -> VM2 is OK
- UDP VM2 -> VM1 is OK

Change setup:
- sg-1 is deleted

Test that:
- ping VM1 -> VM2 is KO
- ping VM2 -> VM1 is KO
- TCP VM1 -> VM2 is KO
- TCP VM2 -> VM1 is KO
- UDP VM1 -> VM2 is KO
- UDP VM2 -> VM1 is KO

Change setup:
- Add rule to sg-1: allow all on TCP port 8000

Test that:
- ping VM1 -> VM2 is KO
- ping VM2 -> VM1 is KO
- TCP VM1 -> VM2 is OK
- TCP VM2 -> VM1 is OK
- UDP VM1 -> VM2 is KO
- UDP VM2 -> VM1 is KO

Change setup:
- Remove rule from sg-1: allow all on TCP port 8000
- Add rule to sg-1: allow all on UDP port 8000

Test that:
- ping VM1 -> VM2 is KO
- ping VM2 -> VM1 is KO
- TCP VM1 -> VM2 is KO
- TCP VM2 -> VM1 is KO
- UDP VM1 -> VM2 is OK
- UDP VM2 -> VM1 is OK
