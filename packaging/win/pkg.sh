#!/bin/bash
#
# Copy application files into temporary directory 
# before running packaging script
#
# Usage: pkg.sh [debug | release]

MODE=debug
if test "$1" ; then
  MODE="$1"
fi

doo() {
  echo $*
  eval $*
}

THISDIR=`dirname $0`

#
# Copy Tao application and dependencies into buildroot/
#

OBJDIR=../../tao/$MODE
SRCDIR=../../tao
(
cd $THISDIR
export PATH=$PATH:.
rm -rf buildroot
mkdir buildroot
mkdir buildroot/fonts
doo cp $OBJDIR/Tao.exe buildroot/
SFILES="builtins.xl xl.syntax xl.stylesheet git.stylesheet srcview.stylesheet srcview.css tutorial.ddd"
for f in $SFILES ; do
    doo cp $SRCDIR/$f buildroot/
done
FONTS="unifont-5.1.20080907.ttf"
for f in $FONTS ; do
    doo cp $SRCDIR/fonts/$f buildroot/fonts
done
for f in `bin/ldd buildroot/Tao.exe | \
          grep -v -i 'windows/system' | \
          grep -v -i 'ntdll.dll' | \
          grep -v -i 'comctl' | \
          sed 's/^.*=> \\(.*\\)(0x.*)$/\\1/' | \
          sed 's@/cygdrive@@'` ; 
do 
    doo cp $f buildroot/
done
)

#
# Copy SshAskPass and Detach utilities
#
doo cp ../../ssh_ask_pass/$MODE/SshAskPass.exe buildroot/
doo cp ../../detach/$MODE/Detach.exe buildroot/

#
# Uncompress Git distribution into buildroot/
#

GITKIT=PortableGit-1.7.0.2-preview20100309.7z
(
cd $THISDIR
mkdir -p buildroot/git
cd buildroot/git
../../bin/7z x -bd ../../3rdparty/$GITKIT
)
