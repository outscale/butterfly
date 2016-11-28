Butterfly with a pr tests for jenkins
=========
Butterfly connects Virtual Machines and controls their traffic flow.

Each VM traffic is contained in a specific [VXLAN](https://en.wikipedia.org/wiki/Virtual_Extensible_LAN)
network and traffic is filtered by (EC2/Openstack-like) security groups.

Security groups can be applied to any VM interface, and contain a list of simple
network rules (dropping traffic by default).

# Using Butterfly

Butterfly is a daemon you can control over a network API.

It is packaged with a client mainly allowing you to add/remove/list virtual
network interfaces and security groups.

You can of course directly code your calls to Butterfly's API.
API message transport is based on [ZeroMQ](http://zeromq.org/) and messages
are encoded in [Protobuf](https://github.com/google/protobuf/ "Google's protobuf")
format. Check out [protocol](https://github.com/outscale/butterfly/tree/master/api/protocol)
for more details.

Here is an example of Butterfly with 6 virtual machines isolated in 3 networks (VNI 42, 51 and 1337).

![Butterfly execution](https://osu.eu-west-2.outscale.com/jerome.jutteau/16d1bc0517de5c95aa076a0584b43af6/butterfly.svg)

Butterfly binds a dedicated NIC to send/receive VXLAN packets and binds a socket
(default: tcp) to listen to queries on its API. If you use a DPDK compatible
card, you won't be able to access the API through it.

# Installing Butterfly

The easiest way to install Butterfly is to download and install a package from [github releases](https://github.com/outscale/butterfly/releases).
You can also build Butterfly yourself (as shown in the next section) and run `make install` .

# Building Butterfly

This building procedure has been tested on a fresh Centos7.

First, install some dependencies (jemalloc needs manual installation):
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

# Preparing your machine

## Configure huge pages

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
- You may want to add this mount in your `/etc/fstab`:
```
hugetlbfs       /mnt/huge  hugetlbfs       rw,mode=0777        0 0
```

## Prepare DPDK compatible NIC

Before being able to bind your port, you will need to enable Intel VT-d in your BIOS and have IOMMU explicitly enabled in your kernel parameters.
Check [DPDK compatible NICs](http://dpdk.org/doc/nics) and how to [bind NIC drivers](http://people.redhat.com/~pmatilai/dpdk-guide/setup/binding.html).
Packetgraph also has an [example](https://github.com/outscale/packetgraph/tree/master/examples/firewall#configure-your-nics) on how to bind DPDK NICs.

Additionally, you may also want to isolate a specific core for Butterfly, check isolcpus [kernel parameters](https://www.kernel.org/doc/Documentation/kernel-parameters.txt)

# Running Butterfly server

To get help, see: `butterflyd --help`

For example, if you have a DPDK compatible NIC, Butterfly will use the first
available DPDK port. If no port is found, a (slow) tap interface is created.
```
$ sudo butterflyd -i 192.168.0.1 -s /tmp
```

If you don't have a DPDK compatible card, you can also init a DPDK virtual
device (which is _much_ slower than a DPDK compatible hardware).

For example, we can ask Butterfly to listen to existing `eth0` interface:
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

## Why another virtual switch?

Because we just want a fast vswitch corresponding to our simple needs:
- _simple_ API: EC2/Openstack security groups style.
- Have some VXLAN for network isolation.
- Have some firewalling per Virtual Machine based on security groups.
- Use as little CPU as possible (and let Virtual Machines use all other cores).
- Ease a (Cloud) orchestrator to control the whole thing through a simple API.

## What's behind Butterfly?

Butterfly is based on:
- [Packetgraph](http://github.com/outscale/packetgraph): creates network graph.
- [DPDK](http://dpdk.org/): fast access to NICs (in Packetgraph).
- [NPF](http://www.netbsd.org/~rmind/npf/): firewalling (in Packetgraph).
- [ZeroMQ](http://zeromq.org/): message transport.
- [Protobuf](https://github.com/google/protobuf/ "Google's protobuf"): message encoding/versioning.

## How fast?

Benchmarks setup:
- Two physical machines directly connected.
- A third machine remotely setup and launch benchmarks using `./benchmarks/run_benchmarks.sh`.

Machines:
- OS: Centos 7 (3.10.0-327.18.2.el7.x86_64)
- NICs: Intel 82599ES 10-Gigabit SFI/SFP+ (DPDK compatible used with vfio-pci driver).
- CPU: AMD Opteron(tm) Processor 3350 HE
- All details for [host 1](https://osu.eu-west-2.outscale.com/jerome.jutteau/16d1bc0517de5c95aa076a0584b43af6/butterfly_1.txt) and [host 2](https://osu.eu-west-2.outscale.com/jerome.jutteau/16d1bc0517de5c95aa076a0584b43af6/butterfly_2.txt) (same)

Results (nov 10 2016):
```
                     |  VMs on same host  | VMs on remote host |
 --------------------+--------------------+--------------------|
| Ping (min/average) |  0.067ms / 0.110ms |  0.157ms / 0.169ms |
| TCP                |    15.9 Gbits/s    |     3.0 Gbits/s    |
| UDP                |     1.4 Gbits/s    |     1.7 Gbits/s    |
```

Notes:
- We are working on accelerating inter-host performances.
- UDP is really _bad_ at the moment, working on it.
- We can get even faster with zero copy in vhost-user (in future DPDK versions).
- We can get faster by embedding a more recent libc (`make package-fat`).
- If you try to run some benchmarks, you may want to configure your [CPU throttling](https://en.wikipedia.org/wiki/Dynamic_frequency_scaling). On Centos7, check [cpufreq governors page](https://access.redhat.com/documentation/en-US/Red_Hat_Enterprise_Linux/7/html/Power_Management_Guide/cpufreq_governors.html).

Well, we have a good margin for improvements :)

## How to connect a Virtual Machine to Butterfly?

Butterfly does not launch your virtual machine for you, it just creates a special
network interface (vhost-user) so you can connect your Virtual Machine on it.
Vhost-user interfaces are Unix sockets allowing you to directly communicate with
virtual machines in userland.

We tested Butterfly with Qemu >= 2.5 and added the following parameters to
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

## Do you support any containers?

Not yet, Butterfly only supports vhost-user network interfaces.

Anyways, connecting a container should be possible too.

## Does Butterfly support IPv6?

- Virtual Machine traffic can be in IPv6.
- Outer network (VXLAN's side) is made in IPv4 for the moment.

## What if my virtual machine crashes/reboots/stops?

Vhost-user interface will still exist until it is removed from Butterfly.

You can just restart your VM, it will be reconnected to Butterfly and run as
before.

This is possible because Butterfly acts as "server" in vhost-user communication.

## What if Butterfly crashes?

Too bad, you can just restart Butterfly but the VM won't reconnect, as vhost-user
"server" is located on Butterfly's side.

Filling an issue is very valuable to the project. Please provide the following information:
- The operating system with its version
- The Butterfly version (butterfly --version)
- The Butterfly logs (check syslogs)
- Is your system under memory pressure?
- What was Butterfly doing? (Heavy traffic? Doing nothing? How many VMs?)
- The estimated Butterfly uptime until crash.
- Do you have a way to reproduce it?

It may be soon possible to choose which one is the vhost-user server between Qemu and
Butterfly, [comming soon in DPDK](http://dpdk.org/ml/archives/dev/2016-May/038627.html) :)

## What is Butterfly’s license?

Butterfly is licensed under [GPLv3](http://gplv3.fsf.org/).

## Is there any authentification on the API or protection?

The network API is currently not protected at all.

For now, it's up to you to secure your administration network where the Butterfly
API listens.

## On which port does Butterfly listen?

By default, Butterfly listens on the `tcp://0.0.0.0:9999` port, but it's up to you!

Butterfly uses ZeroMQ for message transport and allows you to bind in
[different ways](http://api.zeromq.org/4-0:zmq-bind). (like tcp, ipc, inproc, pgm, ...)

## Questions? Problems? Contact us!

Butterfly is an open-source project, feel free to [chat with us on IRC]
(https://webchat.freenode.net/?channels=betterfly&nick=butterfly_user), open
a Github issue or propose a pull request.

> server: irc.freenode.org

> chan: #betterfly
