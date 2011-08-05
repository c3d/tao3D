#!/bin/bash
#
usage(){
    echo "Usage: $0 [options...] <tag1> <tag2>"
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
    echo "  -html   Generate HTML output (link IDs to Redmine)."
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
    -s) SPLIT=1;SHOW_ALL="";;
    -h) HASH="";;
    -q) QUIET=1;;
    -a) REVERT="";;
    -html) HTML="1";;
    *)
      if [ -z "$TAG1" ] ; then
        TAG1="$1"
      else
        if [ -z "$TAG2" ] ; then
          TAG2="$1"
        else
          usage
        fi
      fi
      ;; 
  esac
  shift
done

SPACE=" "
[ "$HTML" ] && SPACE="&nbsp;"
NL="\n"
[ "$HTML" ] && NL="<br>"


FIXED=`git log --pretty=oneline $TAG1..$TAG2 | grep -e '[Ff]ixe\?s\? *#' | sed 's/.*[Ff]ixe\?s\? *#\([0-9][0-9]*\).*/\1/' | sort -n$REVERT | uniq`
CLOSED=`git log --pretty=oneline $TAG1..$TAG2 | grep -e '[Cc]loses\? *#' | sed 's/.*[Cc]loses\? *#\([0-9][0-9]*\).*/\1/' | sort -n$REVERT | uniq`
FIXED_AND_CLOSED=`for i in $FIXED $CLOSED ; do echo $i ; done | sort -n$REVERT`
ALL=`git log --pretty=oneline $TAG1..$TAG2 | awk '{ split($0, array) ; for (item in array) { if (match($item, "#[0-9][0-9]+")) { print substr($item, RSTART+1, RLENGTH-1) } } }' | sort -n$REVERT | uniq`

print_ids() {
  eval IDS='$'$1
  for i in $IDS ; do
    [ "$HTML" ] && printf "<a href=\"https://nano.taodyne.com/redmine/issues/$i\">"
    [ "$HASH" ] && printf "#"
    printf $i
    [ "$HTML" ] && printf "</a>"
    printf "$SPACE"
  done
}

ALL=`print_ids ALL`
FIXED=`print_ids FIXED`
CLOSED=`print_ids CLOSED`
FIXED_AND_CLOSED=`print_ids FIXED_AND_CLOSED`

[ "$HTML" ] && echo "<html><body>"
[ "$QUIET" ] || printf "$TAG1 .. $TAG2$NL"
if [ "$SHOW_ALL" ] ; then
    [ "$QUIET" ] || printf "All tracker IDs: "
    printf "$ALL""$NL" 
else
  if [ "$SPLIT" ] ; then
    [ "$QUIET" ] || printf "(Bugs) fixed: "
    printf "$FIXED""$NL"
    [ "$QUIET" ] || printf "(Feats) closed: "
    printf "$CLOSED""$NL"
  else
    [ "$QUIET" ] || printf "Bugs and fetures: "
    printf "$FIXED_AND_CLOSED""$NL"
  fi
fi
[ "$HTML" ] && echo "</body></html>"
