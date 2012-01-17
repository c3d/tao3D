#!/bin/sh
#
# Wrapper script to the doxygen program
# Invokes Doxygen once per output language
#
# Usage:
#
#  doxygen.sh [Doxyfile]

DOXYFILE="$1"
[ "$1" = "" ] && DOXYFILE=Doxyfile
[ "$DOXYLANG" = "" ] && DOXYLANG=en

longname() {
    case $1 in
        en) echo "English" ;;
        fr) echo "French" ;;
    esac
}

doo(){
  echo "[doxygen.sh] $@"
  "$@"
}

if [ -e "$DOXYFILE" ] ; then
    LANGUAGES=`echo $DOXYLANG | tr , ' '`
    for lang in $LANGUAGES ; do
        htmlout=output/$lang/html
        qchout=output/$lang/qch
        (
            cat "$DOXYFILE" ;
            echo OUTPUT_LANGUAGE=`longname $lang` ;
            echo HTML_OUTPUT=$htmlout
        ) > Doxyfile.tmp
        doo mkdir -p $htmlout
        if [ "$QHP_ADDFILES" != "" ] ; then
            qhelpgenerator=`cat Doxyfile.tmp | grep '^QHG_LOCATION' | sed 's/QHG_LOCATION *= *//'`
            qchfile=`cat Doxyfile.tmp | grep '^QCH_FILE' | sed 's/QCH_FILE *= *//'`
            # Doxygen should not run qhelpgenerator yet
            echo QHG_LOCATION= >> Doxyfile.tmp
        fi
        doo doxygen Doxyfile.tmp
        rm -f Doxyfile.tmp
        if [ -e $htmlout/index.qhp -a "$QHP_ADDFILES" != "" -a "$qhelpgenerator" != "" ] ; then
          toadd=""
          for f in $QHP_ADDFILES ; do
            toadd="<file>$f</file>$toadd"
          done
          echo "[doxygen.sh] # Patching $htmlout/index.qhp: adding $toadd"
          (
            cd $htmlout
            cat index.qhp | sed "s@<files>@<files>$toadd@" > index2.qhp
            mv index2.qhp index.qhp
            doo $qhelpgenerator index.qhp -o $qchfile
          )
        fi
    done
else
    echo $DOXYFILE not found or unreadable >&2
    exit 1
fi
