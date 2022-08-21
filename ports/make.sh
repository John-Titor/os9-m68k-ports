#!/bin/sh
#
# Kick off make in the Wine environment.
#
env -i HOME=${HOME} WINEDEBUG=-all `which wine` cmd /c make.bat $*
