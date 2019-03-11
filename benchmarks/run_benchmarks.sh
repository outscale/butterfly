#!/bin/bash
set -e

function usage {
    echo "This script connects to two machines and launch several performance"
    echo "tests between two machines based on locally build and packaged"
    echo "Butterfly."
    echo "We assume that machines:"
    echo "- Already have their DPDK ports configured"
    echo "- Have their dpdk test ports connected together"
    echo "- Have Qemu installed"
    echo "- Huge pages are configured"
    echo "- SSH keys configured"
    echo "It things goes wrong, run this script with bash -x"
    echo ""
    echo "Usage: run_benchmark.sh"
    echo "    -s | --sources : Butterfly source directory (mandatory)"
    echo "    -b | --build   : Butterfly build directoty (mandatory)"
    echo "    -o | --output  : file where to write benchmark"
    echo "                     (optional, default: output.csv)"
    echo "    --ip-a         : IP address of first machine (optional, default: 127.0.0.1)"
    echo "    --port-a       : SSH port of first machine (optional, default: 22)"
    echo "    --ip-b         : IP address of second machine (optional, default: 127.0.0.1)"
    echo "    --port-b       : SSH port of second machine (optional, default: 22)"
    echo "    --package      : 'rpm' or 'deb' (optional, default: rpm)"
    echo "    --fat          : use fat package instead of clasique one"
    echo "    --qemu-args    : append args to qemu commande"
    echo "    --no-perf      : don't use perf record durring benchmark"
    echo "    --tso-on       : force usage of tso"
    echo "    -t | --time    : duration (seconds) of each benchmark"
    echo "                     (optional, default: 100)"
    echo "    --keep-running : launch endless TCP iperf before leaving"
    echo "    -h | --help    : show this help"
}

# read arguments

tso_on=0
source_dir=""
build_dir=""
run_dir=$(pwd)
ip_a="127.0.0.1"
port_a=22
ip_b="127.0.0.1"
port_b=22
package='rpm'
output="$(pwd)/output.csv"
bench_duration=100
keep_running=0
fat=""
qemu_args=""
no_perf=0

args=`getopt -o s:b:o:t:h:: --long sources:,build:,output:,ip-a:,ip-b:,port-a:,port-b:,package:,time:,keep-running,fat,no-perf,tso-on,qemu-args:,help:: -- "$@"`
eval set -- "$args"
while true ; do
    case "$1" in
        -s|--sources)
            source_dir=$2 ; shift 2 ;;
        -b|--build)
            build_dir=$2 ; shift 2 ;;
        -o|--output)
            output=$2 ; shift 2 ;;
        --ip-a)
            ip_a=$2 ; shift 2 ;;
        --port-a)
            port_a=$2 ; shift 2 ;;
        --ip-b)
            ip_b=$2 ; shift 2 ;;
        --port-b)
            port_b=$2 ; shift 2 ;;
        --package)
            package=$2 ; shift 2 ;;
        --qemu-args)
            qemu_args="$2"; shift 2 ;;
        -t|--time)
            bench_duration=$2 ; shift 2 ;;
        --keep-running)
            keep_running=1 ; shift 1 ;;
        --fat)
            fat="fat-" ; shift 1 ;;
        --no-perf)
            no_perf=1 ; shift 1 ;;
        --tso-on)
            tso_on=1 ; shift 1 ;;
        -h|--help)
            usage; exit 0 ;;
        --)
            break ;;
        *)
            usage; exit 1 ;;
    esac
done

# check arguments

if [ -z "$source_dir" ]; then
    echo -e "Error: Butterfly source dir not set\n"
    usage
    exit 1
fi

if [ -z "$build_dir" ]; then
    echo -e "Error: Butterfly build dir not set\n"
    usage
    exit 1
fi

f=$source_dir/CMakeLists.txt
if [ ! -f $f ]; then
    echo -e "Error: Butterfly source dir seems bad: $f does not exist\n"
    usage
    exit 1
fi

f=$build_dir/CMakeCache.txt
if [ ! -f $f ]; then
    echo -e "Error: Butterfly build dir seems bad: $f does not exist\n"
    usage
    exit 1
fi

case "$package" in
    rpm) ;;
    deb) ;;
    *)
        echo -e "Invalid package type $package\n"
        usage
        exit 1
        ;;
esac

source $source_dir/benchmarks/functions.sh
ping_test $ip_a
ping_test $ip_b

ssh_run $ip_a $port_a true
ssh_run $ip_b $port_b true

# build butterfly
cd $build_dir
rm *.$package || true
make
make package-$fat$package

# clean targets
tmp=/root/tmp_butterfly
clean_hard $ip_a $port_a $tmp
clean_hard $ip_b $port_b $tmp
(cd $run_dir && rm $output) || true

# install butterfly
ssh_run $ip_a $port_a mkdir $tmp
ssh_run $ip_b $port_b mkdir $tmp

remote_cp $ip_a $port_a *.$package $tmp/butterfly.$package
remote_cp $ip_b $port_b *.$package $tmp/butterfly.$package

ssh_run $ip_a $port_a rpm -i $tmp/butterfly.rpm || \
ssh_run $ip_a $port_a dpkg -i $tmp/butterfly.deb
ssh_run $ip_b $port_b rpm -i $tmp/butterfly.rpm || \
ssh_run $ip_b $port_b dpkg -i $tmp/butterfly.deb

# check that remote machines has needed binaries
ssh_run $ip_a $port_a butterflyd -h &> /dev/null
ssh_run $ip_b $port_b butterflyd -h &> /dev/null

ssh_run $ip_a $port_a qemu-system-x86_64 -h &> /dev/null
ssh_run $ip_b $port_b qemu-system-x86_64 -h &> /dev/null

ssh_run $ip_a $port_a test -e /usr/bin/iperf &> /dev/null
ssh_run $ip_b $port_b test -e /usr/bin/iperf &> /dev/null

ssh_run $ip_a $port_a tmux new -d -s $(date +%Y-%m-%d-%H.%M.%S) true &> /dev/null
ssh_run $ip_b $port_b tmux new -d -s $(date +%Y-%m-%d-%H.%M.%S) true &> /dev/null

# launch butterfly
butterfly_start 1 $ip_a $port_a
butterfly_start 2 $ip_b $port_b
sleep 5

# create some network interface
nic_add $ip_a $port_a 1 42
nic_add $ip_a $port_a 2 42
nic_add $ip_b $port_b 3 42

# launch virtual machines
declare -a pids
vtmp=/var/tmp
ssh_run $ip_a $port_a test -e $vtmp/image.qcow || ssh_run $ip_a $port_a wget -q $IMG_URL -O $vtmp/image.qcow &
pids[$!]=$!
ssh_run $ip_b $port_b test -e $vtmp/image.qcow || ssh_run $ip_b $port_b wget -q $IMG_URL -O $vtmp/image.qcow &
pids[$!]=$!
ssh_run $ip_a $port_a test -e $vtmp/image.rsa || ssh_run $ip_a $port_a wget -q $KEY_URL -O $vtmp/image.rsa &
pids[$!]=$!
ssh_run $ip_b $port_b test -e $vtmp/image.rsa || ssh_run $ip_b $port_b wget -q $KEY_URL -O $vtmp/image.rsa &
pids[$!]=$!
wait ${pids[*]};
unset $pids;

ssh_run $ip_a $port_a chmod 600 /var/tmp/image.rsa
ssh_run $ip_b $port_b chmod 600 /var/tmp/image.rsa

# 2 hosts A and B:
# VM 1 is on host A
# VM 2 is on host A
# VM 3 is on host B
declare -a pids;
vm_start $ip_a $port_a 1 $tso_on "$qemu_args" &
pids[$!]=$!
vm_start $ip_a $port_a 2 $tso_on "$qemu_args" &
pids[$!]=$!
vm_start $ip_b $port_b 3 $tso_on "$qemu_args" &
pids[$!]=$!
wait ${pids[*]};
unset $pids;

cd $run_dir

# launch ICMP test between two machines on the same host
icmp_bench $output $ip_a $port_a 1 2 same_host

# launch ICMP test between two machines on different host
icmp_bench $output $ip_a $port_a 1 3

# launch TCP test between two machines on the same host
tcp_bench $output $ip_a $port_a 1 $ip_a $port_a 2

# launch TCP test between two machines on different host
if [ $no_perf -eq 0 ]; then
    perf_start $ip_a $port_a &
    perf_start $ip_b $port_b &
fi
tcp_bench $output $ip_a $port_a 1 $ip_b $port_b 3

# launch UDP test between two machines on the same host
udp_bench $output $ip_a $port_a 1 $ip_a $port_a 2

# launch UDP test between two machines on different host
udp_bench $output $ip_a $port_a 1 $ip_b $port_b 3

if [ $keep_running = "1" ]; then
    ssh_vm $ip_b $port_b 3 iperf -s & &> /dev/null
    sleep 1
    ssh_vm $ip_a $port_a 1 iperf -t 0 -c 42.0.0.3
fi

echo "Benchmark end, check $output file:"
cat $output
