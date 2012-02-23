#!/bin/bash
#
# Find dependencies of executable files. See usage below.

# All executables found
f_executables=$(mktemp /tmp/sd_executables.XXXXX)
# Direct dependencies (lib names with no path)
# E.g., libQtCore.so.4 libfontconfig.so.1
f_needed=$(mktemp /tmp/sd_needed.XXXXX)
# All dependencies (libs with full paths)
# E.g., /usr/lib/i386-linux-gnu/libQtCore.so.4 /usr/lib/i386-linux-gnu/libfontconfig.so.1
f_ldd_libs=$(mktemp /tmp/sd_ldd_libs.XXXXX)
# Direct dependencies (libs with full paths)
f_libs=$(mktemp /tmp/sd_libs.XXXXX)
# Packages that provide above libs
f_packages=$(mktemp /tmp/sd_packages.XXXXX)


# executables <path_to_dir>
function executables() {
  find $1 -type f -a -executable
}

# libraries_f <executable_file>
function libraries_f() {
  ldd "$1" | awk '/not found/ { next } /=> \// { print $3 }'
}

# libraries_d <directory>
function libraries_d() {
  executables "$1" >$f_executables
  cat $f_executables | \
    while read e ; do
      libraries "$e"
    done
}

# libraries <file or directory>
function libraries() {
  if [ ! -e "$1" ] ; then
    echo "File does not exist or is unreadable: $1" >&2
    return
  fi

  if [ -f "$1" ] ; then
    libraries_f "$1" >>$f_ldd_libs
  else
    if [ -d "$1" ] ; then
      libraries_d "$1"
    else
      echo "Unknown file type: $1" >&2
    fi
  fi
}

# needed_f <executable_file>
function needed_f() {
  objdump -p "$1" 2>/dev/null | awk '/ NEEDED / { print $2 }'
}

# needed_d <directory>
function needed_d() {
  executables "$1" | { \
    while read e; do
      needed "$e"
    done;
  }
}

# needed <file or directory>
function needed() {
  if [ ! -e "$1" ] ; then
    echo "File does not exist or is unreadable: $1" >&2
    return
  fi

  if [ -f "$1" ] ; then
    needed_f "$1" >>$f_needed
  else
    if [ -d "$1" ] ; then
      needed_d "$1"
    else
      echo "Unknown file type: $1" >&2
    fi
  fi
}

# package <file>
function package() {
  dpkg -S "$1" | sed 's/:.*//'
}

if [ $# -lt 1 ] ; then
  echo "Usage: $0 [file|dir]..."
  echo ""
  echo "Shows all the runtime packages required by one or more executable file(s)."
  echo "Only direct dependencies are considered."
  echo ""
  echo "  dir    Find all executable binary file under 'dir', and add"
  echo "         them to the list of files to process."
  echo "  file   Add 'file' to the list of files to process."
  echo ""
  echo "Example:"
  echo "    make prepare"
  echo "    $0 debian"
  exit 1
fi

echo -n "Reading direct dependencies... "

while [ ! -z "$1" ] ; do
  needed "$1"
  libraries "$1"
  shift
done

# Remove duplicates
cat $f_needed | sort | uniq >$f_needed.tmp
mv $f_needed.tmp $f_needed
cat $f_ldd_libs | sort | uniq >$f_ldd_libs.tmp
mv $f_ldd_libs.tmp $f_ldd_libs

# Filter out indirect dependencies
cat $f_ldd_libs | while read lib ; do
    grep -q $(basename "$lib") $f_needed && echo "$lib" >>$f_libs
done

#echo "NEEDED"
#cat $f_needed
#echo "LDD_LIBS"
#cat $f_ldd_libs
#echo "LIBS"
#cat $f_libs

fcount=$(cat $f_executables | wc -l)
count=$(cat $f_libs | wc -l)
echo "$fcount executables need $count libraries (direct)"
echo -n "Finding packages... "

# List packages and remove duplicates
cat $f_libs | while read lib ; do
    package "$lib" >>$f_packages
done
cat $f_packages | sort | uniq >$f_packages.tmp
mv $f_packages.tmp $f_packages

count=$(cat $f_packages | wc -l)
echo "$count packages found"
cat $f_packages

[ -e "$f_executables" ] && rm $f_executables
[ -e "$f_needed" ] && rm $f_needed
[ -e "$f_ldd_libs" ] && rm $f_ldd_libs
[ -e "$f_libs" ] && rm $f_libs
[ -e "$f_packages" ] && rm $f_packages
