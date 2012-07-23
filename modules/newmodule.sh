#!/bin/bash
#
# Interactive script to create a new module for Tao Presentations.
#
# Usage: ./newmodule.sh

function fatal() {
  echo "*** Fatal: $@"
  exit 1
}

function error() {
  echo "*** Error: $@"
}

# $1 = variable name [in/out]
# Note: default value is printed only if variable is set, even empty.
function ask() {
  local varname=$1
  local current;
  shift
  local txt="$@"
  [ "$txt" ] || txt="Value for $varname?"
  printf "$txt"
  local notset=""
  eval "[ -z \"\${${varname}+xxx}\" ] && notset=1"
  if [ -z "$notset" ] ; then
    current=$(eval echo \$$varname)
    printf " [$current]"
  fi
  printf ": "
  read $varname
  local newval=$(eval echo \$$varname)
  [ -z "$newval" ] && eval ${varname}=\"$current\" # restore previous value
  # "" means empty value
  local e=$(echo $newval | tr \" X)
  [ "$e" = "XX" ] && eval ${varname}=\"\"
}

# $1 = variable name
# $2 = name of validation function (returns true if input is accepted)
# $3 = text
function ask_validate() {
  local fn=$1
  local varname=$2
  shift
  local notok=1
  local wasnotset=""
  eval "[ -z \"\${${varname}+xxx}\" ] && wasnotset=1"
  local default=$(eval echo \$$varname)
  while [ "$notok" ] ; do
    ask "$@"
    # Call validation function
    eval $fn $varname && notok=
    if [ "$notok" ] ; then
      if [ "$wasnotset" ] ; then
        unset $varname
      else
        eval ${varname}=\"$default\"
      fi
    fi
  done
}

function validate_not_empty() {
  local val=$(eval echo \$$1)
  if [ "$val" ] ; then
    true
  else
    error "the value cannot be empty."
    false
  fi
}

function validate_yes_no() {
  local val=$(eval echo \$$1)
  local notok=1
  [ "$val" = "y" ] && notok=
  [ "$val" = "n" ] && notok=
  if [ "$notok"  ] ; then
    error "please enter 'y' or 'n'."
    false
  else
    true
  fi
}

function ask_not_empty() {
    ask_validate validate_not_empty "$@"
}

function ask_yes_no() {
    ask_validate validate_yes_no "$@"
}

function capitalize() {
  local word=$1
  echo -n ${word:0:1} | tr 'a-z' 'A-Z'
  echo ${word:1}
}

# module_name => ModuleName
function tocamelcase() {
  wordArr=(${1//_/ })
  for i in "${wordArr[@]}" ; do
    echo -n $(capitalize $i)
  done
}

function help_text() {
  echo "> $1"
}

function new_uuid() {
  case $(uname) in
    MINGW*)
      unset uuid ; ask_not_empty uuid "UUID"
      echo $uuid
      ;;
    *)
      uuidgen
      ;;
  esac
}

function set_vars() {
  echo
  echo "=== Module creation script ==="
  echo
  help_text "Note: default values are shown like this: [default]."
  help_text "Just hit <return> to accept the default. Enter \"\" to specify an empty value."
  echo

  unset MODULE_DIR ; ask_not_empty MODULE_DIR "Directory name (example: lens_flare)"
  help_text "Import name"
  help_text "When empty (\"\"), the module will auto-load on startup (example: most display modules)."
  help_text "Note that there is a separate setting to force a named module to auto-load."
  SHORT_NAME=$(tocamelcase $MODULE_DIR)
  IMPORT_NAME=$SHORT_NAME ; ask IMPORT_NAME "Import name"
  if [ "$IMPORT_NAME" ] ; then
    help_text "Auto-load"
    help_text "Modules that have an import name are usually loaded on demand with 'import'."
    help_text "However, it is possible to have a named module be loaded on Tao startup."
    help_text "For example: a display module that exports some functions."
    WITH_AUTO_LOAD=n ; ask_yes_no WITH_AUTO_LOAD "Auto-load module on application startup?"
    WITH_IMPORT_NAME=y
  fi
  NAME="$SHORT_NAME module" ; ask_not_empty NAME "Module name (example: Lens Flare effect)"
  DESCRIPTION="$SHORT_NAME module" ; ask_not_empty DESCRIPTION "Description"
  NAME_FR="" ; ask NAME_FR "Module name (french)"
  DESCRIPTION_FR="" ; ask DESCRIPTION_FR "Description (french)"
  VERSION="1.0" ; # ask VERSION "Version"
  AUTHOR="Taodyne SAS" ; ask AUTHOR "Author"
  WEBSITE="http://www.taodyne.com" ; ask WEBSITE "Web site"
  WITH_LICENSE=n ; ask_yes_no WITH_LICENSE "Create license file template (.taokey.notsigned)?"
  [ "$WITH_LICENSE" = "y" ] && FEATURES="$SHORT_NAME 1\\.0.*"
  help_text "Encryption"
  help_text "To have the main .xl file import a secondary, encrypted .xl file, answer 'y'"
  help_text "to the next question. The script will then create $MODULE_DIR.xl and"
  help_text "${MODULE_DIR}2.xl. \"make install\" will encrypt ${MODULE_DIR}2.xl as"
  help_text "${MODULE_DIR}2.xl.crypt, and $MODULE_DIR.xl will import ${MODULE_DIR}2.xl.crypt."
  WITH_CRYPT=n ; ask_yes_no WITH_CRYPT "Crypt .xl file?"
  WITH_DOC=y ; ask_yes_no WITH_DOC "Create documentation files (Doxygen)?"

  COPYRIGHT_YEAR=$(date "+%Y")
  ID=$(new_uuid)

  echo $AUTHOR | grep -n Taodyne >/dev/null && AUTHOR_IS_TAODYNE=1
}

# Echo all configuration variables: VAR=value
function dump_vars() {
    local vars="MODULE_DIR SHORT_NAME IMPORT_NAME NAME DESCRIPTION NAME_FR DESCRIPTION_FR"
    vars="$vars VERSION AUTHOR WEBSITE COPYRIGHT_YEAR ID FEATURES"
    for varname in $vars ; do
      echo "$varname=$(eval echo \$$varname)"
    done
}

# Prepares the sed command to substitute env vars, and the defines for the Perl
# preprocessor.
# Call once before calling edit.
pp_defines=""
function edit_setup() {
  f_sedscript=$(mktemp /tmp/newmodule.sed.XXXXX)
  dump_vars | sed 's/[\%]/\\&/g;s/\([^=]*\)=\(.*\)/s%@@\1@@%\2%/' > $f_sedscript
  for varname in WITH_DOC WITH_LICENSE WITH_CRYPT WITH_IMPORT_NAME WITH_AUTO_LOAD ; do
    local val=$(eval echo \$$varname)
    [ "$val" = "y" ] && pp_defines="$pp_defines -D$varname"
  done
}

# $1 path to template file
# - Pre-process #ifdef's
# - Replace all occurrences of @@VAR_NAME@@ in file by the value of the
# environment variable VAR_NAME
function edit() {
  perl ../tools/preprocessor.pl $pp_defines $1 | sed -f $f_sedscript
}

function create_module() {

  echo
  echo "Creating files module under ./$MODULE_DIR/"

  local M=$MODULE_DIR
  mkdir -p $M
  edit_setup

  # .pro
  edit newmodule/newmodule.pro.t > $M/$M.pro

  # .xl
  edit newmodule/newmodule.xl.t  > $M/$M.xl
  local has_fr=""
  [ "$NAME_FR" ] && has_fr=y
  [ "$DESCRIPTION_FR" ] && has_fr=y
  if [ "$has_fr" ] ; then
    {
      echo
      echo "module_description \"fr\","
      [ "$NAME_FR" ] && echo "    name \"$NAME_FR\""
      [ "$DESCRIPTION_FR" ] && echo "    description \"$DESCRIPTION_FR\""
    } >> $M/$M.xl
  fi

  # .taokey.notsigned
  [ "$WITH_LICENSE" = "y" ] && edit newmodule/newmodule.taokey.notsigned.t > $M/$M.taokey.notsigned

  # doc/Doxyfile.in
  if [ "$WITH_DOC" = "y" ] ; then
    mkdir -p $M/doc
    edit newmodule/doc/Doxyfile.in.t > $M/doc/Doxyfile.in
    edit newmodule/doc/newmodule.doxy.h.t > $M/doc/$M.doxy.h
  fi
}

function cleanup() {
  rm -f $f_sedscript
}

###

set_vars
create_module
cleanup
