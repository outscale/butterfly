Butterfly
=========
Butterfly connects Virtual Machines (VM) and controls their traffic flow.

Each VM traffic is contained in a specific [VXLAN](https://en.wikipedia.org/wiki/Virtual_Extensible_LAN)
network and traffic is filtered by (EC2/Openstack-like) security groups.

Security groups can be applied to any VM interface, and contain a list of simple
network rules (dropping traffic by default).


# Virtual NICs

In Butterfly, a Virtual NIC (or vnic) enables you to add a virtual network
interface to your Qemu VM through vhost-user.
Each vnic has a 24 bit network id called VNI.
If two vnics with the same VNI are located on different physical hosts,
Butterfly will encapsulate VM packets over VXLAN and send them to the
corresponding physical host.
Once received, packets will be decapsulated and routed to their final
destination.
All vnics created with the same VNI are located on the same network.
If two vnics with the same VNI are on the same physical host, then packets do
not exit to physical network.

Butterfly is meant to be connected to the physical network using a dedicated
[DPDK](http://dpdk.org/) port.
It allows Butterfly to have a very low latency between VMs while using physical
NIC offload capabilities.

For VM-to-VM communication, checksum and segmentation do not occurs as packets
do not transit on a physical network. This enables Butterfly to have a high speed
and low latency communication between VMs.

Example: create of a new vnic "vnic_1" on vni "1337":
```
butterfly nic add --ip 42.0.0.1 --mac 52:54:00:12:34:01 --vni 1337 --id vnic_1
```

# Filtering

VMs traffic is filtered using an integrated firewall within Butterfly
([NetBSD's NPF](http://www.netbsd.org/~rmind/npf/)) for each vnic.
Filtering rules are applied to each VM depending on the rules contained in its _Security Groups_.
A vnic can use several security groups and a security group can be used by several vnics.
When a vnic use several security group, then rules are cumulated.
A security group contains a list of rules to allow (default policy is to block) and a list
of members (IP addresses).

A Butterfly rule is mainly described by a protocol/port and source to allow.
This source can be either a CIDR block _or_ members of a security group.

Example: Add a rule in the "mysg" security group that allows 42.0.3.1 in TCP protocol on port 22:
```
butterfly sg rule add mysg --ip-proto tcp --port 22 --cidr 42.0.3.1/32
```

Example: Add a rule in the "mysg" security group that allows "users" security group members in TCP protocol on port 80:
```
butterfly sg rule add mysg --ip-proto tcp --port 80 --sg-members users
```

Note: When a security group used by one or more vnics is modified, firewalling rules
attached to each impacted VM are reloaded.

# Using Butterfly

Butterfly is a daemon you can control over a network API.

It is packaged with a client mainly allowing you to add/remove/list vnics and security groups.

You can of course directly code your calls to Butterfly's API.
API message transport is based on [ZeroMQ](http://zeromq.org/) and messages
are encoded in [Protobuf](https://github.com/google/protobuf/ "Google's protobuf")
format. Check out [protocol](https://github.com/outscale/butterfly/tree/master/api/protocol)
for more details.

Here is an example of Butterfly with 6 VMs isolated in three networks (VNI 42, 51 and 1337).

![Butterfly execution](https://osu.eu-west-2.outscale.com/jerome.jutteau/16d1bc0517de5c95aa076a0584b43af6/butterfly.svg)

Butterfly binds a dedicated NIC to send/receive VXLAN packets and binds a socket
(default: tcp) to listen to queries on its API. If you use a DPDK compatible
card, you will not be able to access the API through it.

You can build this configuration using a few lines of client calls:
```
butterfly nic add --ip 42.0.0.1 --mac 52:54:00:12:34:01 --vni 42 --id vnic_1
butterfly nic add --ip 42.0.0.1 --mac 52:54:00:12:34:01 --vni 51 --id vnic_2
butterfly nic add --ip 42.0.0.2 --mac 52:54:00:12:34:02 --vni 51 --id vnic_3
butterfly nic add --ip 42.0.0.3 --mac 52:54:00:12:34:03 --vni 51 --id vnic_4
butterfly nic add --ip 42.0.0.1 --mac 52:54:00:12:34:01 --vni 1337 --id vnic_5
butterfly nic add --ip 42.0.0.2 --mac 52:54:00:12:34:02 --vni 1337 --id vnic_6
```

Tip: if you want to see what the graph looks like: run `butterfly status` and copy past the dot diagram in [webgraphviz.com](http://www.webgraphviz.com/)

You can edit security groups whenever you want, which automatically updates vnics filtering.
In the following example, we create a new rule to allow everyone in http protocol and ask some vnics to use this security group.
```
butterfly sg add sg-web
butterfly sg rule add sg-web --ip-proto tcp --port 80 --cidr 0.0.0.0/0
butterfly nic sg add vnic_1 sg-web
butterfly nic sg add vnic_2 sg-web
```
Note: Butterfly API uses idempotence, meaning that two calls should produce the
same result.

# Installing Butterfly

The easiest way to install Butterfly is to download and install a package from [github releases](https://github.com/outscale/butterfly/releases).
You can also build Butterfly yourself (as shown in the next section) and run `make install` .

# Building Butterfly

This building procedure has been tested on a fresh Centos7.

First, install some dependencies (jemalloc needs manual installation:
```
$ sudo yum update -y
$ sudo yum install -y gcc-c++ glibc-devel glib2-devel libtool libpcap-devel automake kernel-headers make git cmake kernel-devel unzip zlib-devel wget libstdc++-static
$ wget http://cbs.centos.org/kojifiles/packages/jemalloc/3.6.0/8.el7.centos/x86_64/jemalloc-devel-3.6.0-8.el7.centos.x86_64.rpm
$ wget http://cbs.centos.org/kojifiles/packages/jemalloc/3.6.0/8.el7.centos/x86_64/jemalloc-3.6.0-8.el7.centos.x86_64.rpm
$ sudo rpm -i jemalloc-devel-3.6.0-8.el7.centos.x86_64.rpm jemalloc-3.6.0-8.el7.centos.x86_64.rpm
```

Build Butterfly:
```
$ git clone https://github.com/outscale/butterfly.git
$ mkdir butterfly/build
$ cd butterfly/build
$ cmake ..
$ make
```

# Preparing Your Machine

## Configure Huge Pages

Butterfly needs some [huge pages](https://en.wikipedia.org/wiki/Page_%28computer_memory%29#Huge_pages)
(adjust to your needs):

- Edit your `/etc/sysctl.conf` and add some huge pages:
```
vm.nr_hugepages=2000
```
- Reload your sysctl configuration:
```
$ sudo sysctl -p /etc/sysctl.conf
```
- Check that your huge pages are available:
```
$ cat /proc/meminfo | grep Huge
```
- Mount your huge pages:
```
$ sudo mkdir -p /mnt/huge
$ sudo mount -t hugetlbfs nodev /mnt/huge
```
- (optional) Add this mount in your `/etc/fstab`:
```
hugetlbfs       /mnt/huge  hugetlbfs       rw,mode=0777        0 0
```

## Prepare DPDK Compatible NIC

Before being able to bind your port, you need to enable Intel VT-d in your BIOS and have IOMMU explicitly enabled in your kernel parameters.
Check [DPDK compatible NICs](http://dpdk.org/doc/nics) and how to [bind NIC drivers](http://people.redhat.com/~pmatilai/dpdk-guide/setup/binding.html).
Packetgraph also has an [example](https://github.com/outscale/packetgraph/tree/master/examples/firewall#configure-your-nics) on how to bind DPDK NICs.

Additionally, you may also want to isolate a specific core for Butterfly, check isolcpus [kernel parameters](https://www.kernel.org/doc/Documentation/kernel-parameters.txt).

# Running Butterfly Server

To get help, see: `butterflyd --help`

For example, if you have a DPDK compatible NIC, Butterfly will use the first
available DPDK port. If no port is found, a (slow) tap interface is created.
```
$ sudo butterflyd -i 192.168.0.1 -s /tmp
```

If you do not have a DPDK compatible card, you can also init a DPDK virtual
device (which is _much_ slower than a DPDK compatible hardware).

For example, we can ask Butterfly to listen to the existing `eth0` interface:
```
$ sudo butterflyd -i 192.168.0.1 -s /tmp --dpdk-args "-c1 -n1 --socket-mem 64 --vdev=eth_pcap0,iface=eth0"
```

Alternatively, you can ask Butterfly to read a [configuration file]
(https://github.com/outscale/butterfly/blob/master/api/server/butterflyd.ini)
at init:
```
$ sudo butterflyd -c /etc/butterfly/butterfly.conf
```

# F.A.Q.

## Why Another Virtual Switch?

Because we just want a fast vswitch answering our simple needs:
- _simple_ API: EC2/Openstack security groups style
- Have some VXLAN for network isolation
- Have some firewalling per Virtual Machine based on security groups
- Use as little CPU as possible (and let Virtual Machines use all other cores)
- Ease a (Cloud) orchestrator to control the whole thing through a simple API

## What's Behind Butterfly?

Butterfly is based on:
- [Packetgraph](http://github.com/outscale/packetgraph): creates network graph
- [DPDK](http://dpdk.org/): fast access to NICs (in Packetgraph)
- [NPF](http://www.netbsd.org/~rmind/npf/): firewalling (in Packetgraph)
- [ZeroMQ](http://zeromq.org/): message transport
- [Protobuf](https://github.com/google/protobuf/ "Google's protobuf"): message encoding/versioning

## How Fast?

Benchmarks setup:
- Two physical machines directly connected
- A third machine remote setup and launch benchmarks using `./benchmarks/run_benchmarks.sh`

Machines:
- OS: Centos 7 (3.10.0-327.18.2.el7.x86_64)
- NICs: Intel 82599ES 10-Gigabit SFI/SFP+ (DPDK compatible used with vfio-pci driver)
- CPU: AMD Opteron(tm) Processor 3350 HE
- All details for [host 1](https://osu.eu-west-2.outscale.com/jerome.jutteau/16d1bc0517de5c95aa076a0584b43af6/butterfly_1.txt) and [host 2](https://osu.eu-west-2.outscale.com/jerome.jutteau/16d1bc0517de5c95aa076a0584b43af6/butterfly_2.txt) (same)

Results (april 2017, 60 seconds per tests):
```
                         |   VMs on same host  |  VMs on remote host |
 ------------------------+---------------------+---------------------|
| Ping (min/average/max) | 0.046/0.69/0.115ms | 0.158/0.179/0.220ms |
| TCP                    |     18.9 Gbits/s    |     5.0 Gbits/s     |
| UDP                    |     1.6  Gbits/s    |     1.6 Gbits/s     |
```

Notes:
- UDP is really _bad_ at the moment, we are working on it
- We can get even faster with zero copy in vhost-user
- We can get faster by embedding a more recent libc (`make package-fat`)
- If you try to run some benchmarks, you may want to configure your [CPU throttling](https://en.wikipedia.org/wiki/Dynamic_frequency_scaling). On Centos7, check [cpufreq governors page](https://access.redhat.com/documentation/en-US/Red_Hat_Enterprise_Linux/7/html/Power_Management_Guide/cpufreq_governors.html)

Well, we still have a good margin for improvements :)

## How to Connect a Virtual Machine to Butterfly?

Butterfly does not launch your Virtual Machine for you, it just creates a special
network interface (vhost-user) so you can connect your Virtual Machine to it.
Vhost-user interfaces are Unix sockets allowing you to directly communicate with
Virtual Machines in userland.

We tested Butterfly with QEMU >= 2.5 and added the following parameters to
the machine's arguments (to adapt):

Some shared memory between the guest and Butterfly:
```
-object memory-backend-file,id=mem,size=124M,mem-path=/mnt/huge,share=on -numa node,memdev=mem -mem-prealloc
```

For each network interface:
```
-chardev socket,id=char0,path=/path/to/socket -netdev type=vhost-user,id=mynet0,chardev=char0,vhostforce -device virtio-net-pci,netdev=mynet0,gso=off
```

For more details, check [vhost-user dpdk guide](http://dpdk.org/browse/dpdk/plain/doc/guides/prog_guide/vhost_lib.rst).

## Do You Support Any Containers?

Not yet, Butterfly only supports vhost-user network interfaces.

Anyways, connecting a container should be possible too.

## Does Butterfly Support IPv6?

- Virtual Machine traffic can be in IPv6.
- Outer network (VXLAN's side) is made in IPv4 for the moment.

## What If My Virtual Machine Crashes/Reboots/Stops?

Vhost-user interface will still exist until it is removed from Butterfly.

You can just restart your VM, it will be reconnected to Butterfly and run as
before.

This is possible because Butterfly acts as "server" in vhost-user communication.

## What If Butterfly Crashes?

Too bad, you can just restart Butterfly but the VM won't reconnect, as vhost-user
"server" is located on Butterfly's side.

Filing an issue is very valuable to the project. Please provide the following information:
- The operating system with its version
- The Butterfly version (butterfly --version)
- The Butterfly logs (check syslogs)
- Is your system under memory pressure?
- What was Butterfly doing? (Heavy traffic? Doing nothing? How many VMs?)
- The estimated Butterfly uptime until crash
- Do you have a way to reproduce it?

It may be soon possible to choose which one is the vhost-user server between QEMU and
Butterfly, [comming soon in DPDK](http://dpdk.org/ml/archives/dev/2016-May/038627.html) :)

## What Is Butterfly’s License?

Butterfly is licensed under [GPLv3](http://gplv3.fsf.org/).

## Is There Any Authentication on the API or Protection?

By default, there is no protection on the API but you can configure Butterfly
in order to have all it's messages encrypted using AES-256.

For this, you will need to generate a 32 Bytes key (encoded in base 64) in a file and share this file:
```
dd if=/dev/urandom bs=32 count=1 | base64 > api-key
```
Then you will need to provide the path to this file using `--key --k` in the command line or `encryption_key_path` option in `butterflyd.ini`.
Once a key is correctly loaded, all clear messages will be rejected.

For encryption format details, check [api/protocol/encrypted.proto](https://github.com/outscale/butterfly/blob/develop/api/protocol/encrypted.proto).

## On Which Port Does Butterfly Listen?

By default, Butterfly listens on the `tcp://0.0.0.0:9999` port, but it's up to you!

Butterfly uses ZeroMQ for message transport and allows you to bind in
[different ways](http://api.zeromq.org/4-0:zmq-bind). (like tcp, ipc, inproc, pgm, ...)

## Questions? Problems? Contact Us!

Butterfly is an open-source project, feel free to [chat with us on IRC]
(https://webchat.freenode.net/?channels=betterfly&nick=butterfly_user), open
a Github issue or propose a pull request.

> server: irc.freenode.org

> chan: #betterfly
