#!/bin/bash
# Prepend Tao directory to LD_LIBRARY_PATH and run program

SCRIPT=$(readlink -f "$0")
TAODIR=$(dirname "$SCRIPT")
LD_LIBRARY_PATH=${TAODIR}${LD_LIBRARY_PATH+:$LD_LIBRARY_PATH} ${TAODIR}/Tao\ Presentations "$@"
