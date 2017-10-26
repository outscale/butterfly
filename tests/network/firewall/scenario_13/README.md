# Description

Test dump/load of Butterfly configuration.

Initial setup:
- butterfly starts
- a complex configuration is pushed to butterfly (several nics, sg and rules)
- a configuration dump is performed to dump1
- shutdown butterfly
- butterfly stats
- load configuration from dump to the freshly spawned Butterfly
- dump configuration to dump2

Test that
- dump1 and dump2 are the same
