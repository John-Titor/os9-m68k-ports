#!/bin/sh
#
# Kick off makedist in the Wine environment.
#
env -i HOME=${HOME} WINEDEBUG=-all `which wine` cmd /c `dirname $0`/makedist.bat $*
