# Description

```
+-----------+
|           |-----------[ VM 1 ] (vni 42)
| Butterfly |
|           |-----------[ VM 2 ] (vni 42)
+-----------+

```

This scenario test that overwriting a whole SG will change rules
(adding new rules, removing old ones and keep the same rules)

Initial setup
- VM1 configured on vni 42 with security group sg-1
- VM1 configured on vni 42 with security group sg-1
- sg-1 has no rules configured

Loop to 10:

Test that:
- UDP communication VM1 -> VM2 is KO
- UDP communication VM2 -> VM1 is KO
- TCP communication VM1 -> VM2 is KO
- TCP communication VM2 -> VM1 is KO

Change setup:
- Add rule full opened to sg-1

Test that:
- UDP communication VM1 -> VM2 is OK
- UDP communication VM2 -> VM1 is OK
- TCP communication VM1 -> VM2 is OK
- TCP communication VM2 -> VM1 is OK

Change setup:
- Overwrite sg-1 has no rules configured