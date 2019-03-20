#!/bin/bash
#
# Use AFTER 'make install'
#
# Creates a tarball of the whole Tao documentation (Tao + modules).
# The layout is suitable for uploading to www.taodyne.com/docs.
#
# Tested on MacOSX only but should work on other platforms, too.

# List names of directories in $1
dirs() {
  [ -d "$1" ] && ( cd "$1" ; for i in * ; do [ -d "$i" ] && echo "$i" ; done )
}


case `uname` in
  Darwin)
    INST="install/Tao3D.app/Contents/MacOS"
    ;;
  MINGW*|Linux)
    INST=install
    ;;
esac

TMPDIR=taodoc_$$
mkdir -p $TMPDIR || exit 1

VER=`app/updaterev.sh -n`
mkdir -p $TMPDIR/taodoc-$VER/help

DEST=$TMPDIR/taodoc-$VER

# Modules

printf "MODULES: "
for mod in `dirs "$INST/modules"` ; do
  BS=""
  printf "$mod ("
  for lang in `dirs "$INST/modules/$mod/doc"` ; do
    printf "$lang "
    if [ -d "$INST/modules/$mod/doc/$lang/html" ] ; then
      mkdir -p $DEST/modules/$mod/$lang
      cp -R "$INST/modules/$mod/doc/$lang/html"/* $DEST/modules/$mod/$lang
    fi
    BS="\b"
  done
  printf "$BS) "
done
echo

# Disabled

## Tao (without modules)
#
#BS=""
#printf "TAO: ("
#for lang in `dirs "$INST/doc"` ; do
#  printf "$lang "
#  if [ -d "$INST/doc/$lang/html" ] ; then
#    mkdir -p $DEST/help/$lang
#    cp -R "$INST/doc/$lang/html"/* $DEST/help/$lang
#  fi
#  BS="\b"
#done
#printf "$BS) "
#echo
#
#fi

# Tao and modules in a self-contained HTML docset

BS=""
printf "TAO+MODULES: "
printf "building... "
(cd doc ; make webdoc 2>/dev/null >/dev/null || echo "make webdoc failed!")
printf "("
for lang in `dirs "doc/webhtml"` ; do
  printf "$lang "
  if [ -d "doc/webhtml/$lang/html" ] ; then
    mkdir -p $DEST/help/$lang
    cp -R "doc/webhtml/$lang/html"/* $DEST/help/$lang
  fi
  BS="\b"
done
printf "$BS) "
echo

# Archive

printf "Packing..."
TARBALL=taodoc-$VER.tar.xz
( cd $TMPDIR && tar cf - taodoc-$VER | xz - >../$TARBALL )
SZ=`ls -lh $TARBALL | awk '{ print $5}'`
echo " $TARBALL ($SZ)"

# Cleanup

rm -rf $TMPDIR
