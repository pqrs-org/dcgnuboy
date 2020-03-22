#!/bin/sh

# Little version release util for KOS
# (c)2000 Dan Potter
# version.sh,v 1.1.1.1 2001/09/26 07:05:01 bardtx Exp

# Call this program on each file to substitute in the proper version code
# for the version header. This works with find|xargs.

VERSION=$1
shift
for i in $*; do
	echo processing $i to version $VERSION
	sed -e "s/##version##/$VERSION/g" < $i > /tmp/tmp.out
	mv /tmp/tmp.out $i
done



