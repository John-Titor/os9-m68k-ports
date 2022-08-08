#!/usr/bin/env bash
#
# Generate filesets by searching the OS-9 SDK for either CPU-specific
# or generic versions of files.
#

fail() {
	echo $1
	exit 1
}
if [ -z "$MWOS" ]; then
	fail "Must set MWOS to the root of the OS-9 for 68K SDK"
fi
if [ ! -d "${MWOS}/OS9" ]; then
	fail "MWOS does not appear to point to the root of the OS-9 for 68K SDK"
fi

gen_fileset() {
	lists=$1
	paths=$2

	for i in `cat ${lists}`; do

		found=""
		for p in ${paths}; do
			candidate=OS9/${p}/${i}
			if [ -f ${MWOS}/${candidate} ]; then
				found=${candidate}
				break
			fi
		done
		if [ -z "${found}" ]; then
			fail "could not locate '${i}' anywhere in '${paths}'"
		fi
		echo ${found}
	done
}

# Generate base filesets
gen_fileset 'cmds libs' 			'68000/CMDS' 			> files.68000
gen_fileset 'cmds libs' 			'68020/CMDS 68000/CMDS' > files.68020
gen_fileset 'cmds libs' 			'CPU32/CMDS 68000/CMDS' > files.CPU32

# Generate networking filesets, eg.
#gen_fileset 'cmds libs cmds_net' 	'68000/CMDS' 			> files.net.68000
#gen_fileset 'cmds libs cmds_net' 	'68020/CMDS 68000/CMDS' > files.net.68020
#gen_fileset 'cmds libs cmds_net' 	'CPU32/CMDS 68000/CMDS' > files.net.CPU32
