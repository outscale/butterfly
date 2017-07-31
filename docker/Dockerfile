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

# Butterfly build env
FROM centos:centos7.3.1611
LABEL vendor="Outscale SAS"
LABEL com.outscale.butterfly.description="Butterfly build env"

RUN yum update -y
RUN yum install -y libpcap libpcap-devel cmake clang git make automake autoconf rubygems ruby-devel ruby-devel gcc make rpm-build glibc-static libstdc++-static glib2-devel libtool gcc-c++ zlib-devel openssl-devel openssl-libs numactl numactl-devel
RUN yum clean all
RUN gem install --no-ri --no-rdoc fpm
ADD http://cbs.centos.org/kojifiles/packages/jemalloc/3.6.0/8.el7.centos/x86_64/jemalloc-devel-3.6.0-8.el7.centos.x86_64.rpm jemalloc-devel.rpm
ADD http://cbs.centos.org/kojifiles/packages/jemalloc/3.6.0/8.el7.centos/x86_64/jemalloc-3.6.0-8.el7.centos.x86_64.rpm jemalloc.rpm
RUN rpm -i jemalloc-devel.rpm jemalloc.rpm && rm -rf jemalloc-devel.rpm jemalloc.rpm
ADD https://gitlab.com/mojo42/mojopack/raw/master/mojopack.sh /usr/bin/mojopack.sh
RUN chmod a+rx /usr/bin/mojopack.sh
