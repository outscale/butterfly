#!/bin/bash

set -e

function usage {
    echo "This script build and upload packages to S3"
    echo
    echo "Usage: $0"
    echo "    -b           : Butterfly build directoty (mandatory)"
    echo "    -e           : S3 endpoint (mandatory)"
    echo "    --ak         : S3 access key (mandatory)"
    echo "    --sk         : S3 secret key (mandatory)"
    echo "    --path       : prefix path where to send files, must end with '/' (mandatory)"
    echo "    -h | --help  : show this help"
    echo
    echo "example: ./upload_packages.sh -e osu.eu-west-2.outscale.com -b ../build --ak XXX --sk XXX --path s3://butterfly/builds/"
}

# read arguments

source_dir=$(cd "$(dirname $0)/.." && pwd)
build_dir=""
endpoint=""
ak=""
sk=""
path=""

args=`getopt -o b:e:h:: --long ak:,sk:,path:,help:: -- "$@"`
eval set -- "$args"
while true ; do
    case "$1" in
        -b|--build)
            build_dir=$(cd "$2" && pwd); shift 2 ;;
        -e|--endpoint)
            endpoint=$2 ; shift 2 ;;
        --ak)
            ak=$2 ; shift 2 ;;
        --sk)
            sk=$2 ; shift 2 ;;
        --path)
            path=$2 ; shift 2 ;;
        -h|--help)
            usage; exit 0 ;;
        --)
            break ;;
        *)
            usage; exit 1 ;;
    esac
done

# check arguments

if [ -z "$build_dir" ]; then
    echo -e "Error: Butterfly build dir not set\n"
    usage
    exit 1
fi

if [ -z "$endpoint" ]; then
    echo -e "Error: S3 endpoint not set\n"
    usage
    exit 1
fi

if [ -z "$ak" ]; then
    echo -e "Error: access key not set\n"
    usage
    exit 1
fi

if [ -z "$sk" ]; then
    echo -e "Error: secret key not set\n"
    usage
    exit 1
fi

if [ -z "$path" ]; then
    echo -e "Error: path not set\n"
    usage
    exit 1
fi

if [ "${path: -1}" != "/" ] ; then
    path="${path}/"
fi

function check_bin {
    run=${@:1}
    $run &> /dev/null
    if [ ! "$?" == "0" ]; then
        echo "cannot execute $run: not found"
        exit 1
    fi
}

check_bin s3cmd --help

build_date=$(date +%d%m%y)
build_hash=$(git log --oneline -1 | awk -F ' ' '{print $1}')
prefix="${build_date}_$build_hash"

echo "-- Build packages"
cd $build_dir
$source_dir/scripts/build_package.sh $build_dir rpm
$source_dir/scripts/build_package.sh $build_dir deb

echo "-- Push packages on S3"
for p in *.rpm *.deb ; do
    dst=$path${prefix}_$p
    s3cmd put --acl-public --access_key=$ak --secret_key=$sk --host=$endpoint --host-bucket='%(bucket).$e' $p $dst
    extension=${p##*.}
    dst2="${path}lastest.$extension"
    s3cmd cp --access_key=$ak --secret_key=$sk --host=$endpoint --host-bucket='%(bucket).$e' $dst $dst2
done

echo "-- Build fat packages"
$source_dir/scripts/build_fat_package.sh $build_dir rpm
$source_dir/scripts/build_fat_package.sh $build_dir deb

echo "-- Push fat packages on S3"
for p in *.rpm *.deb ; do
    dst=$path${prefix}_fat_$p
    s3cmd put --acl-public --access_key=$ak --secret_key=$sk --host=$endpoint --host-bucket='%(bucket).$e' $p $dst
    extension=${p##*.}
    dst2="${path}lastest_fat.$extension"
    s3cmd cp --access_key=$ak --secret_key=$sk --host=$endpoint --host-bucket='%(bucket).$e' $dst $dst2
done
