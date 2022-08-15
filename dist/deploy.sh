#!/usr/bin/env bash
#
# Deploy to a disk image
#

fail() {
	echo $1
	exit 1
}

usage() {
	if [[ ! -z $1 ]]; then
		echo $1
	fi
	fail "$0 -p <portname> -d <diskimage> [-f [-s <size-MB>]] <fileset> [<fileset>...]"
}

if [[ $# -lt 2 ]]; then
	usage
fi
port=""
diskimage=""
sizeMB=""
filesets=""
do_format="no"
while [[ $# -gt 0 ]]; do
	key=$1
	case "${key}" in
		-d)
		shift; diskimage="$1"
		;;
		-f)
		do_format="yes"
		;;
		-p)
		shift; port="../ports/$1"
		;;
		-s)
		shift; sizeMB=$1
		;;
		*)
		filesets="${filesets} $1"
		;;
	esac
	shift
done

if [[ -z "${port}" ]]; then
	usage "missing <port>"
fi
if [[ ! -d "${port}" ]]; then
	fail "port ${port} does not appear to exist"
fi
diskboot="${port}/CMDS/BOOTOBJS/BOOTFILES/diskboot.bf"
if [[ ! -f "${diskboot}" ]]; then
	fail "${diskboot} not found; need to build ${port} first."
fi

if [[ -z "${diskimage}" ]]; then
	usage "missing <diskimage>"
fi

if [[ "${do_format}" == "no" ]]; then
	if [[ ! -f "${diskimage}" ]]; then
		fail "format not requested but ${diskimage} does not exist"
	fi
else
	if [[ -z "${sizeMB}" ]]; then
		if [[ ! -e "${diskimage}" ]]; then
			fail "<size-MB> not specified but ${diskimage} does not exist"
		fi
		sizearg=""
	else 
		if [[ -f ${diskimage} ]]; then
			fail "<size-MB> set but '${diskimage}' appears to exist, refusing to overwrite"
		fi
		sizearg=-l$((${sizeMB}*2048))
	fi
fi

if [[ -z "${filesets}" ]]; then
	usage "need at least one <fileset>"
fi
for set in ${filesets}; do
	if [[ ! -f ${set} ]]; then
		fail "'${set}' does not appear to point to a fileset"
	fi
done


if [[ -z "$MWOS" ]]; then
	fail "Must set MWOS to the root of the OS-9 for 68K SDK"
fi
if [[ ! -d ${MWOS}/OS9 ]]; then
	fail "MWOS does not appear to point to the root of the OS-9 for 68K SDK"
fi
if [[ ! -e "${TOOLSHED}/os9" ]]; then
	fail "TOOLSHED does not appear to point to the location of the Toolshed os9 binary"
fi
format="${TOOLSHED}/os9 format"
gen="${TOOLSHED}/os9 gen "
makdir="${TOOLSHED}/os9 makdir"
copy="${TOOLSHED}/os9 copy -r"
attr="${TOOLSHED}/os9 attr"

# format the image if requested
if [[ "${do_format}" == "yes" ]]; then
	${format} -q -e -k -bs512 ${sizearg} ${diskimage} || fail "format failed"
fi

# make the bootfile bootable
${gen} -b=${diskboot} -e ${diskimage} || fail "gen failed"

# make directories
for i in CMDS SYS USER; do
	${makdir} ${diskimage},$i || fail "makdir $i failed"
done

# copy filesets to the image
for set in ${filesets}; do
	for i in `cat ${set}`; do
		echo $i
		${copy} -o=0 ${MWOS}/$i ${diskimage},CMDS || fail "copy $i failed"
		${attr} -e -pe ${diskimage},CMDS/$i || fail "attr $i failed"
	done
done

# copy app binaries to the image
for i in `find ../apps/bin -type f`; do
	echo $i
	${copy} -o=0 $i ${diskimage},CMDS || fail "copy $i failed"
	${attr} -e -pe ${diskimage},CMDS/$i || fail "attr $i failed"
done

# copy sys files
${copy} -o=0 ${MWOS}/OS9/SRC/SYS/errmsg ${diskimage},SYS
${copy} -o=0 ${MWOS}/OS9/SRC/SYS/termcap ${diskimage},SYS
${copy} -o=0 ${MWOS}/OS9/SRC/SYS/umacs.hlp ${diskimage},SYS
${copy} -o=0 ${MWOS}/OS9/SRC/SYS/moded.fields ${diskimage},SYS
${copy} -o=0 SYS/startup ${diskimage},SYS

