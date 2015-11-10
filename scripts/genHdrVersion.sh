#!/bin/bash
# Copyright 2015 Outscale SAS
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
#


#
# This file is used to increment revision at EACH modification of the
# protocol. This does not mean there will be any incompatibility but
# permits to track changes and eventually debug in case of incompatibilities.
# This revision has no link with the "0" in "MessageV0" for example.
#

DATA=`git log -1 | grep Date | awk -F "   " '{ print $2 }' | awk -F '+' '{ print $1 }'`
NAME=`git log --oneline -1 | awk -F ' ' '{print $1}'`

REVISION=0

echo '#ifndef __HDR_VERSION__'
echo '#define __HDR_VERSION__'

echo -n '#define PROTOS_REVISION '
echo $REVISION

echo -n '#define VERSION_INFO "'
echo -n revision: \\\"$REVISION\\\" commit: \\\"$NAME\\\" date: \\\"$DATA\\\" 
echo '"'
echo '#endif'
