#!/bin/bash
#
# Copyright 2017 Outscale SAS
#
# This file is part of Butterfly.
#
# Butterfly is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 3 as published
# by the Free Software Foundation.
#
# Butterfly is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Butterfly.  If not, see <http://www.gnu.org/licenses/>.

set -eu

# Create a user with the same uid/gid
but_path=/butterfly
user_id=$(stat --printf=%u $but_path)
group_id=$(stat --printf=%g $but_path)
groupadd -g $group_id user || echo "group id $group_id already exists"
useradd -u $user_id -g user user || echo "user id $user_id already exists"
user=$(getent passwd $user_id | cut -d : -f 1)
home=$(getent passwd $user_id | cut -d : -f 6)

# Build Butterfly & packages
cd $home
su $user -c "mkdir -p build"
cd build
su $user -c "cmake $but_path"
su $user -c "make"
su $user -c "make package-deb"
su $user -c "make package-rpm"
su $user -c "mojopack.sh api/client/butterfly"
su $user -c "mojopack.sh api/server/butterflyd"

# Move output files
dst=$but_path/packages
su $user -c "mkdir -p $dst"
mv *.deb $dst
mv *.rpm $dst
mv butterfly.sh $dst
mv butterflyd.sh $dst

echo "Congratulation, packages are now located in ./packages folder"
