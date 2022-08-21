#!/bin/sh
#
# Kick off make in the Wine environment.
#
env -i HOME=${HOME} WINEDEBUG=-all `which wine` cmd /c `dirname $0`/make.bat $*
