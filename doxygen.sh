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
[ "$DOXYOUTPUT" = "" ] && DOXYOUTPUT=output

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

# If specified Doxyfile contains an @INCLUDE command, return the path to
# the directory that contains the included file.
find_included_doxyfile_dir() {
    local included=`cat $1 | awk '/@INCLUDE/{print $3}'`
    [ "$included" ] && dirname $included
}

# Look for a default layout file for a given Doxyfile and language
find_doxygenlayout() {
    local doxyfile=$1
    local lang=$2

    local doxyfile_dir=`dirname $doxyfile`
    local included_doxyfile_dir=`find_included_doxyfile_dir $doxyfile`
    totest="$doxyfile_dir/DoxygenLayout_$lang.xml"
    [ "$included_doxyfile_dir" ] && totest="$totest $included_doxyfile_dir/DoxygenLayout_$lang.xml"
    totest="$totest $doxyfile_dir/DoxygenLayout.xml"
    [ "$included_doxyfile_dir" ] && totest="$totest $included_doxyfile_dir/DoxygenLayout.xml"
    for t in $totest ; do
       if [ -e "$t" ] ; then
           echo $t
           return
       fi
    done
}

# Filter some Doxygen warnings we can't fix easily.
filter_doxy_warnings() {
  # This one is caused by the syntax we use to mimic XL types, such as in:
  #   foo (W:real, H:real);
  grep -v "is not found in the argument list"
}

# For portability
_mktemp() {
  case $(uname) in
    MINGW*)
      if [ "$1" = "-d" ] ; then
        isdir = 1
      fi
      dst=$(echo $1 | sed 's/\.X\+/.'$$'/')
      [ -e $dst ] && dst=$dst-
      if [ "$isdir" ] ; then
        mkdir -p $dst
      else
        touch $dst
      fi
      echo $dst
      ;;
    *)
      mktemp $@
      ;;
  esac
}

# Find value of variable $1 in Doxyfile file $2
# Also process include files (@INCLUDE =)
find_variable_value() {
  varname=$1
  filename=$2
  OUT=`_mktemp /tmp/doxygen_sh_out.XXX`
  cat $filename | while read line ; do
    case $line in
      @INCLUDE*)
        includefile=`echo $line | grep '^@INCLUDE' | sed 's/@INCLUDE *= *//'`
        find_variable_value $varname $includefile
        ;;
      $varname*)
        echo $line | grep "^$varname" | sed 's/'$varname' *= *//' | tr -d '"'
        ;;
      *)
        ;;
    esac
  done | tail -1 >$OUT
  cat $OUT
  rm $OUT
}

if [ -e "$DOXYFILE" ] ; then
    LANGUAGES=`echo $DOXYLANG | tr , ' '`
    for lang in $LANGUAGES ; do
        doxygenlayout=`find_doxygenlayout $DOXYFILE $lang`
        [ "$doxygenlayout" ] && echo "[doxygen.sh] # Using layout file: $doxygenlayout"
        htmlout=$DOXYOUTPUT/$lang/html
        qchout=$DOXYOUTPUT/$lang/qch
        (
            cat "$DOXYFILE" ;
            [ "$doxygenlayout" ] && echo LAYOUT_FILE = $doxygenlayout ;
            echo OUTPUT_LANGUAGE=`longname $lang` ;
            echo HTML_OUTPUT=$htmlout
        ) > Doxyfile.tmp
        doo mkdir -p $htmlout
        if [ "$QHP_ADDFILES" != "" ] ; then
            qhelpgenerator=`find_variable_value QHG_LOCATION Doxyfile.tmp`
            qchfile=`find_variable_value QCH_FILE Doxyfile.tmp`
            if [ -z "$qchfile" ] ; then
              projectname=`find_variable_value PROJECT_NAME Doxyfile.tmp`
              projectnumber=`find_variable_value PROJECT_NUMBER Doxyfile.tmp`
              qchfile="../qch/$projectname-$projectnumber.qch"
            fi
            # Doxygen should not run qhelpgenerator yet
            echo QHG_LOCATION= >> Doxyfile.tmp
        fi
        { doo doxygen Doxyfile.tmp "# NB: stderr filtered to remove some messages" 2>&1 1>&3 | filter_doxy_warnings 1>&2; } 3>&1
        rm -f Doxyfile.tmp
        if [ -e $htmlout/index.qhp -a "$QHP_ADDFILES" != "" -a "$qhelpgenerator" != "" ] ; then
          toadd=""
          for f in $(echo $QHP_ADDFILES | tr , " "); do
            toadd="<file>$f</file>$toadd"
          done
          echo "[doxygen.sh] # Patching $htmlout/index.qhp: adding $toadd"
          (
            cd $htmlout
            cat index.qhp | sed "s@<files>@<files>$toadd@" > index2.qhp
            mv index2.qhp index.qhp
            doo $qhelpgenerator index.qhp -o "$qchfile"
          )
        fi
    done
else
    echo $DOXYFILE not found or unreadable >&2
    exit 1
fi
