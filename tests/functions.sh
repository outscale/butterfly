IMG_URL=https://osu.eu-west-2.outscale.com/jerome.jutteau/16d1bc0517de5c95aa076a0584b43af6/arch-100816.qcow
IMG_MD5=1ca000ddbc5ac271c77d1875fab71083
KEY_URL=https://osu.eu-west-2.outscale.com/jerome.jutteau/16d1bc0517de5c95aa076a0584b43af6/arch-100816.rsa
KEY_MD5=eb3d700f2ee166e0dbe00f4e0aa2cef9

function usage {
    echo "Usage: test.sh BUTTERFLY_SRC_ROOT BUTTERFLY_BUILD_ROOT" 1>&2
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

function ssh_bash {
    id=$1
    cmd="${@:2}"
    key=$BUTTERFLY_BUILD_ROOT/vm.rsa
    ssh -p 500$id -l root -i $key -oStrictHostKeyChecking=no 127.0.0.1
}

function ssh_ping {
    id1=$1
    id2=$2
    ssh_run $id1 ping 42.0.0.$id2 -c 1 &> /dev/null
    if [ $? -ne 0 ]; then
        echo "ping $id1 --> $id2 FAIL"
        RETURN_CODE=1
    else
        echo "ping $id1 --> $id2 OK"
    fi
}

function ssh_no_ping {
    id1=$1
    id2=$2
    ssh_run $id1 ping 42.0.0.$id2 -c 1 &> /dev/null
    if [ $? -ne 0 ]; then
        echo "no ping $id1 --> $id2 OK"
    else
        echo "no ping $id1 --> $id2 FAIL"
        RETURN_CODE=1
    fi
}

function ssh_iperf_tcp {
    id1=$1
    id2=$2
    (ssh_run $id1 iperf -s &> /dev/null &)
    local server_pid=$!
    sleep 1
    ssh_run $id2 iperf -c 42.0.0.$id1 -t 3 &> /dev/null
    if [ $? -ne 0 ]; then
        echo "iperf tcp $id1 --> $id2 FAIL"
        RETURN_CODE=1
    else
        echo "iperf tcp $id1 --> $id2 OK"
    fi
    kill $server_pid &> /dev/null
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
        echo "iperf udp $id1 --> $id2 FAIL"
        RETURN_CODE=1
    else
        echo "iperf udp $id1 --> $id2 OK"
    fi
    kill server_pid &> /dev/null
    rm /tmp/iperf_tmp_results
}

function ssh_iperf3_tcp {
    id1=$1
    id2=$2
    (ssh_run $id1 iperf3 -s &> /dev/null &)
    local server_pid=$!
    sleep 1
    ssh_run $id2 iperf3 -c 42.0.0.$id1 -t 3 &> /dev/null
    if [ $? -ne 0 ]; then
        echo "iperf3 tcp $id1 --> $id2 FAIL"
        RETURN_CODE=1
    else
        echo "iperf3 tcp $id1 --> $id2 OK"
    fi
    kill $server_pid &> /dev/null

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
    if [ $res -eq 0 ] || [ $ret -ne 0 ]; then
        echo "iperf3 udp $id1 --> $id2 FAIL"
        RETURN_CODE=1
    else
        echo "iperf3 udp $id1 --> $id2 OK"
    fi
    kill -9 $server_pid &> /dev/null
    rm /tmp/iperf3_tmp_results
}

function ssd_connection_tests_internal {
    protocol=$1
    id1=$2
    id2=$3
    port=$4
    proto_cmd=""

    if [ "$protocol" == "udp" ]; then
	proto_cmd="-u"
    elif [ "$protocol" == "tcp" ]; then
	proto_cmd=""
    else
	echo -e "protocol $protocol not supported by nic_add_port_open"
	RETURN_CODE=1
	return $RETURN_CODE
    fi

    ssh_run_background $id1 "nc $proto_cmd -lp  $port > /tmp/test"
    sleep 0.4
    ssh_run_background $id2 "echo 'this message is from vm $id2' | nc $proto_cmd 42.0.0.$id1 $port"
    return 0
}

function ssh_clean_connection {
    id1=$1
    id2=$2
    
    ssh_run $id1 "rm /tmp/test" &> /dev/null
    ssh_run $id1 "killall nc" &> /dev/null
    ssh_run $id2 "killall nc" &> /dev/null
}

function ssh_connection_test_file {
    timeout=50
    it=0
    while [ $it -ne $timeout ]; do
	ssh_run $1 [ -s "/tmp/test" ]
	if [ "$?" == "0" ]; then
	    return 0
	fi
	sleep 0.1
	it=$( expr $it + 1 )
    done
    return 1
}

function ssh_connection_test {
    protocol=$1
    id1=$2
    id2=$3
    port=$4

    if [ $( ssd_connection_tests_internal $protocol $id1 $id2 $port ) ]; then
	return
    fi

    ssh_connection_test_file $id1
    if [ "$?" == "0" ]; then
	echo -e "$protocol test $id2 --> $id1 OK"
    else
	echo -e "$protocol test $id2 --> $id1 FAIL"
	RETURN_CODE=1
    fi
    ssh_clean_connection $id1 $id2
    return $RETURN_CODE
}

function ssh_no_connection_test {
    protocol=$1
    id1=$2
    id2=$3
    port=$4

    if [  $( ssd_connection_tests_internal $protocol $id1 $id2 $port ) ]; then
	return
    fi
    ssh_run $id1 [ -s "/tmp/test" ]
    if [ "$?" == "0" ]; then
	echo -e "no $protocol test $id2 --> $id1 FAIL"
	RETURN_CODE=1
    else
	echo -e "no $protocol test $id2 --> $id1 OK"
    fi
    ssh_clean_connection $id1 $id2
    return $RETURN_CODE
}

function qemu_start {
    id=$1
    echo "starting VM $id"
    SOCKET_PATH=/tmp/qemu-vhost-nic-$id
    IMG_PATH=$BUTTERFLY_BUILD_ROOT/vm.qcow
    MAC=52:54:00:12:34:0$id

    CMD="sudo qemu-system-x86_64 -netdev user,id=network0,hostfwd=tcp::500${id}-:22 -device e1000,netdev=network0 -m 124M -enable-kvm -chardev socket,id=char0,path=$SOCKET_PATH -netdev type=vhost-user,id=mynet1,chardev=char0,vhostforce -device virtio-net-pci,csum=off,gso=off,mac=$MAC,netdev=mynet1 -object memory-backend-file,id=mem,size=124M,mem-path=/mnt/huge,share=on -numa node,memdev=mem -mem-prealloc -drive file=$IMG_PATH -snapshot -nographic"
    exec $CMD &> /tmp/qemu_log_$id &
    pid=$!
    echo "joe" | nc -w 1  127.0.0.1 500$id &> /dev/null
    TEST=$?
    while  [ $TEST -ne 0 ]
    do
	echo "joe" | nc -w 1  127.0.0.1 500$id &> /dev/null
	TEST=$?
	sleep 0.2
    done
    sudo kill -s 0 $pid &> /dev/null
    if [ $? -ne 0 ]; then
        cat /tmp/qemu_log_$id
        rm /tmp/qemu_log_$id
        echo "failed to start qemu"
        clean_all
        exit 1
    fi
    rm /tmp/qemu_log_$id
    qemu_pids["$id"]=$pid

    # Wait for ssh to be ready
    ssh_run_timeout $id 60 true

    # Configure IP on vhost interface
    ssh_run $id ip link set ens4 up
    ssh_run $id ip addr add 42.0.0.$id/16 dev ens4
}

function qemu_stop {
    id=$1
    echo "stopping VM $id"
    sudo kill -9 $(ps --ppid ${qemu_pids[$id]} -o pid=) &> /dev/null
}

function server_start {
    id=$1
    echo "starting butterfly $id"

    exec sudo $BUTTERFLY_BUILD_ROOT/api/server/butterfly-server --dpdk-args "--no-shconf -c1 -n1 --vdev=eth_pcap$id,iface=but$id --no-huge" -l debug -i noze -s /tmp --endpoint=tcp://0.0.0.0:876$id -t &> /dev/null &
    pid=$!
    sudo kill -s 0 $pid
    if [ $? -ne 0 ]; then
        echo "failed to start butterfly"
        clean_all
        exit 1
    fi

    server_pids["$id"]=$pid
}

function server_stop {
    id=$1
    echo "stopping butterfly $id"
    sudo kill -2 $(ps --ppid ${server_pids[$id]} -o pid=)
    while sudo kill -s 0 ${server_pids[$id]} &> /dev/null ; do
        sleep 0.1
    done
}

function network_connect {
    id1=$1
    id2=$2
    echo "network connect but$id1 <--> but$id2"
    sudo socat TUN:192.168.42.$id1/24,tun-type=tap,iff-running,iff-up,iff-promisc,tun-name=but$id1 TUN:192.168.42.$id2/24,tun-type=tap,iff-running,iff-promisc,iff-up,tun-name=but$id2 &
    pid=$!
    sleep 0.2
    sudo kill -s 0 $pid
    if [ $? -ne 0 ]; then
        echo "failed connect but$id1 and but$id2"
        clean_all
        exit 1
    fi
    socat_pids["$id1$id2"]=$pid
    sudo ip link set dev but$id1 mtu 2000
    sudo ip link set dev but$id2 mtu 2000
}

function network_disconnect {
    id1=$1
    id2=$2
    echo "network disconnect but$id1 <--> but$id2"
    sudo kill -9 $(ps --ppid ${socat_pids[$id1$id2]} -o pid=)
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
    nic_id=$2
    f=$3

    $BUTTERFLY_BUILD_ROOT/api/client/butterfly-client -e tcp://127.0.0.1:876$but_id -i $f
    ret=$?
    rm $f
    if [ ! "$ret" == "0" ]; then
        echo "client failed to send message to butterfly $but_id"
        clean_all
        exit 1
    fi
}

function nic_add {
    sg=$1
    but_id=$2
    nic_id=$3
    vni=$4
    f=/tmp/butterfly-client.req
    echo "add nic $nic_id full open in butterfly $but_id in vni $vni"

    echo -e "messages {
  revision: 0
  message_0 {
    request {
      nic_add {
        id: \"nic-$nic_id\"
        mac: \"52:54:00:12:34:0$nic_id\"
        vni: $vni
        ip: \"42.0.0.$nic_id\"
        ip_anti_spoof: true
        security_group: \"$sg\"
      }
    }
  }
}
messages {
  revision: 0
  message_0 {
    request {
      sg_add {
        id: \"$sg\"
        rule {
          direction: INBOUND
          protocol: -1
          cidr {
            address: \"0.0.0.0\"
            mask_size: 0
          }
        }
      }
    }
  }
}
" > $f
    request $but_id $nic_id $f
    sleep 0.3

    if ! test -e /tmp/qemu-vhost-nic-$nic_id ; then
        echo "client failed: we should have a socket in /tmp/qemu-vhost-nic-$nic_id"
        clean_all
        exit 1
    fi
}

function nic_add_void {
    but_id=$1
    nic_id=$2
    vni=$3
    f=/tmp/butterfly-client.req

    echo "add a void nic $nic_id in butterfly $but_id"
    echo -e "messages {
  revision: 0
  message_0 {
    request {
      nic_add {
        id: \"nic-$nic_id\"
        mac: \"52:54:00:12:34:0$nic_id\"
        vni: $vni
        ip: \"42.0.0.$nic_id\"
        ip_anti_spoof: true
      }
    }
  }
}
" > $f
    request $but_id $nic_id $f
    
    if ! test -e /tmp/qemu-vhost-nic-$nic_id ; then
        echo "client failed: we should have a socket in /tmp/qemu-vhost-nic-$nic_id"
        clean_all
        exit 1
    fi
}

function nic_del {
    but_id=$1
    nic_id=$2
    f=/tmp/butterfly-client.req
    echo "delete nic $nic_id in butterfly $but_id"

    echo -e "messages {
  revision: 0
  message_0 {
    request {
      nic_del: \"nic-$nic_id\"
    }
  }
}
" > $f
    request $but_id $nic_id $f
}

function nic_add_port_open {
    protocol=$1
    sg=$2
    but_id=$3
    nic_id=$4
    vni=$5
    port=$6
    echo "add nic $nic_id $protocol port $port opened in butterfly $but_id in vni $vni"
    if [ "$protocol" == "tcp" ]; then
	protocol=6
    elif [ "$protocol" == "udp" ]; then
	protocol=17
    else
	echo -e "protocol $protocol not supported by nic_add_port_open"
	RETURN_CODE=1
    fi
    f=/tmp/butterfly-client.req

    echo -e "messages {
  revision: 0
  message_0 {
    request {
      nic_add {
        id: \"nic-$nic_id\"
        mac: \"52:54:00:12:34:0$nic_id\"
        vni: $vni
        ip: \"42.0.0.$nic_id\"
        ip_anti_spoof: true
        security_group: \"$sg\"
      }
    }
  }
}
messages {
  revision: 0
  message_0 {
    request {
      sg_add {
        id: \"$sg\"
        rule {
          direction: INBOUND
          protocol: $protocol
          port_start: $port
          port_end: $port
          cidr {
            address: \"0.0.0.0\"
            mask_size: 0
          }
        }
      }
    }
  }
}
" > $f
    request $but_id $nic_id $f

    if ! test -e /tmp/qemu-vhost-nic-$nic_id ; then
        echo "client failed: we should have a socket in /tmp/qemu-vhost-nic-$nic_id"
        clean_all
        exit 1
    fi
}

function nic_add_no_rules {
    sg=$1
    but_id=$2
    nic_id=$3
    vni=$4
    f=/tmp/butterfly-client.req
    nic_add_void $but_id $nic_id $vni
    set_nic_sg $sg $but_id $nic_id
}

function sg_rule_add_full_open {
    sg=$1
    but_id=$2
    nic_id=$3
    echo "add sg rule full open in butterfly $but_id"
    f=/tmp/butterfly-client.req

    echo -e "messages {
  revision: 0
  message_0 {
    request {
      sg_rule_add {
        sg_id: \"$sg\"
        rule {
          direction: INBOUND
          protocol: -1
          cidr {
            address: \"0.0.0.0\"
            mask_size: 0
          }
        }
      }
    }
  }
}
" > $f
    request $but_id $nic_id $f
}

function sg_rule_add_port_open {
    protocol=$1
    sg=$2
    but_id=$3
    nic_id=$4
    port=$5
    echo "add sg rule $protocol port $port opened in butterfly $but_id"
    if [ "$protocol" == "tcp" ]; then
	protocol=6
    elif [ "$protocol" == "udp" ]; then
	protocol=17
    else
	echo -e "protocol $protocol not supported by sg_rule_add_port_open"
	RETURN_CODE=1
    fi
    f=/tmp/butterfly-client.req

    echo -e "messages {
  revision: 0
  message_0 {
    request {
      sg_rule_add {
        sg_id: \"$sg\"
        rule {
          direction: INBOUND
          protocol: $protocol
          port_start: $port
          port_end: $port
          cidr {
            address: \"0.0.0.0\"
            mask_size: 0
          }
        }
      }
    }
  }
}
" > $f
    request $but_id $nic_id $f
}

function set_nic_sg {
    sg=$1
    but_id=$2
    nic_id=$3
    echo "set nic $nic_id SG in butterfly $but_id"
    f=/tmp/butterfly-client.req

    echo -e "messages {
  revision: 0
  message_0 {
    request {
      nic_update {
        id: \"nic-$nic_id\"
        ip: \"42.0.0.$nic_id\"
        ip_anti_spoof: true
        security_group: \"$sg\"
      }
    }
  }
}
" > $f
    request $but_id $nic_id $f
}

function delete_sg {
    sg=$1
    but_id=$2
    echo "delete SG $sg in butterfly $but_id"
    f=/tmp/butterfly-client.req

    echo -e "messages {
  revision: 0
  message_0 {
    request {
      sg_del: \"$sg\"
    }
  }
}
" > $f
    request $but_id 0 $f
}

function sg_rule_del_full_open {
    sg=$1
    but_id=$2
    nic_id=$3
    echo "delete sg rule full open in butterfly $but_id"
    f=/tmp/butterfly-client.req

    echo -e "messages {
  revision: 0
  message_0 {
    request {
      sg_rule_del {
        sg_id: \"$sg\"
        rule {
          direction: INBOUND
          protocol: -1
          cidr {
            address: \"0.0.0.0\"
            mask_size: 0
          }
        }
      }
    }
  }
}
" > $f
    request $but_id $nic_id $f
}

function sg_rule_del_port_open {
    protocol=$1
    sg=$2
    but_id=$3
    nic_id=$4
    port=$5
    if [ "$protocol" == "tcp" ]; then
	protocol=6
    elif [ "$protocol" == "udp" ]; then
	protocol=17
    else
	echo -e "protocol $protocol not supported by sg_rule_del_port_open"
	RETURN_CODE=1
    fi
    echo "delete sg rule $protocol port $port opened in butterfly $but_id"
    f=/tmp/butterfly-client.req

    echo -e "messages {
  revision: 0
  message_0 {
    request {
      sg_rule_del {
        sg_id: \"$sg\"
        rule {
          direction: INBOUND
          protocol: $protocol
          port_start: $port
          port_end: $port
          cidr {
            address: \"0.0.0.0\"
            mask_size: 0
          }
        }
      }
    }
  }
}
" > $f
    request $but_id $nic_id $f
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
    sudo killall -9 butterfly-server butterfly-client qemu-system-x86_64 socat &> /dev/null
    sudo rm -rf /tmp/*vhost* /dev/hugepages/* /mnt/huge/*  &> /dev/null
    sleep 0.5
}

if [ ! -f $BUTTERFLY_SRC_ROOT/LICENSE ]; then
    echo "Butterfly's source root not found"
    usage
    exit 1
fi

if [ ! -f $BUTTERFLY_BUILD_ROOT/api/server/butterfly-server ]; then
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
chmod og-r $BUTTERFLY_BUILD_ROOT/vm.rsa

# run sudo one time
sudo echo "ready to roll !"

