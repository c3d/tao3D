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

longname() {
    case $1 in
        en) echo "English" ;;
        fr) echo "French" ;;
    esac
}

if [ -e "$DOXYFILE" ] ; then
    for lang in en fr ; do
        htmlout=output/$lang/html
        qchout=output/$lang/qch
        set -x
        (
            cat "$DOXYFILE" ;
            echo OUTPUT_LANGUAGE=`longname $lang` ;
            echo HTML_OUTPUT=$htmlout
        ) > Doxyfile.tmp
        mkdir -p $htmlout
        doxygen Doxyfile.tmp
        rm -f Doxyfile.tmp
        set +x
    done
else
    echo $DOXYFILE not found or unreadable >&2
    exit 1
fi
