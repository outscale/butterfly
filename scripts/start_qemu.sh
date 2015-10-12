#!/bin/sh

if [ -z "$2" ]; then
    echo "Usage: start-qemu.sh SOCKET_PATH IMG_PATH" 1>&2
    exit 1
fi

SOCKET_PATH=$1
IMP_PATH=$2
MAC=$3

CMD="qemu-system-x86_64 -m 512M -enable-kvm -chardev socket,id=char0,path=$SOCKET_PATH -netdev type=vhost-user,id=mynet1,chardev=char0,vhostforce -device virtio-net-pci,mac=$MAC,netdev=mynet1 -object memory-backend-file,id=mem,size=512M,mem-path=/mnt/huge,share=on -numa node,memdev=mem -mem-prealloc  -drive file=$IMP_PATH --curses  "
echo -n 'executing: '
echo $CMD
exec $CMD
