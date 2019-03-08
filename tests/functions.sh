IMG_URL=https://osu.eu-west-2.outscale.com/jerome.jutteau/16d1bc0517de5c95aa076a0584b43af6/arch-260417.qcow
IMG_MD5=7bd77480631145b679bc8fac3583b163
KEY_URL=https://osu.eu-west-2.outscale.com/jerome.jutteau/16d1bc0517de5c95aa076a0584b43af6/arch-260417.rsa
KEY_MD5=eb3d700f2ee166e0dbe00f4e0aa2cef9

function usage {
    echo "Usage: test.sh BUTTERFLY_BUILD_ROOT" 1>&2
    echo "Usage: test.sh option" 1>&2
    echo "option:" 1>&2
    echo "   -h, --help : print this help" 1>&2
}


declare -A qemu_pids
declare -A server_pids
declare -A socat_pids

RETURN_CODE=0

function return_result {
    clean_all
    exit $RETURN_CODE
}

function ssh_run {
    id=$1
    cmd="${@:2}"
    key=$BUTTERFLY_BUILD_ROOT/vm.rsa
    ssh -q -p 500$id -l root -i $key -oStrictHostKeyChecking=no 127.0.0.1 $cmd
}

function ssh_run_background {
    id=$1
    cmd="${@:2}"
    key=$BUTTERFLY_BUILD_ROOT/vm.rsa
    ssh -q -f -p 500$id -l root -i $key -oStrictHostKeyChecking=no 127.0.0.1 $cmd &> /dev/null
}

function ssh_run_timeout {
    id=$1
    timeout=$2
    cmd="${@:3}"
    key=$BUTTERFLY_BUILD_ROOT/vm.rsa
    ssh -q -p 500$id -l root -i $key -oStrictHostKeyChecking=no -oConnectTimeout=$timeout 127.0.0.1 $cmd
}

function scp_to {
    id=$1
    local_file=$2
    distant_file=$3
    key=$BUTTERFLY_BUILD_ROOT/vm.rsa
    scp -P 500$id -i $key $local_file root@127.0.0.1:$distant_file &> /dev/null
}

function ssh_bash {
    id=$1
    cmd="${@:2}"
    key=$BUTTERFLY_BUILD_ROOT/vm.rsa
    ssh -p 500$id -l root -i $key -oStrictHostKeyChecking=no 127.0.0.1
}

function ssh_ping_ip {
    id=$1
    src=$2
    dst=$3

    ssh_run $id ping -c 1 -W 2 -I $src $dst &> /dev/null
    if [ $? -ne 0 ]; then
        fail "ping on VM $id: $src ---> $dst FAIL"
    else
        echo "ping on VM $id: $src ---> $dst OK"
    fi
}

function ssh_no_ping_ip {
    id=$1
    src=$2
    dst=$3

    ssh_run $id ping -c 1 -W 2 -I $src $dst &> /dev/null
    if [ $? -ne 0 ]; then
        echo "ping on VM $id: $src -/-> $dst OK"
    else
        fail "ping on VM $id: $src -/-> $dst FAIL"
    fi
}

function ssh_ping_ip6 {
    id=$1
    src=$2
    dst=$3

    ssh_run $id ping -6 -c 1 -W 2 -I $src $dst &> /dev/null
    if [ $? -ne 0 ]; then
        fail "ping6 on VM $id: $src ---> $dst FAIL"
    else
        echo "ping6 on VM $id: $src ---> $dst OK"
    fi
}

function ssh_no_ping_ip6 {
    id=$1
    src=$2
    dst=$3

    ssh_run $id ping -6 -c 1 -W 2 -I $src $dst &> /dev/null
    if [ $? -ne 0 ]; then
        echo "ping on VM $id: $src -/-> $dst OK"
    else
        fail "ping6 on VM $id: $src -/-> $dst FAIL"
    fi
}

function ssh_ping {
    id1=$1
    id2=$2

    ssh_run $id1 ping 42.0.0.$id2 -c 1 -W 2 &> /dev/null
    if [ $? -ne 0 ]; then
        fail "ping VM $id1 ---> VM $id2 FAIL"
    else
        echo "ping VM $id1 ---> VM $id2 OK"
    fi
}

function ssh_no_ping {
    id1=$1
    id2=$2

    ssh_run $id1 ping 42.0.0.$id2 -c 1 -W 2 &> /dev/null
    if [ $? -ne 0 ]; then
        echo "ping VM $id1 -/-> VM $id2 OK"
    else
        fail "ping VM $id1 -/-> VM $id2 FAIL"
    fi
}

function ssh_ping6 {
    id1=$1
    id2=$2
    ssh_run $id1 ping -6 2001:db8:2000:aff0::$id2 -c 1 -W 2 &> /dev/null
    if [ $? -ne 0 ]; then
        fail "ping6 VM $id1 ---> VM $id2 FAIL"
    else
        echo "ping6 VM $id1 ---> VM $id2 OK"
    fi
}

function ssh_no_ping6 {
    id1=$1
    id2=$2

    ssh_run $id1 ping -6 2001:db8:2000:aff0::$id2 -c 1 -W 2 &> /dev/null
    if [ $? -ne 0 ]; then
        echo "ping6 VM $id1 -/-> VM $id2 OK"
    else
        fail "ping6 VM $id1 -/-> VM $id2 FAIL"
    fi
}

function tap_ping {
    id1=$1
    id2=$2
    sudo ip netns exec ns$id1 ping 42.0.0.$id2 -c 5
    ret=$?
    echo ret $ret
    if [ $ret -eq 0 ]; then
        echo "ping TAP ns$id1 -/-> $id2 OK"
    else
        fail "ping TAP ns$id1 -/-> $id2 FAIL"
    fi
}

function tap_iperf3_tcp {
    id1=$1
    id2=$2

    sudo ip netns exec ns$id1 iperf3 -s &
    local server_pid=$!
    sleep 1
    sudo ip netns exec ns$id2 iperf3 -c 42.0.0.$id1 -t 5
    if [ $? -ne 0 ]; then
        fail "iperf3 tcp ns $id1 ---> $id2 FAIL"
    else
        echo "iperf3 tcp ns $id1 ---> $id2 OK"
    fi
    kill -15 $server_pid &> /dev/null
}

function ssh_iperf_tcp {
    id1=$1
    id2=$2

    (ssh_run $id1 iperf -s &> /dev/null &)
    local server_pid=$!
    sleep 1
    ssh_run $id2 iperf -c 42.0.0.$id1 -t 3 &> /dev/null
    if [ $? -ne 0 ]; then
        fail "iperf tcp VM $id1 ---> VM $id2 FAIL"
    else
        echo "iperf tcp VM $id1 ---> VM $id2 OK"
    fi
    kill -15 $server_pid &> /dev/null
}

function ssh_iperf_udp {
    id1=$1
    id2=$2

    (ssh_run $id1 iperf -s -u &> /tmp/iperf_tmp_results &)
    local server_pid=$!
    sleep 1
    ssh_run $id2 iperf -l 1400 -c 42.0.0.$id1 -t 3 -u &> /dev/null
    ret=$?
    res=$(cat /tmp/iperf_tmp_results |grep "%" | cut -d '(' -f 2 | cut -d '%' -f 1)
    if [ $ret -ne 0 ] || [ ".$res" != ".0" ]; then
        fail "iperf udp VM $id1 ---> VM $id2 FAIL"
    else
        echo "iperf udp VM $id1 ---> VM $id2 OK"
    fi
    kill -15 server_pid &> /dev/null
    rm /tmp/iperf_tmp_results
}

function ssh_iperf3_tcp {
    id1=$1
    id2=$2

    (ssh_run $id1 iperf3 -s &> /dev/null &)
    local server_pid=$!
    sleep 1
    ssh_run $id2 iperf3 -c 42.0.0.$id1 -t 10 #&> /dev/null
    if [ $? -ne 0 ]; then
        fail "iperf3 tcp VM $id1 ---> VM $id2 FAIL"
    else
        echo "iperf3 tcp VM $id1 ---> VM $id2 OK"
    fi
    kill -15 $server_pid &> /dev/null
}

function ssh_iperf3_udp {
    id1=$1
    id2=$2

    (ssh_run $id1 iperf3 -s &> /dev/null &)
    local server_pid=$!
    sleep 1
    ssh_run $id2 iperf3 -l 1400 -c 42.0.0.$id1 -t 3 -u --reverse --json > /tmp/iperf3_tmp_results
    local ret=$?
    local res=$(cat /tmp/iperf3_tmp_results | grep packets | tail -n 1 | cut -d ':' -f 2 | cut -d ',' -f 1 | tr -d ' ' | tr -d '\t')
    if [ "$res" == "0" ] || [ "$ret" != "0" ]; then
        fail "iperf3 udp VM $id1 ---> VM $id2 FAIL"
    else
        echo "iperf3 udp VM $id1 ---> VM $id2 OK"
    fi
    kill -15 $server_pid &> /dev/null
    rm /tmp/iperf3_tmp_results
}

function ssh_connection_tests_send {
    protocol=$1
    if [ "$protocol" == "udp" ]; then
        proto_cmd="-4 -u"
    elif [ "$protocol" == "tcp" ]; then
        proto_cmd="-4"
    elif [ "$protocol" == "udp6" ]; then
        proto_cmd="-6 -u"
    elif [ "$protocol" == "tcp6" ]; then
        proto_cmd="-6"
    elif [ "$protocol" == "sctp" ]; then
        proto_cmd="sctp"
    else
        fail "protocol $protocol not supported by nic_add_port_open"
    fi

    if [ "$1" == "udp6" ] || [ "$1" == "tcp6" ]; then
        ssh_run_background $2 "echo 'this message is from vm $2' | ncat $proto_cmd 2001:db8:2000:aff0::$3 $4"
    else
        ssh_run_background $2 "echo 'this message is from vm $2' | ncat $proto_cmd 42.0.0.$3 $4"
    fi
}

function ssh_connection_tests_internal {
    protocol=$1
    id1=$2
    id2=$3
    port=$4
    proto_cmd=""
    is_udp=0

    if [ "$protocol" == "udp" ]; then
        proto_cmd="-4 -u"
        is_udp=1
    elif [ "$protocol" == "tcp" ]; then
        proto_cmd="-4"
    elif [ "$protocol" == "udp6" ]; then
        is_udp=1
        proto_cmd="-6 -u"
    elif [ "$protocol" == "tcp6" ]; then
        proto_cmd="-6"
    elif [ "$protocol" == "sctp" ]; then
        proto_cmd="sctp"
    else
        fail "protocol $protocol not supported by nic_add_port_open"
    fi

    if [ "$proto_cmd" == "sctp" ]; then
        ssh_run_background $id2 "sctp_test -H 0 -P $port -l > /tmp/test"
        sleep 0.4
        ssh_run_background $id1 "sctp_test -H 0 -P 8142 -h 42.0.0.$id2 -p $port -s"
    else
        ssh_run_background $id2 "ncat $proto_cmd -lp $port > /tmp/test"
        if [ $is_udp -eq 1 ]; then
            sleep 0.4
        else
            sleep 5
        fi
        ssh_connection_tests_send $protocol $id1 $id2 $port
    fi
    return 0
}

function ssh_clean_connection {
    id1=$1
    id2=$2

    ssh_run $id2 "rm /tmp/test" &> /dev/null
    ssh_run $id1 "killall ncat" &> /dev/null
    ssh_run $id2 "killall ncat" &> /dev/null
    ssh_run $id2 "killall sctp_test" &> /dev/null
}

function ssh_connection_test_file {
    id=$1
    protocol=$2
    timeout=20
    it=0

    if [ "$protocol" != "sctp" ]; then
        while [ $it -ne $timeout ]; do
            ssh_run $1 "cat /tmp/test"
            ssh_run $1 [ -s "/tmp/test" ]
            if [ "$?" == "0" ]; then
                return 0
            fi
            if [ "$protocol" == "udp" ] || [ "$protocol" == "udp6" ] ; then
                # as udp can fail, we retry every time
               ssh_connection_tests_send $protocol $3 $1 $4
            fi
            sleep 1
            it=$(( $it + 1 ))
        done
    else
        while [ $it -ne $timeout ]; do
            ret=$(ssh_run $id "cat /tmp/test | wc -l")
            if [ "$ret" -gt "20" ]; then
                return 0
            fi
            sleep 0.1
            it=$(( $it + 1 ))
        done
    fi
    return 1
}

function ssh_connection_test {
    protocol=$1
    id1=$2
    id2=$3
    port=$4

    ssh_connection_tests_internal $protocol $id1 $id2 $port
    if [ "$?" != "0" ]; then
        fail "$protocol test VM $id1 ---> VM $id2 FAIL (1)"
    fi

    ssh_connection_test_file $id2 $protocol $id1 $port
    if [ "$?" == "0" ]; then
        echo -e "$protocol test VM $id1 ---> VM $id2 OK"
    else
        fail "$protocol test VM $id1 ---> VM $id2 FAIL"
    fi

    ssh_clean_connection $id1 $id2
    return $RETURN_CODE
}

function ssh_no_connection_test {
    protocol=$1
    id1=$2
    id2=$3
    port=$4

    ssh_connection_tests_internal $protocol $id1 $id2 $port
    if [ "$?" != "0" ]; then
        fail "$protocol test VM $id1 -/-> VM $id2 FAIL (1)"
    fi
    if [ "$protocol" == "sctp" ]; then
        ssh_connection_test_file $id2 $protocol $id1 $port
        if [ "$?" == "0" ]; then
            fail "$protocol test VM $id1 -/-> VM $id2 FAIL"
        else
            echo -e "$protocol test VM $id1 -/-> VM $id2 OK"
        fi
    else
        ssh_run $id2 [ -s "/tmp/test" ]
        if [ "$?" == "0" ]; then
            fail "$protocol test VM $id1 -/-> VM $id2 FAIL"
        else
            echo -e "$protocol test VM $id1 -/-> VM $id2 OK"
        fi
    fi

    ssh_clean_connection $id1 $id2
    return $RETURN_CODE
}

function qemu_add_ipv4 {
    id=$1
    ips="${@:2}"
    for ip in $ips; do
        echo "[VM $id] add ipv4 $ip"
        ssh_run $id ip addr add $ip dev ens4
    done
}

function qemu_del_ipv4 {
    id=$1
    ips="${@:2}"
    for ip in $ips; do
        echo "[VM $id] remove ipv4 $ip"
        ssh_run $id ip addr del $ip dev ens4
    done
}

function qemu_add_ipv6 {
    id=$1
    ips="${@:2}"
    for ip in $ips; do
        echo "[VM $id] add ipv6 $ip"
        ssh_run $id ip -6 addr add $ip dev ens4
    done
}

function qemu_del_ipv6 {
    id=$1
    ips="${@:2}"
    for ip in $ips; do
        echo "[VM $id] remove ipv6 $ip"
        ssh_run $id ip -6 addr del $ip dev ens4
    done
}

function qemu_start_async {
    qemu_start $1 $2 &
}


function qemus_wait {
    if [ "$#" -eq 0 ]; then
        return 0
    fi
    qemu_wait $1
    shift 1
    qemus_wait $@
}

function qemus_start_ {
    if [ "$#" -eq 0 ]; then
        return 0
    fi
    qemu_start_async $1
    shift 1
    qemus_start_ $@
}

function qemus_start {
    qemus_start_ $@
    qemus_wait $@
}

function qemu_wait_pid {
    id=$1
    timeleft=$2
    while [ $timeleft -gt 0 ]; do
       if [ -f "$BUTTERFLY_BUILD_ROOT/qemu_pids$id" ]; then
        return 0
       fi
       sleep 1
       timeleft=$((timeleft - 1))
    done
    return 1
}

function qemu_wait {
    id=$1
    echo "hello" | nc -w 1  127.0.0.1 500$id &> /dev/null
    TEST=$?
    MAX_TEST=0
    while  [ $TEST -ne 0 -a $MAX_TEST -ne 20 ]
    do
        echo "hello" | nc -w 1  127.0.0.1 500$id &> /dev/null
        TEST=$?
        MAX_TEST=$(($MAX_TEST + 1))
        sleep 0.2
    done

    qemu_wait_pid $id 30
    if [ ! $? ]; then
        fail "[VM $id] qemu timeout"
        return
    fi
    qemu_pids["$id"]=$(< $BUTTERFLY_BUILD_ROOT/qemu_pids$id)
}

function qemu_start {
    id=$1
    ip=$2
    nic_type=$3
    echo "[VM $id] starting"
    SOCKET_PATH=/tmp/qemu-vhost-nic-$id
    IMG_PATH=$BUTTERFLY_BUILD_ROOT/vm.qcow
    MAC=52:54:00:12:34:0$id

    if [ "$nic_type" == "tap" ]; then
        sudo ip link delete but-br-$id type bridge
        sudo ip link delete tap-q-$id type bridge
        sudo ip tuntap add dev tap-q-$id mode tap
        sudo ip link add but-br-$id type bridge
        sudo ip link set tap-q-$id master but-br-$id
        sudo ip link set tap-$id master but-br-$id
        sudo ip link set tap-q-$id up
        sudo ip link set tap-$id up
        sudo ip link set but-br-$id up
        sudo iptables -A FORWARD -p all -i but-br-$id -j ACCEPT
        CMD="sudo qemu-system-x86_64 -netdev user,id=network0,hostfwd=tcp::500${id}-:22 -device e1000,netdev=network0 -m 124M -enable-kvm  -netdev type=tap,id=mynet1,ifname=tap-q-$id,script=no,downscript=no -device virtio-net-pci,gso=off,mac=$MAC,netdev=mynet1 -drive file=$IMG_PATH -snapshot -display none"
    else
        CMD="sudo qemu-system-x86_64 -netdev user,id=network0,hostfwd=tcp::500${id}-:22 -device e1000,netdev=network0 -m 124M -enable-kvm -chardev socket,id=char0,path=$SOCKET_PATH -netdev type=vhost-user,id=mynet1,chardev=char0,vhostforce -device virtio-net-pci,csum=off,gso=off,mac=$MAC,netdev=mynet1 -object memory-backend-file,id=mem,size=124M,mem-path=/mnt/huge,share=on -numa node,memdev=mem -mem-prealloc -drive file=$IMG_PATH -snapshot -display none"
    fi
    exec $CMD &> $BUTTERFLY_BUILD_ROOT/qemu_${id}_output &
    pid=$!

    echo "hello" | nc -w 1  127.0.0.1 500$id &> /dev/null
    TEST=$?
    MAX_TEST=0
    while  [ $TEST -ne 0 -a $MAX_TEST -ne 20 ]
    do
        echo "hello" | nc -w 1  127.0.0.1 500$id &> /dev/null
        TEST=$?
        MAX_TEST=$(($MAX_TEST + 1))
        sleep 0.2
    done

    sudo kill -s 0 $pid &> /dev/null
    if [ $? -ne 0 ]; then
        fail "failed to start qemu, check qemu_${id}_output file"
    fi
    qemu_pids["$id"]=$pid

    # Wait for ssh to be ready
    ssh_run_timeout $id 60 true

    # Configure IP on vhost interface
    ssh_run $id ip link set ens4 up
    if [ "$ip" == "dhcp-server" ]; then
        qemu_add_ipv4 $id 42.0.0.$id/24
        qemu_add_ipv6 $id 2001:db8:2000:aff0::$id/64
        echo -e "
            option domain-name-servers 8.8.8.8, 8.8.4.4;
            option subnet-mask 255.255.255.0;
            subnet 42.0.0.0 netmask 255.255.255.0 {
              range 42.0.0.20 42.0.0.50;
              host vm1{
                hardware ethernet 52:54:00:12:34:01;
                fixed-address 42.0.0.1;
              }
              host vm2{
                hardware ethernet 52:54:00:12:34:02;
                fixed-address 42.0.0.2;
              }
              host vm3{
                hardware ethernet 52:54:00:12:34:03;
                fixed-address 42.0.0.3;
              }
              host vm4{
                hardware ethernet 52:54:00:12:34:04;
                fixed-address 42.0.0.4;
              }
            }" > /tmp/tmp_dhcpd.conf
        scp_to $id /tmp/tmp_dhcpd.conf /etc/dhcpd.conf
        rm /tmp/tmp_dhcpd.conf
        ssh_run $id systemctl start dhcpd4 &> /dev/null
    elif [ "$ip" == "dhcp-client" ]; then
        ssh_run $id dhcpcd ens4 &> /dev/null || ( echo "DHCP failed !" && false )
    elif [ "$ip" == "noip" ]; then
        true
    else
        qemu_add_ipv4 $id 42.0.0.$id/24
        qemu_add_ipv6 $id 2001:db8:2000:aff0::$id/64
    fi

    echo $pid > $BUTTERFLY_BUILD_ROOT/qemu_pids$id
}

function qemu_stop {
    id=$1
    rm -f $BUTTERFLY_BUILD_ROOT/qemu_pids$id
    echo "[VM $id] stopping (pid ${qemu_pids[$id]})"
    sudo kill -15 $(ps --ppid ${qemu_pids[$id]} -o pid=) &> /dev/null
    sleep 0.3
}

function qemus_stop {
    if [ "$#" -eq 0 ]; then
        return 0
    fi
    qemu_stop $1
    shift 1
    qemus_stop $@
    sudo ip link delete but-br-$id type bridge
    sudo ip link delete tap-q-$id type bridge
}

function server_start {
    id=$1
    server_start_options $id -t
}

function server_start_huge {
    id=$1
    options=${@:2}
    echo "[butterfly-$id] starting"

    exec sudo $BUTTERFLY_BUILD_ROOT/api/server/butterflyd --dpdk-args "--no-shconf -c1 -n1 --vdev=eth_pcap$id,iface=but$id -m 1024  --file-prefix=host --no-pci" -l debug -i ::101 -s /tmp --endpoint=tcp://0.0.0.0:876$id $options &> $BUTTERFLY_BUILD_ROOT/butterflyd_${id}_output &
    pid=$!
    sleep 1
    sudo kill -s 0 $pid
    if [ $? -ne 0 ]; then
        fail "failed to start butterfly, check butterflyd_${id}_output file"
    fi

    server_pids["$id"]=$pid
}

function server_start_options {
    id=$1
    options=${@:2}
    echo "[butterfly-$id] starting"

    exec sudo $BUTTERFLY_BUILD_ROOT/api/server/butterflyd --dpdk-args "--no-shconf -c1 -n1 --vdev=eth_pcap$id,iface=but$id --no-huge" -l debug -i ::101 -s /tmp --endpoint=tcp://0.0.0.0:876$id $options &> $BUTTERFLY_BUILD_ROOT/butterflyd_${id}_output &
    pid=$!
    sleep 1
    sudo kill -s 0 $pid
    if [ $? -ne 0 ]; then
        fail "failed to start butterfly, check butterflyd_${id}_output file"
    fi

    server_pids["$id"]=$pid
}

function server_start_no_trace {
    id=$1
    server_start_options $id
}

function server_start_bonding {
    id=$1
    mode=$2
    echo "[butterfly-$id] starting"

    exec sudo $BUTTERFLY_BUILD_ROOT/api/server/butterflyd --dpdk-port=3 --dpdk-args "--no-shconf -c1 -n1 --vdev=eth_pcap$id,iface=but$id --vdev=eth_pcap1_$id,iface=but$id --vdev=eth_pcap2_$id,iface=but$id  --vdev net_bonding$id,mode=$mode,slave=0,slave=1,slave=2 --no-huge" -l debug -i ::101 -s /tmp --endpoint=tcp://0.0.0.0:876$id -t &> $BUTTERFLY_BUILD_ROOT/butterflyd_${id}_output &
    pid=$!
    sleep 1
    sudo kill -s 0 $pid
    if [ $? -ne 0 ]; then
        fail "failed to start butterfly, check butterflyd_${id}_output file"
    fi

    server_pids["$id"]=$pid
}

function server_stop {
    id=$1
    echo "[butterfly-$id] stopping"
    sudo kill -2 $(ps --ppid ${server_pids[$id]} -o pid=)
    sudo kill -s 0 ${server_pids[$id]} &> /dev/null ;
    if [ $? -ne 0 ]; then
        sudo kill -2 $(ps --ppid ${server_pids[$id]} -o pid=)
        sleep 0.1
        sudo kill -s 0 ${server_pids[$id]} &> /dev/null ;
    fi
    if [ $? -ne 0 ]; then
        sudo kill -2 $(ps --ppid ${server_pids[$id]} -o pid=)
        sleep 0.1
        sudo kill -s 0 ${server_pids[$id]} &> /dev/null ;
    fi
    if [ $? -ne 0 ]; then
        sudo kill -2 $(ps --ppid ${server_pids[$id]} -o pid=)
        sleep 0.1
        sudo kill -s 0 ${server_pids[$id]} &> /dev/null ;
    fi
    if [ $? -ne 0 ]; then
        sudo kill -2 $(ps --ppid ${server_pids[$id]} -o pid=)
        sleep 0.1
        sudo kill -s 0 ${server_pids[$id]} &> /dev/null ;
    fi
    if [ $? -ne 0 ]; then
        sudo kill -15 $(ps --ppid ${server_pids[$id]} -o pid=)
        sleep 0.4
        sudo kill -s 0 ${server_pids[$id]} &> /dev/null ;
    fi
    if [ $? -ne 0 ]; then
        sudo kill -9 $(ps --ppid ${server_pids[$id]} -o pid=)
        sleep 0.3
    fi
}

function server_start_ipv4 {
    id=$1
    server_start_options_ipv4 $id -t
}

function server_start_options_ipv4 {
    id=$1
    options=${@:2}
    echo "[butterfly-$id] starting"

    exec sudo $BUTTERFLY_BUILD_ROOT/api/server/butterflyd --dpdk-args "--no-shconf -c1 -n1 --vdev=eth_pcap$id,iface=but$id --no-huge" -l debug -i 10.0.0.$id -s /tmp --endpoint=tcp://0.0.0.0:876$id $options -t &> $BUTTERFLY_BUILD_ROOT/butterflyd_${id}_output &
    pid=$!
    sleep 1
    sudo kill -s 0 $pid
    if [ $? -ne 0 ]; then
        fail "failed to start butterfly, check butterflyd_${id}_output file"
    fi
    server_pids["$id"]=$pid
}

function network_connect {
    id1=$1
    id2=$2
    echo "network connect butterfly-$id1 <--> butterfly-$id2"
    sudo socat TUN:192.168.42.$id1/24,tun-type=tap,iff-running,iff-up,iff-promisc,tun-name=but$id1 TUN:192.168.42.$id2/24,tun-type=tap,iff-running,iff-promisc,iff-up,tun-name=but$id2 &
    pid=$!
    sleep 0.2
    sudo kill -s 0 $pid
    if [ $? -ne 0 ]; then
        fail "failed connect but-$id1 and but-$id2"
    fi
    socat_pids["$id1$id2"]=$pid
    sudo ip link set dev but$id1 mtu 2000
    sudo ip link set dev but$id2 mtu 2000
}

function network_disconnect {
    id1=$1
    id2=$2
    echo "network disconnect butterfly-$id1 <--> butterfly-$id2"
    sudo kill -15 $(ps --ppid ${socat_pids[$id1$id2]} -o pid=)
}

function download {
    url=$1
    md5=$2
    path=$3

    if [ ! -f $path ]; then
        echo "$path, let's download it..."
        wget -q $url -O $path || exit 1
    fi

    if [ ! "$(md5sum $path | cut -f 1 -d ' ')" == "$md5" ]; then
        echo "Bad md5 for $path, let's download it ..."
        wget -q $url -O $path || exit 1
        if [ ! "$(md5sum $path | cut -f 1 -d ' ')" == "$md5" ]; then
            echo "Still bad md5 for $path ... abort."
            exit 1
        fi
    fi
}

function request {
    but_id=$1
    f=$2

    $BUTTERFLY_BUILD_ROOT/api/client/butterfly request $f -e tcp://127.0.0.1:876$but_id
    ret=$?
    rm $f
    if [ ! "$ret" == "0" ]; then
        fail "client failed to send message to butterfly $but_id"
    fi
    sleep 0.5
}

function cli {
    but_id=$1
    excepted_result=$2
    opts=${@:3}
    echo "[butterfly-$but_id] cli run $opts"

    $BUTTERFLY_BUILD_ROOT/api/client/butterfly $opts -e tcp://127.0.0.1:876$but_id &> $BUTTERFLY_BUILD_ROOT/cli_output
    if [ ! "$?" == "$excepted_result" ]; then
        fail "cli run failed, check cli_output file"
    fi
    sleep 0.5
}

function nic_add_noip {
    but_id=$1
    nic_id=$2
    vni=$3
    sg_list=${@:4}
    cli $but_id 0 nic add --id nic-$nic_id --mac 52:54:00:12:34:0$nic_id --vni $vni --enable-antispoof + nic sg add nic-$nic_id $sg_list
}

function nic_update {
     but_id=$1
     nic_id=$2
     update_options=${@:3}

     echo "[butterfly-$but_id] update nic-$nic_id with: $update_options"
     cli $but_id 0 nic update --id nic-$nic_id $update_options

     if ! test -e /tmp/qemu-vhost-nic-$nic_id ; then
         echo "client failed: we should have a socket in /tmp/qemu-vhost-nic-$nic_id"
         clean_all
         exit 1
     fi
}

function tap_del {
    nic_id=$1

    sudo ip netns del ns$1
}

function tap_create {
    but_id=$1
    nic_id=$2
    vni=$3
    sg_list=${@:4}

    cli $but_id 0 nic add --id "tap-$nic_id" --mac "52:54:00:12:34:0$nic_id" --vni $vni --ip "42.0.0.$nic_id" --enable-antispoof --type TAP
    sleep 1
    for i in $sg_list; do
        cli $but_id 0 nic sg add "tap-$nic_id" $i
    done
    #sudo ip link set tap-$nic_id
}

function tap_add {
    but_id=$1
    nic_id=$2
    vni=$3
    sg_list=${@:4}

    echo "[butterfly-$but_id] add tap nic $nic_id with vni $vni"

    cli $but_id 0 nic add --id "tap-$nic_id" --mac "52:54:00:12:34:0$nic_id" --vni $vni --ip "42.0.0.$nic_id" --enable-antispoof --type TAP
    sleep 1

    sudo ip netns add ns$nic_id
    sudo ip link set tap-$nic_id up netns ns$nic_id address "52:54:00:12:34:0$nic_id"
    sudo ip netns exec ns$nic_id ip link set dev lo up
    sudo ip netns exec ns$nic_id ip addr add 42.0.0.$nic_id/24 dev tap-$nic_id
    for i in $sg_list; do
        cli $but_id 0 nic sg add "tap-$nic_id" $i
    done
    ip netns list
    sleep 1
}

function nic_add {
    but_id=$1
    nic_id=$2
    vni=$3
    sg_list=${@:4}

    echo "[butterfly-$but_id] add nic $nic_id with vni $vni"

    cli $but_id 0 nic add --id "nic-$nic_id" --mac "52:54:00:12:34:0$nic_id" --vni $vni --ip "42.0.0.$nic_id" --enable-antispoof
    sleep 1

    for i in $sg_list; do
        cli $but_id 0 nic sg add "nic-$nic_id" $i
    done
    sleep 1

    if ! test -e /tmp/qemu-vhost-nic-$nic_id ; then
        fail "client failed: we should have a socket in /tmp/qemu-vhost-nic-$nic_id"
    fi
}

function nic_add_trace {
    but_id=$1
    nic_id=$2
    vni=$3
    sg_list=${@:4}

    echo "[butterfly-$but_id] add nic $nic_id with vni $vni"

    cli $but_id 0 nic add --id "nic-$nic_id" --mac "52:54:00:12:34:0$nic_id" --vni $vni --ip "42.0.0.$nic_id" --enable-antispoof --packet-trace true
    sleep 1

    for i in $sg_list; do
        cli $but_id 0 nic sg add "nic-$nic_id" $i
    done
    sleep 1

    if ! test -e /tmp/qemu-vhost-nic-$nic_id ; then
        fail "client failed: we should have a socket in /tmp/qemu-vhost-nic-$nic_id"
    fi
}

function nic_add6 {
    but_id=$1
    nic_id=$2
    vni=$3
    sg_list=${@:4}

    echo "[butterfly-$but_id] add nic(6) $nic_id with vni $vni"

    cli $but_id 0 nic add --id "nic-$nic_id" --mac "52:54:00:12:34:0$nic_id" --vni $vni --ip "2001:db8:2000:aff0::$nic_id" --enable-antispoof

    for i in $sg_list; do
       cli $but_id 0 nic sg add "nic-$nic_id" $i
    done
    sleep 0.3

    if ! test -e /tmp/qemu-vhost-nic-$nic_id ; then
        fail "client failed: we should have a socket in /tmp/qemu-vhost-nic-$nic_id"
    fi
}

function nic_add6_trace {
    but_id=$1
    nic_id=$2
    vni=$3
    sg_list=${@:4}

    echo "[butterfly-$but_id] add nic(6) $nic_id with vni $vni"

    cli $but_id 0 nic add --id "nic-$nic_id" --mac "52:54:00:12:34:0$nic_id" --vni $vni --ip "2001:db8:2000:aff0::$nic_id" --enable-antispoof --packet-trace true
    sleep 1

    for i in $sg_list; do
       cli $but_id 0 nic sg add "nic-$nic_id" $i
    done
    sleep 0.3

    if ! test -e /tmp/qemu-vhost-nic-$nic_id ; then
        fail "client failed: we should have a socket in /tmp/qemu-vhost-nic-$nic_id"
    fi
}

function test_packet_tracing {
    check=$1
    id1=$2
    id2=$3
    id_list=${@:2}

    for i in $id_list; do
        if [ "$i" != "0" ] && [ -e /tmp/*nic-$i.pcap ]; then
            sleep 0.2
            paths+=" /tmp/*nic-$i.pcap"
        else
            fail "Tracing failed: can not found pcap file with id $i"
        fi
    done
    test_packet_trace_path $check $paths
    unset paths
}

function print_message {
    if [ "$1" == "1" ]; then
        echo "$2 OK"
    else
        fail "$2 FAIL"
    fi
}

function test_size {
    declare -A my_arry  
    path_lists=${@:1}
    ret=0;

    for path in $path_lists; do
        my_arry[$path]=$(du -b "$path" | awk '{ print $1 }')
        if [ ! -e $path-butterfly_size ]; then
            if [ ${my_arry[$path]} -gt "0" ]; then
                ret=1
            fi
            echo "${my_arry[$path]}" > $path-butterfly_size
        fi
    done

    if [ $ret -eq "1" ]; then
        unset path_lists
        unset my_arry[@]
        unset path
        return $ret
    fi

    for path in $path_lists; do
        if [ ${my_arry[$path]} -eq $(cat "$path-butterfly_size") ]; then
            echo "${my_arry[$path]}" > $path-butterfly_size
            ret=0
        elif [ ${my_arry[$path]} -gt $(cat "$path-butterfly_size") ]; then
            echo "${my_arry[$path]}" > $path-butterfly_size
            ret=1
        else
            ret=-1
        fi
    done
    unset path_lists
    unset my_arry[@]
    unset path
    return $ret
}

function test_packet_trace_path {
    check=$1
    path_list=${@:2}

    for path in $path_list; do
        if [ ! -f $path ]; then
            fail "Trace path FAIL: can not found $path"
            return 1
        fi
    done

    test_size $path_list
    result="$?"
    if [ "$check" == "false" ]; then
        print_message $(($result == 0)) "Disable tracing on: $path_list"
    elif [ "$check" == "true" ]; then
        print_message $(($result == 1)) "Enable tracing on: $path_list"
    else
        fail "Error $check have to be true or false"
    fi
    unset path_list
    unset check
}

function nic_add_bypass {
    but_id=$1
    nic_id=$2
    vni=$3

    f=/tmp/butterfly.req
    echo "[butterfly-$but_id] add nic $nic_id with vni $vni (filtering bypass)"

    cli $but_id 0 nic add --id "nic-$nic_id" --mac "52:54:00:12:34:0$nic_id" --vni $vni --ip "42.0.0.$nic_id" --bypass-filtering
    sleep 0.3

    if ! test -e /tmp/qemu-vhost-nic-$nic_id ; then
        fail "client failed: we should have a socket in /tmp/qemu-vhost-nic-$nic_id"
    fi
}

function nic_add_bypass_trace {
    but_id=$1
    nic_id=$2
    vni=$3

    f=/tmp/butterfly.req
    echo "[butterfly-$but_id] add nic $nic_id with vni $vni (filtering bypass and packet tracing)"


    cli $but_id 0 nic add --id "nic-$nic_id" --mac "52:54:00:12:34:0$nic_id" --vni $vni --ip "42.0.0.$nic_id" --bypass-filtering --packet-trace true
    sleep 0.3

    if ! test -e /tmp/qemu-vhost-nic-$nic_id ; then
        fail "client failed: we should have a socket in /tmp/qemu-vhost-nic-$nic_id"
    fi
}

function nic_del {
    but_id=$1
    nic_id=$2
    echo "[butterfly-$but_id] delete nic $nic_id"

    cli $but_id 0 nic del "nic-$nic_id"
    sleep 0.3
}

function sg_rule_add_all_open {
    but_id=$1
    sg=$2
    echo "[butterfly-$but_id] add rule all open in $sg"

    cli $but_id 0 sg rule add $sg --dir in --ip-proto all --cidr 0.0.0.0/0 + sg rule add $sg --dir in --ip-proto all --cidr ::0/0
}

function sg_rule_add_port_open {
    protocol=$1
    but_id=$2
    port=$3
    sg=$4
    echo "[butterfly-$but_id] add rule $protocol port $port open in $sg"
    cli $but_id 0 sg rule add $sg --dir in --ip-proto $protocol --port-start $port --port-end $port --cidr 0.0.0.0/0
}

function sg_rule_add_ip_and_port_direction {
    protocol=$1
    but_id=$2
    ip=$3
    mask_size=$4
    port=$5
    sg=$6
    dir=$7
    echo "[butterfly-$but_id] add rule $protocol port $port ip $ip/$mask_size in $sg"
    cli $but_id 0 sg rule add $sg --dir $dir --ip-proto $protocol --port-start $port --port-end $port --cidr $ip/$mask_size
}


function sg_rule_add_ip_and_port {
    sg_rule_add_ip_and_port_direction $1 $2 $3 $4 $5 $6 in
}

function sg_rule_del_ip_and_port_direction {
    protocol=$1
    but_id=$2
    ip=$3
    mask_size=$4
    port=$5
    sg=$6
    dir=$7
    echo "[butterfly-$but_id] del rule $protocol port $port ip $ip/$mask_size in $sg"

    cli $but_id 0 sg rule del $sg --dir $dir --ip-proto $protocol --port-start $port --port-end $port --cidr $ip/$mask_size
}

function sg_rule_del_ip_and_port {
    sg_rule_del_ip_and_port_direction $1 $2 $3 $4 $5 $6 "in"
}

function sg_rule_add_ip_direction {
    but_id=$1
    ip=$2
    mask_size=$3
    sg=$4
    dir=$5

    echo "[butterfly-$but_id] add rule to $sg: allow $ip/$mask_size on all protocols"

    cli $but_id 0 sg rule add $sg --dir $dir --ip-proto -1 --cidr $ip/$mask_size
}

function sg_rule_add_ip {
    sg_rule_add_ip_direction $1 $2 $3 $4 "in"
}

function sg_rule_del_ip_direction {
    but_id=$1
    ip=$2
    mask_size=$3
    sg=$4
    dir=$5

    echo "[butterfly-$but_id] delete rule on $sg: allow $ip/$mask_size on all protocols"

    cli $but_id 0 sg rule del $sg --dir $dir --ip-proto -1 --cidr $ip/$mask_size
}

function sg_rule_del_ip {
    sg_rule_del_ip_direction $1 $2 $3 $4 "in"
}

function sg_rule_add_with_sg_member {
    protocol=$1
    sg=$2
    but_id=$3
    port=$4
    sg_member=$5
    echo "[butterfly-$but_id] add rule to $sg: allow sg members of $sg_member on $protocol:$port"
    cli $but_id 0 sg rule add $sg --dir in --ip-proto $protocol --port-start $port --port-end $port --sg-members $sg_member
}

function sg_rule_del_with_sg_member {
    protocol=$1
    sg=$2
    but_id=$3
    port=$4
    sg_member=$5
    echo "[butterfly-$but_id] delete rule from $sg: allow sg members of $sg_member on $protocol:$port"

    cli $but_id 0 sg rule del --dir in --ip-proto $protocol --port-start $port --port-end $port --sg-members $sg_member
}

function sg_rule_add_dhcp {
    but_id=$1
    sg=$2
    sg_rule_add_port_open udp $but_id 67 $sg
    sg_rule_add_port_open udp $but_id 68 $sg
}

function sg_rule_del_dhcp {
    but_id=$1
    sg=$2
    sg_rule_del_port_open udp $but_id 67 $sg
    sg_rule_del_port_open udp $but_id 68 $sg
}

function sg_member_add {
    but_id=$1
    sg=$2
    ip_member=$3
    echo "[butterfly-$but_id] add member $ip_member in $sg"

    cli $but_id 0 sg member add $sg $ip_member
}

function nic_set_sg {
    but_id=$1
    nic_id=$2
    sg_list=${@:3}
    echo "[butterfly-$but_id] update nic's security groups of $nic_id to: $sg_list"

    cli $but_id 0 nic sg set "nic-$nic_id" $sg_list
}

function remove_sg_from_nic {
    but_id=$1
    nic_id=$2
    echo "[butterfly-$but_id] remove sg from nic $nic_id"

    cli $but_id 0 nic sg set "nic-$nic_id"
}

function sg_add {
    sg=$1
    but_id=$2
    echo "[butterfly-$but_id] add $sg"

    cli $but_id 0 sg add $sg
}

function sg_del {
    but_id=$1
    sg=$2
    echo "[butterfly-$but_id] delete $sg"

    cli $but_id 0 sg del $sg
}

function sg_rule_del {
    but_id=$1
    sg=$2
    protocol=$3
    port=$4

    if [ "$#" -eq "2" ]; then
        sg_rule_del_all_open $but_id $sg
    fi

    if [ "$#" -eq "4" ]; then
        sg_rule_del_port_open $but_id $sg $protocol $port
    fi
}

function sg_rule_del_all_open {
    but_id=$1
    sg=$2
    echo "[butterfly-$but_id] delete rule all open from $sg"

    cli $but_id 0 sg rule del $sg --dir in --ip-proto -1 --cidr 0.0.0.0/0 + sg rule del $sg --dir in --ip-proto -1 --cidr 0::/0
}

function sg_rule_del_port_open {
    but_id=$1
    sg=$2
    protocol=$3
    port=$4
    echo "[butterfly-$but_id] delete rule $protocol port $port open from $sg"

    cli $but_id 0 sg rule del $sg --dir in --ip-proto $protocol --port-start $port --port-end $port --cidr 0.0.0.0/0 + sg rule del $sg --dir in --ip-proto $protocol --port-start $port --port-end $port --cidr 0::/0
}

function sg_rule_add_icmp {
    but_id=$1
    sg=$2
    echo "[butterfly-$but_id] add rule allowing icmp from $sg"

    cli $but_id 0 sg rule add $sg --dir in --ip-proto 1 --cidr 0.0.0.0/0
}

function sg_rule_del_icmp {
    but_id=$1
    sg=$2
    echo "[butterfly-$but_id] delete rule allowing icmp from $sg"

    cli $but_id 0 sg rule del $sg --dir in --ip-proto 1 --cidr 0.0.0.0/0
}

function sg_rule_add_icmp6 {
    but_id=$1
    sg=$2
    echo "[butterfly-$but_id] add rule allowing icmp6 from $sg"

    cli $but_id 0 sg rule add $sg --dir in --ip-proto 58 --cidr ::0/0
}

function sg_rule_del_icmp6 {
    but_id=$1
    sg=$2
    echo "[butterfly-$but_id] delete rule allowing icmp6 from $sg"

    cli $but_id 0 sg rule del $sg --dir in --ip-proto 58 --cidr 0::0/0
}

function sg_member_del {
    but_id=$1
    sg=$2
    ip_member=$3
    echo "[butterfly-$but_id] delete member $ip_member from $sg"

    cli $but_id 0 sg member del $sg $ip_member
}

function check_bin {
    run=${@:1}
    $run &> /dev/null
    if [ ! "$?" == "0" ]; then
        echo "cannot execute $run: not found"
        exit 1
    fi
}

function clean_pcaps {
    sudo rm -rf /tmp/butterfly-*.pcap
}

function clean_all {
    sudo killall butterflyd butterfly qemu-system-x86_64 socat &> /dev/null
    sleep 0.5
    sudo killall -15 butterflyd butterfly qemu-system-x86_64 socat &> /dev/null
    sudo rm -rf /tmp/*vhost* /dev/hugepages/* /mnt/huge/*  &> /dev/null
    sleep 0.5
    rm -rf $BUTTERFLY_BUILD_ROOT/qemu_pids*
    rm -rf $BUTTERFLY_BUILD_ROOT/*-butterfly_size
    rm -rf /tmp/*-butterfly_size
}

function fail {
    echo $1
    clean_all
    exit 1
}

if [ "$BUTTERFLY_BUILD_ROOT" = "-h" ] || [ "$BUTTERFLY_BUILD_ROOT" = "--help" ] ||
       [ "$BUTTERFLY_SRC_ROOT" = "-h" ] || [ "$BUTTERFLY_SRC_ROOT" = "--help" ] ; then
    usage
    exit 0
fi

if [ ! -f $BUTTERFLY_SRC_ROOT/LICENSE ]; then
    echo "Butterfly's source root not found"
    usage
    exit 1
fi

if [ ! -f $BUTTERFLY_BUILD_ROOT/api/server/butterflyd ]; then
    echo "Butterfly's build root not found (is butterfly built ?)"
    usage
    exit 1
fi

# Check some binaries
check_bin sudo -h
check_bin ssh -V
check_bin sudo qemu-system-x86_64 -h
check_bin sudo socat -h
check_bin kill -l
check_bin killall -l
check_bin nc -h
check_bin wget -h

clean_all
clean_pcaps

download $IMG_URL $IMG_MD5 $BUTTERFLY_BUILD_ROOT/vm.qcow
download $KEY_URL $KEY_MD5 $BUTTERFLY_BUILD_ROOT/vm.rsa
chmod og-rw $BUTTERFLY_BUILD_ROOT/vm.rsa
sudo chown $USER:$USER -R $BUTTERFLY_BUILD_ROOT/api/

# run sudo one time
sudo echo "ready to roll !"

