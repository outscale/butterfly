DESCRIPTION

## Packets come from VM (VHOST)

### Poll:

- We collect VM packets in the queue. If not packet in the queue it return.
- We count and collect all packets from the queue and build they metadata(packets size; layers size and type)
- We add packets size to the brick packets counter(so yes we can retrieve information about how much packets have been burst/receive).
- We burst packets.

## Packets go to VM (VHOST)

### Burst:
- Vhost bricks got all packet
- Copy packets in vhosts queue.
- We count all packets bytes that have been busted and we add it to packet count.

## PRINT (SNIFFER)

This brick is optional
- Sniffer write all networks traffic in pcap files.
- At this moment we only use pcap files but it possible to display it with FPRINTF().
- We can change the path of pcap files.
- For each packet:
- We collect data size
- Compute date and time of reception.
- If packet is segmented we store each segment in a buffer before writing them into the pcap file. otherwise write it directly.

## ANTISPOOF

### Burst:

Antispoof Brick have 2 parameter: a Mac Address and a boolean value
- We let pass incoming packets into VM.
- If packets are coming from VM:
- We block all reverse ARP packets.
- We verify that packet mac address source with Brick mac address are the same.
- If all verifiction are OK (is ARP reverse packet, is IPv4 or IPv6), we burst packet.

## NPF (FIREWALL)

Firewall use BPF rules, that can be pass to packetgraph API.
- We set firewalling rules by converting SG to BPF rule, see (link to better explications about that)
- Each rules have a direction. (IN/OUT or Both)
- Firewall analyse only IPv4, IPv6 protocols and let pass no-IP protocols.
- For each IP packets firewall brick check that packets pass the firewall (configured by butterfly earlier)

## SWITCH

- Switches are automatically added in network graphs when two or more VM connect to VTEP bricks with the same VNI on the same host.
- If mac tables are death, we try to rebuild them. If it fail, we return an error.
- We collect packet source address and port, (build metadata for each packet).
- For each non-multicast mac address, we associate mac address to corresponding source port, store it in mac table and burst packets to corresponding ports.
- For multicast mac and unknow one, we burst to all but incoming brick

## To VTEP

If mac tables are death, we try to rebuild them. If it fail, we return an error.
- We verify if destination IP address is multicast or unicast.
- If unicast we find corresponding port in mac table.
- Else we collect multicast mac address and create a new multicast IP address.
- We collect VNI corresponding to communication port, build and arrange needful headers (ip, udp, vxlan ...) VxLAN packets encapsulations.
- Burst them all.

## From VTEP

If mac tables are death, we try to rebuild them. If it fail, we return an error.

for each packet:
- We compare Vxlan packet VNI with port VNI, if it is OK, we remove packet VxLAN layer, record its mac address to VTEP mac table and get destination ip address.
- We restore network layers L2,L3 and L3, and we send packet to corresponding port VNI.

## SNIFFER (cf PRINT)

## NIC

### Poll

- We collect packets in nic, store them in an array.
- We rebuild all metadata for each packet
- Burst all packets in opposite direction

### Burst

- We collect packets, its numbers, put them in ethernet device.
- We burst packets on ethernet port and free packets.

