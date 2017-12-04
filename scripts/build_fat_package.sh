#!/bin/bash

# Butterfly root
BUTTERFLY_BUILD_ROOT="$( cd "$1" && pwd )"
TARGET=$2
BUTTERFLY_ROOT=$(cd "$(dirname $0)/.." && pwd)


function usage {
    echo "build_fat_package.sh BUTTERFLY_BUILD_ROOT TARGET"
    echo "BUTTERFLY_BUILD_ROOT: absolute path to Bitterfly build root"
    echo "TARGET: deb, rpm, etc.. depending on fpm"
}

# Check if fpm is present
fpm -h &> /dev/null
if [ $? != 0 ]; then
    echo "You need fpm installed. Check https://github.com/jordansissel/fpm"
    usage
    exit 1
fi

# Test Butterfly build root
if [ ! -f $BUTTERFLY_BUILD_ROOT/CMakeCache.txt ]; then
    echo "BUTTERFLY_BUILD_ROOT invalid"
    usage
    exit 1
fi

# Test Butterfly root
if [ ! -d $BUTTERFLY_ROOT/api ]; then
    echo "BUTTERFLY_ROOT invalid"
    usage
    exit 1
fi

# Test package type
if [ -z $TARGET ]; then
    echo "TARGET not set"
    usage
    exit 1
fi

INSTALL_ROOT=$BUTTERFLY_BUILD_ROOT/install_root
ORIGINAL_DIR=$(pwd)
BUTTERFLY_VERSION=$(cat $BUTTERFLY_BUILD_ROOT/butterfly_version)

# Clean old files/folders
rm -rf $INSTALL_ROOT &> /dev/null
rm -rf $BUTTERFLY_BUILD_ROOT/*.$TARGET

export DESTDIR=$INSTALL_ROOT
cd $BUTTERFLY_BUILD_ROOT && make package-fat

# FIXME: can't force cmake to use /usr prefix instead of /usr/local
mkdir $INSTALL_ROOT
cd $INSTALL_ROOT
ls -R
mkdir opt/
mkdir -p etc/butterfly/
mkdir -p usr/bin/
cp -rv ../fat* opt/
echo '#!/bin/bash' > usr/bin/butterfly
echo '#!/bin/bash' > usr/bin/butterflyd
chmod +x usr/bin/butterfly
chmod +x usr/bin/butterflyd
echo '/opt/fat-butterfly/butterfly.sh $@' >> usr/bin/butterfly
echo '/opt/fat-butterflyd/butterflyd.sh $@' >> usr/bin/butterflyd

cp $BUTTERFLY_ROOT/api/server/butterflyd.ini etc/butterfly/
# Path to systemd units distributed with installed RPM packages
mkdir -p usr/lib/systemd/system
cp $BUTTERFLY_ROOT/scripts/butterfly.service usr/lib/systemd/system/


cd $ORIGINAL_DIR
fpm -s dir \
    -t $TARGET \
    -n butterfly \
    -C $INSTALL_ROOT \
    -p butterfly_VERSION_ARCH_fat.$TARGET \
    --url https://github.com/outscale/butterfly \
    --version $BUTTERFLY_VERSION \
    --iteration 1 \
    --license GPLv3 \
    --description "Butterfly is a virtual switch permiting to isolate, connect and filter traffic between virtual machines using VXLAN." \
    --vendor "Outscale SAS" \
    usr opt etc

# clean
rm -rf $INSTALL_ROOT
