IMG_URL=https://osu.eu-west-2.outscale.com/jerome.jutteau/16d1bc0517de5c95aa076a0584b43af6/arch-260417.qcow
IMG_MD5=1ca000ddbc5ac271c77d1875fab71083
KEY_URL=https://osu.eu-west-2.outscale.com/jerome.jutteau/16d1bc0517de5c95aa076a0584b43af6/arch-260417.rsa
KEY_MD5=eb3d700f2ee166e0dbe00f4e0aa2cef9

function ssh_run {
    local ip=$1
    local port=$2
    local cmd="${@:3}"
    ssh -p $port -l root $ip $cmd
}

function ping_test {
    local ip=$1
    if ! ping -c 1 $ip &> /dev/null; then
        exit 1
    fi
}

function remote_cp {
    local ip=$1
    local port=$2
    local local_file=$3
    local dist_file=$4
    scp -P $port $local_file root@$ip:$dist_file
}

function clean_hard {
    local ip=$1
    local port=$2
    local tmp=$3
    ssh_run $ip_a $port_a pkill -h &> /dev/null
    ssh_run $ip $port pkill --signal 9 -f butterflyd || true
    ssh_run $ip $port pkill --signal 9 -f butterfly || true
    ssh_run $ip $port pkill --signal 9 -f qemu-system-x86_64 || true
    ssh_run $ip $port rm -rf /tmp/*vhost* /dev/hugepages/* /mnt/huge/* $tmp || true
    ssh_run $ip $port yum remove -y butterfly &> /dev/null || true
    ssh_run $ip $port apt-get remove --purge -q butterfly &> /dev/null || true
}

function butterfly_start {
    local id=$1
    local ip=$2
    local port=$3
    ssh_run $ip $port "echo [general] > /tmp/butt-config.ini"
    ssh_run $ip $port "echo dpdk-args=--no-shconf -c1 -n2 --socket-mem 64 --huge-unlink >> /tmp/butt-config.ini"
    ssh_run $ip $port "echo nic-mtu=max >> /tmp/butt-config.ini"
    ssh_run $ip $port tmux new -d -s $(date +%Y-%m-%d-%H.%M.%S) \'butterflyd -c /tmp/butt-config.ini -l debug -i 43.0.0.$id -s /tmp\'
    sleep 5
    ssh_run $ip $port pgrep -f butterflyd
}

function perf_start {
    sleep 1
    ssh_run $1 $2 'perf record -e cycles,instructions,cache-misses -t $(cat /sys/fs/cgroup/cpu/butterfly/tasks) -o ~/perf.data sleep' $((bench_duration - 2))
}

function nic_add {
    local ip=$1
    local port=$2
    local id=$3
    local vni=$4
    f=/tmp/butterfly.req

    echo -e "messages {
  revision: 0
  message_0 {
    request {
      nic_add {
        id: \"nic-$id\"
        mac: \"52:54:00:12:34:0$id\"
        vni: $vni
	bypass_filtering: false
        ip: \"42.0.0.$id\"
        ip_anti_spoof: false
        security_group: \"sg-1\"
      }
    }
  }
}
messages {
  revision: 0
  message_0 {
    request {
      sg_add {
        id: \"sg-1\"
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
}" > $f
    remote_cp $ip $port $f $f
    ssh_run $ip $port butterfly request $f -e tcp://$ip:9999
}

function ssh_vm {
    local ip=$1
    local port=$2
    local id=$3
    local cmd="${@:4}"
    ssh -p $port -l root $ip ssh -p 600$id -l root -i /var/tmp/image.rsa -oStrictHostKeyChecking=no -oConnectTimeout=60 127.0.0.1 $cmd
}

function vm_start {
    local ip=$1
    local port=$2
    local id=$3
    local tso_on=$4
    local qemu_extra_args="$5"
    SOCKET_PATH=/tmp/qemu-vhost-nic-$id
    IMG_PATH=/var/tmp/image.qcow
    KEY_PATH=/var/tmp/image.rsa
    MAC=52:54:00:12:34:0$id

    CMD="qemu-system-x86_64 $qemu_extra_args -cpu host -smp 1 -netdev user,id=network0,hostfwd=tcp::600${id}-:22 -device e1000,netdev=network0 -m 256M -enable-kvm -chardev socket,id=char0,path=$SOCKET_PATH -netdev type=vhost-user,id=mynet1,chardev=char0,vhostforce -device virtio-net-pci,mac=$MAC,netdev=mynet1 -object memory-backend-file,id=mem,size=256M,mem-path=/mnt/huge,share=on -numa node,memdev=mem -mem-prealloc -drive file=$IMG_PATH -snapshot -display none"
    ssh_run $ip $port $CMD &

    sleep 5
    ssh_run $ip $port pgrep -f qemu-system-x86_64

    # Wait for ssh to be ready
    ssh_vm $ip $port $id true || ssh_vm $ip $port $id true || ssh_vm $ip $port $id true

    # Configure IP on vhost interface
    ssh_vm $ip $port $id ip link set ens4 up
    ssh_vm $ip $port $id ip addr add 42.0.0.$id/16 dev ens4
    echo tso : $tso_on
    if [ $tso_on -eq 1 ]; then
        ssh_vm $ip $port $id pacman -Sy --noconfirm archlinux-keyring
        ssh_vm $ip $port $id pacman -Sy --noconfirm ethtool
        ssh_vm $ip $port $id ethtool -K ens4 tso on
    fi
    #ssh_vm $ip $port $id ip link set dev ens4 mtu 1400
}

function icmp_bench {
    local output=$1
    local ip=$2
    local port=$3
    local from_id=$4
    local to_id=$5
    local same_host=$6
    local tmp=/tmp/bench_output
    local state="ok"
    local min=""
    local avg=""
    local max=""
    local geo=""
    ssh_vm $ip $port $from_id ping 42.0.0.$to_id -c 1 || true
    ssh_vm $ip $port $from_id ping 42.0.0.$to_id -c $bench_duration > $tmp || state="ko" 

    if [ "$state" = "ok" ]; then
        min=$(cat $tmp | tail -n 1 | cut -d ' ' -f 4 | cut -d '/' -f 1)
        avg=$(cat $tmp | tail -n 1 | cut -d ' ' -f 4 | cut -d '/' -f 2)
        max=$(cat $tmp | tail -n 1 | cut -d ' ' -f 4 | cut -d '/' -f 3)
    fi
    if ! [ ".$same_host" = "." ]; then
        geo="same host"
    else
        geo="different host"
    fi
    echo "PING;$state;$geo;$from_id;$to_id;$min;$avg;$max" >> $output
}

function tcp_bench {
    local output=$1
    local ip_client=$2
    local port_client=$3
    local id_client=$4
    local ip_server=$5
    local port_server=$6
    local id_server=$7
    local tmp=/tmp/bench_output
    local state="ok"
    local r=""
    local geo=""
    local port=$((RANDOM % (65535 - 1025) + 1025))
    ssh_vm $ip_server $port_server $id_server iperf3 -p $port -s &
    local p=$!
    sleep 1
    ssh_vm $ip_client $port_client $id_client iperf3 -p $port -t $bench_duration -c 42.0.0.$id_server --zerocopy --no-delay --json > $tmp || true
    kill $p
    if [ ".$(cat $tmp)" = "." ]; then
        state="ko"
    else
        r=$(cat $tmp | grep bits_per_second | tail -n 1 | cut -d ':' -f 2 | tr -d '\t')
    fi
    if [ $ip_client = $ip_server ] && [ $port_client = $port_server ]; then
        geo="same host"
    else
        geo="different host"
    fi
    echo "TCP;$state;$geo;$id_client;$id_server;$r" >> $output
}

function udp_bench {
    local output=$1
    local ip_client=$2
    local port_client=$3
    local id_client=$4
    local ip_server=$5
    local port_server=$6
    local id_server=$7
    local tmp=/tmp/bench_output
    local state="ok"
    local r=""
    local geo=""
    local port=$((RANDOM % (65535 - 1025) + 1025))
    ssh_vm $ip_server $port_server $id_server iperf3 -p $port -s &
    local p=$!
    sleep 1
    ssh_vm $ip_client $port_client $id_client iperf3 -p $port -l 1400 -t $bench_duration -c 42.0.0.$id_server --zerocopy -u -b 10G --reverse --json > $tmp || true
    sleep 1
    if [ ".$(cat $tmp)" = "." ]; then
        state="ko"
    else
        r=$(cat $tmp | grep bits_per_second | tail -n 1 | cut -d ':' -f 2 | tr -d '\t')
    fi
    if [ $ip_client = $ip_server ] && [ $port_client = $port_server ]; then
        geo="same host"
    else
        geo="different host"
    fi
    echo "UDP;$state;$geo;$id_client;$id_server;$r" >> $output
}
