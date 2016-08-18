Butterfly
=========
Butterfly connects Virtual Machines and control their traffic flow.

Each VM traffic is contained in a specific [VXLAN](https://en.wikipedia.org/wiki/Virtual_Extensible_LAN)
network and traffic is filtered by Security Groups.

Security Groups can be applied to any VM interface, SG contain a list of simple
network rules (dropping traffic by default).

# Using Butterfly

Butterfly is a daemon you can control over a network API.

It's packaged with a client mainly allowing you to add/remove/list network
interfaces and security groups.

You can of course directly code your calls to Butterfly's API.
API message transport is based on [ZeroMQ](http://zeromq.org/) and message
are encoded in [Protobuf](https://github.com/google/protobuf/ "Google's protobuf")
format. Check out [protocol](https://github.com/outscale/butterfly/tree/master/api/protocol)
for more details.

Here is an example of Butterfly with 6 virtual machines isolated in 3 networks (VNI 42, 51 and 1337).

![Butterfly execution](https://osu.eu-west-2.outscale.com/jerome.jutteau/16d1bc0517de5c95aa076a0584b43af6/butterfly.svg)

# Installing Butterfly

The easiest way to install Butterfly would be to check github releases (coming
soon).

For now, check out how to build butterfly in the next section. Once build, you
can run `make install`.

# Build Butterfly

This build procedure has been tested on a fresh Centos7.

First, install some dependencies (jemalloc need manual installation):
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

![Let's compile, xkcd.com](http://imgs.xkcd.com/comics/compiling.png) (from [xkcd.com](http://xkcd.com/))

# Prepare your machine

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

If you have a DPDK compatible NIC, you can get [some](http://dpdk.org/doc/nics) [help](https://github.com/outscale/packetgraph/tree/master/examples/firewall#configure-your-nics) to configure it.

# Run Butterfly server

Butterfly command line is divided in two parts separated by `--`:
- DPDK arguments
- Butterfly arguments

First, to get help: `butterfly-server --help`

For example, if you have a DPDK compatible NIC, Butterfly will use the first
available DPDK port:
```
sudo butterfly-server -c1 -n1 --socket-mem 64 -- -i 192.168.0.1 -s /tmp
```

If you don't have a DPDK compatible card, you can init a DPDK virtual device
(which is _much_ slower than a DPDK compatible hardware).

For example, we can ask butterfly to listen to existing `eth0` interface:
```
sudo butterfly-server -c1 -n1 --socket-mem 64 --vdev=eth_pcap0,iface=eth0 -- -i 192.168.0.1 -s /tmp
```

Alternatively, you can ask Butterfly to read a [configuration file]
(https://github.com/outscale-jju/butterfly/blob/doc/api/server/original_config.ini)
at init:
```
sudo butterfly-server -c1 -n1 --socket-mem 64 -- -c /etc/butterfly/butterfly.conf
```

# F.A.Q.

## Why another virtual switch ?

Because we just want a fast vswitch corresponding to our simple needs:
- Have some VXLAN for network isolation.
- Have some firewalling per Virtual Machine based on Security Groups.
- Use as little CPU as possible (and let Virtual Machines use all other cores).
- Ease a (cloud) orchestrator to control the whole thing through a simple API.

## What's behind Butterfly ?

Butterfly is based on:
- [Packetgraph](http://github.com/outscale/packetgraph): creates network graph.
- [DPDK](http://dpdk.org/): fast access to NICs (in Packetgraph).
- [NPF](http://www.netbsd.org/~rmind/npf/): firewalling (in Packetgraph).
- [ZeroMQ](http://zeromq.org/): message transport.
- [Protobuf](https://github.com/google/protobuf/ "Google's protobuf"): message encoding/versioning.

## How fast ?

Benchmarks setup:
- Two physical machines directly connected.
- A third machine remotely setup and launch benchmarks using `./benchmarks/run_benchmarks.sh`.

Machines:
- OS: Centos 7 (3.10.0-327.18.2.el7.x86_64)
- NICs: Intel 82599ES 10-Gigabit SFI/SFP+ (DPDK compatible used with vfio-pci driver).
- CPU: AMD Opteron(tm) Processor 3350 HE
- All details for [host 1](https://osu.eu-west-2.outscale.com/jerome.jutteau/16d1bc0517de5c95aa076a0584b43af6/butterfly_1.txt) and [host 2](https://osu.eu-west-2.outscale.com/jerome.jutteau/16d1bc0517de5c95aa076a0584b43af6/butterfly_2.txt) (same)

Results (aug 8 2016):
```
                     |  VMs on same host  | VMs on remote host |
 --------------------+--------------------+--------------------|
| Ping (min/average) |  0.052ms / 0.118ms |  0.62ms / 0.172ms  |
| TCP                |     5.3 Gbits/s    |     4.2 Gbits/s    |
| UDP                |     1.4 Gbits/s    |     1.7 Gbits/s    |
```

## How to connect a Virtual Machine to Butterfly ?

Butterfly does not launch you virtual machine for you, it just create a special
network interface (vhost-user) so you can connect your Virtual Machine on it.
Vhost-user interfaces are unix sockets allowing you to directly communicate with
virtual machines in userland.

We tested Butterfly with Qemu >= 2.5 and add the following parameters to
machine's arguments (to adapt):

Some shared memory::
```
-object memory-backend-file,id=mem,size=124M,mem-path=/mnt/huge,share=on -numa node,memdev=mem -mem-prealloc
```

For each network interface:
```
-chardev socket,id=char0,path=/path/to/socket -netdev type=vhost-user,id=mynet0,chardev=char0,vhostforce -device virtio-net-pci,netdev=mynet0,gso=off
```

## Do you support any containers ?

Not yet, Butterfly only support vhost-user network interfaces.

Anyway, connect a container should be possible too.

## Does Butterfly support IPv6 ?

- Virtual Machine traffic can be in IPv6
- Outer nework (VXLAN's side) is made in IPv4 for the moment

## What if my virtual machine crash/reboot/stop ?

Vhost-user interface will still exist until it is removed in Butterfly.

You can just restart your VM, it will be reconnected to butterfly and run as
before.

This is possible because Butterfly act as "server" in vhost-user communication.

## What if Butterfly crash ?

Too bad, you can just restart butterfly but VM won't reconnect as vhost-user
"server" is located on butterfly's side.

Filling an issue is very valuable for project, please provide:
- Operating system with version
- Butterfly version (butterfly --version)
- Butterfly logs (check syslogs)
- Is your system under memory pressure ?
- What was doing Butterfly ? (Heavy traffic ? Doing nothing ? How many VM ?)
- Estimated Butterfly uptime until crash
- Do you have a way to reproduce it ?

It may be soon possible to choose which is vhost-user server between Qemu and
Butterfly, [comming soon in DPDK](http://dpdk.org/ml/archives/dev/2016-May/038627.html) :)

## What is Butterfly license ?

Butterfly is licensed under [GPLv3](http://gplv3.fsf.org/)

## Is there any authentification on API or protection ?

Network API is currently not protected at all.

For now, it's up to you to secure your administration network where butterfly
API listen.

## On which port does Butterfly listen ?

By default Butterfly listen on `tcp://0.0.0.0:9999` but it's up to you !

Butterfly uses ZeroMQ for message transport and allows you to bind in
[different ways](http://api.zeromq.org/4-0:zmq-bind) (like tcp, ipc, inproc, pgm, ...)

## Question ? Troubles ? Contact us !

Butterfly is an open-source project, feel free to [chat with us on IRC]
(https://webchat.freenode.net/?channels=betterfly&nick=butterfly_user), open
a Github issue or propose a pull request.

> server: irc.freenode.org

> chan: #betterfly

