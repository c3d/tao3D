#!/bin/bash
#
usage(){
    echo "Usage: $0 [-s] <tag1> <tag2>"
    echo ""
    echo "Use 'git log' to extract tracker IDs between two tags/commits."
    echo ""
    echo "Examples: $0 1.0-beta3 1.0-beta4"
    echo "          $0 1.0-beta3 HEAD" 
    echo "Options:" 
    echo "  -f      Filter only bug fixes (\"Fixes\") and features (\"Closes\")."
    echo "          Default is to show all tracker numbers (#id) that appear in git log output." 
    echo "  -h      Strip \# before numbers." 
    echo "  -s      Split bug fixes and features in two separate lists. Implies -f." 
    echo "  -q      Quiet: just list(s) of numbers (no text)." 
    echo "  -a      Sort IDs in ascending order (default is descending order)." 
    exit 1
}

if [ $# -lt 2 ] ; then
  usage
fi

HASH=1
REVERT=r
SHOW_ALL=1
while [ "$1" ] ; do
  case "$1" in
    -f) SHOW_ALL="";;
    -s) SPLIT=1;;
    -h) HASH="";;
    -q) QUIET=1;;
    -a) REVERT="";;
    *)
      if [ -z "$TAG1" ] ; then
        TAG1="$1"
      else
        if [ -z "$TAG2" ] ; then
          TAG2="$2"
        else
          usage
        fi
      fi
      ;; 
  esac
  shift
done
FIXED=`git log --pretty=oneline $TAG1..$TAG2 | grep -e '[Ff]ixes #' | sed 's/.*[Ff]ixes #\([0-9][0-9]*\).*/\1/' | sort -n$REVERT | uniq`
CLOSED=`git log --pretty=oneline $TAG1..$TAG2 | grep -e '[Cc]loses #' | sed 's/.*[Cc]loses #\([0-9][0-9]*\).*/\1/' | sort -n$REVERT | uniq`
FIXED_AND_CLOSED=`for i in $FIXED $CLOSED ; do echo $i ; done | sort -n$REVERT`
ALL=`git log --pretty=oneline $TAG1..$TAG2 | awk '{ split($0, array) ; for (item in array) { if (match($item, "#[0-9][0-9]+")) { print substr($item, RSTART+1, RLENGTH-1) } } }' | sort -n$REVERT | uniq`
[ "$HASH" ] && FIXED=`for i in $FIXED ; do echo \#$i ; done`
[ "$HASH" ] && CLOSED=`for i in $CLOSED ; do echo \#$i ; done`
[ "$HASH" ] && FIXED_AND_CLOSED=`for i in $FIXED_AND_CLOSED ; do echo \#$i ; done`
[ "$HASH" ] && ALL=`for i in $ALL ; do echo \#$i ; done`

if [ "$SHOW_ALL" ] ; then
    [ "$QUIET" ] || echo "All tracker IDs:"
    echo $ALL 
else
  if [ "$SPLIT" ] ; then
    [ "$QUIET" ] || echo "(Bugs) fixed:"
    echo $FIXED
    [ "$QUIET" ] || echo "(Feats) closed:"
    echo $CLOSED
  else
    [ "$QUIET" ] || echo "Bugs and fetures: "
    echo $FIXED_AND_CLOSED 
  fi
fi
