#!/bin/bash
NAME=$( basename $1 )
mkdir $2
cp $( ldd $1 | grep '=>' | cut -d ' ' -f 3 ) $2/
cp $( ldd /bin/bash | tail -n 1 | cut -d ' ' -f 1 ) $2/
echo '#!/bin/bash' >> $2/$NAME.sh
echo 'cd $( dirname "$0" )' >> $2/$NAME.sh
echo "./ld-linux-x86-64.so.2 --library-path ./ ./$NAME \$@" >> $2/$NAME.sh
chmod +x $2/$NAME.sh
cp $1 $2/
