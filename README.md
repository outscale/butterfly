WIP ALERT : This reprository is under active developpement and is subject to
heavy modification.

Butterfly
=========

Butterfly permits to connect Virtual Machine (VM) NICs and manage their
network flow.

Butterfly permits to isolate, connect and filter traffic between virtual
machines using VXLAN. This is particulary usefull in cloud environments in
order to manage network interactions between Virtual Machines or external
network.

# Architecture

Butterfly is based on:
- [DPDK](http://dpdk.org/)
- [Packetgraph](http://github.com/outscale/packetgraph)
- [ZeroMQ](http://zeromq.org/)
- [Protobuf](https://github.com/google/protobuf/ "Google's protobuf")

Butterfly is built on top of packetgraph and build a network graph for
it's own goal: connect virtual machines on different hosts on layer 2
with the best performances.

Here is the butterfly architecture 

![Butterfly architecture](http://i.imgur.com/Mnxid6n.png)

The software architecture is organized around the following
connected bricks (from Packetgraph):

1. Butterfly accelerate traffic latency and minimize Operating System
impact in packet filtering. This corresponds to "nic" brick in packetgraph.

2. Butterfly use [Virtual Extensible LAN (VXLAN)]
(http://en.wikipedia.org/wiki/Virtual_Extensible_LAN/)
permetting to isolate traffic between virtual machines over an external
network. Each links from a specific side corresponds to a VXLAN, the other
side corresponds to VTEP endpoint ("vtep" brick in packetgraph).

3. Butterfly use the layer 2 switch from packetgraph

4. Firewalling allow to filter network traffic from each virtual machines.
It is based on [NPF](http://www.netbsd.org/~rmind/npf/). This corresponds to
the "firewall" brick in packetgraph.

5. Butterfly can manage several Virtual Machines on the same host by using
vhost-user with Qemu. This corresponds to "vhost" brick in packetgraph.

# Butterfly API

Butterfly offers a [protobuf](https://github.com/google/protobuf/ "Google's protobuf")
defined API to configure all it's parameters.

# What do I need to use Butterfly ?

- A DPDK compatible NIC
- A x86_64 host
- At least Qemu 2.1

# Question ? Contact us !

Butterfly is an open-source project, feel free to contact us on IRC:

> server: irc.freenode.org

> chan: #betterfly

