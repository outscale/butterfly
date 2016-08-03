IMG_URL=https://osu.eu-west-2.outscale.com/jerome.jutteau/16d1bc0517de5c95aa076a0584b43af6/arch-051115.qcow
IMG_MD5=4b7b1a71ac47eb73d85cdbdc85533b07
KEY_URL=https://osu.eu-west-2.outscale.com/jerome.jutteau/16d1bc0517de5c95aa076a0584b43af6/arch-051115.rsa
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

function ssh_connection_test {
    protocol=$1
    id1=$2
    id2=$3
    port=$4

    if [ "$protocol" == "udp" ]; then
	ssh_run_background $id1 "nc -w 3 -lup $port > /tmp/test"
	sleep 1
	ssh_run_background $id2 "echo 'this message is from vm $id2' | nc -u -w 3 42.0.0.$id1 $port"
	sleep 1
    elif [ "$protocol" == "tcp" ]; then
	ssh_run_background $id1 "nc -w 3 -lp $port > /tmp/test"
	sleep 1
	ssh_run_background $id2 "echo 'this message is from vm $id2' | nc -w 3 42.0.0.$id1 $port"
	sleep 1
    else
	echo -e "protocol not set, check protocol please"
	protocol="false"
	RETURN_CODE=1
    fi

    if [ "$protocol" != "false" ]; then
	ssh_run $id1 [ -s "/tmp/test" ]
	if [ "$?" == "0" ]; then
	    echo -e "$protocol test $id2 --> $id1 OK"
	else
	    echo -e "$protocol test $id2 --> $id1 FAIL"
	    RETURN_CODE=1
	fi
	ssh_run $id1 "rm /tmp/test"
    fi
}

function ssh_no_connection_test {
    protocol=$1
    id1=$2
    id2=$3
    port=$4

    if [ "$protocol" == "udp" ]; then
	ssh_run_background $id1 "nc -w 3 -lup $port > /tmp/test"
	sleep 1
	ssh_run_background $id2 "echo 'this message is from vm $id2' | nc -u -w 1 42.0.0.$id1 $port"
	sleep 1
    elif [ "$protocol" == "tcp" ]; then
	ssh_run_background $id1 "nc -w 3 -lp $port > /tmp/test"
	sleep 1
	ssh_run_background $id2 "echo 'this message is from vm $id2' | nc -w 3 42.0.0.$id1 $port"
	sleep 1
    else
	echo -e "protocol not set, check protocol please"
	protocol="false"
	RETURN_CODE=1
    fi

    if [ "$protocol" != "false" ]; then
	ssh_run $id1 [ -s "/tmp/test" ]
	if [ "$?" == "0" ]; then
	    echo -e "no $protocol test $id2 --> $id1 FAIL"
	    RETURN_CODE=1
	else
	    echo -e "no $protocol test $id2 --> $id1 OK"
	fi
	ssh_run $id1 "rm /tmp/test"
    fi
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
    sleep 10
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
    while ! ssh_run_timeout $id 60 true ; do
        sleep 1
    done

    # Configure IP on vhost interface
    ssh_run $id ip link set ens4 up
    ssh_run $id ip addr add 42.0.0.$id/16 dev ens4

    ssh_run $id "pacman -Sy --noconfirm pacman" &> /dev/null
    ssh_run $id "pacman -Sy --noconfirm netcat" &> /dev/null
}

function qemu_stop {
    id=$1
    echo "stopping VM $id"
    sudo kill -9 $(ps --ppid ${qemu_pids[$id]} -o pid=)
}

function server_start {
    id=$1
    echo "starting butterfly $id"

    CMD="sudo $BUTTERFLY_BUILD_ROOT/api/server/butterfly-server --no-shconf -c1 -n1 --vdev=eth_pcap$id,iface=but$id --no-huge -- -l debug -i noze -s /tmp --endpoint=tcp://0.0.0.0:876$id -t"
    exec $CMD &> /dev/null &
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
    sudo kill -9 $(ps --ppid ${server_pids[$id]} -o pid=)
    while sudo kill -s 0 ${server_pids[$id]} &> /dev/null ; do
        sleep 1
    done
}

function network_connect {
    id1=$1
    id2=$2
    echo "network connect but$id1 <--> but$id2"
    sudo socat TUN:192.168.42.$id1/24,tun-type=tap,iff-running,iff-up,iff-promisc,tun-name=but$id1 TUN:192.168.42.$id2/24,tun-type=tap,iff-running,iff-promisc,iff-up,tun-name=but$id2 &
    pid=$!
    sleep 1
    sudo kill -s 0 $pid
    if [ $? -ne 0 ]; then
        echo "failed connect but$id1 and but$id2"
        clean_all
        exit 1
    fi
    socat_pids["$id1$id2"]=$pid
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

function add_nic {
    sg=$1
    but_id=$2
    nic_id=$3
    vni=$4
    f=/tmp/butterfly-client.req
    echo "add nic $nic_id in butterfly $but_id in vni $vni"

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

    if ! test -e /tmp/qemu-vhost-nic-$nic_id ; then
        echo "client failed: we should have a socket in /tmp/qemu-vhost-nic-$nic_id"
        clean_all
        exit 1
    fi
}

function add_nic_void {
    but_id=$1
    nic_id=$2
    vni=$3
    f=/tmp/butterfly-client.req

    echo "add a void nic $2 in butterfly $1"
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

function delete_nic {
    but_id=$1
    nic_id=$2
    f=/tmp/butterfly-client.req
    echo "del nic $nic_id in butterfly $but_id"

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

function add_nic_port_open {
    sg=$1
    but_id=$2
    nic_id=$3
    vni=$4
    protocol=$5
    port=$6
    f=/tmp/butterfly-client.req
    echo "add nic $nic_id with only port $port opened in butterfly $but_id in vni $vni"

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

function add_nic_no_rules {
    sg=$1
    but_id=$2
    nic_id=$3
    vni=$4
    f=/tmp/butterfly-client.req
    add_nic_void $but_id $nic_id $vni
    update_nic_sg $sg $but_id $nic_id
}

function update_sg_rule_full_open {
    sg=$1
    but_id=$2
    nic_id=$3
    echo "update sg rules full open in butterfly $but_id"
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

function update_sg_rule_port_open {
    sg=$1
    but_id=$2
    nic_id=$3
    protocol=$4
    port=$5
    echo "update sg rules with only port $port opened in butterfly $but_id"
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

function update_nic_sg {
    sg=$1
    but_id=$2
    nic_id=$3
    echo "Update SG of nic $nic_id in butterfly $1"  
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

function delete_nic_sg {
    sg=$1
    but_id=$2
    nic_id=$3
    echo "delete SG of nic $nic_id in butterfly $but_id"
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
    request $but_id $nic_id $f
}

function delete_rule_full_open {
    sg=$1
    but_id=$2
    nic_id=$3
    echo "delete sg rules full open in butterfly $but_id"
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

function delete_rule_port_open {
    sg=$1
    but_id=$2
    nic_id=$3
    protocol=$4
    port=$5
    echo "update sg rules with only port $port opened in butterfly $but_id"
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

function clean_all {
    sudo killall -9 butterfly-server butterfly-client qemu-system-x86_64 socat &> /dev/null
    sudo rm -rf /tmp/*vhost* /dev/hugepages/* /mnt/huge/* &> /dev/null
    sleep 1
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

download $IMG_URL $IMG_MD5 $BUTTERFLY_BUILD_ROOT/vm.qcow
download $KEY_URL $KEY_MD5 $BUTTERFLY_BUILD_ROOT/vm.rsa
chmod og-r $BUTTERFLY_BUILD_ROOT/vm.rsa

# Check some binaries
check_bin sudo -h
check_bin ssh -V
check_bin sudo qemu-system-x86_64 -h
check_bin sudo socat -h
clean_all

# run sudo one time
sudo echo "ready to roll !"

