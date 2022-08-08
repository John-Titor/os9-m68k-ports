#!/usr/bin/env bash
#
# Deploy to a disk image
# 
# deploy.sh <fileset> <diskimage>
#
fail() {
	echo $1
	exit 1
}
if [ $# != 2 ]; then
	fail "$0 <fileset> <diskimage>"
fi
if [ -z "$MWOS" ]; then
	fail "Must set MWOS to the root of the OS-9 for 68K SDK"
fi
if [ ! -d ${MWOS}/OS9 ]; then
	fail "MWOS does not appear to point to the root of the OS-9 for 68K SDK"
fi
if [ ! -e "${TOOLSHED}/os9" ]; then
	fail "TOOLSHED does not appear to point to the location of the Toolshed os9 binary"
fi
if [ ! -f "../ports/${PORT}/CMDS/BOOTOBJS/BOOTFILES/cf.bf" ]; then
	fail "PORT does not appear to name a port with a CF bootfile ready to deploy"
fi
if [ ! -f $1 ]; then
	fail "'$1' does not appear to point to a fileset"
fi
if [ -e $2 ]; then
	fail "'$2' appears to already exist, refusing to overwrite"
fi

# create a 60MB image suitable for even a small CF card
${TOOLSHED}/os9 format -q -e -k -bs512 -l122180 $2
${TOOLSHED}/os9 gen -b=../ports/${PORT}/CMDS/BOOTOBJS/BOOTFILES/cf.bf -e $2
${TOOLSHED}/os9 makdir $2,CMDS
${TOOLSHED}/os9 makdir $2,SYS

for i in `cat $1`; do
	echo $i
	${TOOLSHED}/os9 copy -r ${MWOS}/$i $2,CMDS
done
