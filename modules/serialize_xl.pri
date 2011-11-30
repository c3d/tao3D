# Serialization of XL source files
#
# Convert XL source files (.xl) into their binary form (.xl.bin).
# Resulting files are automatically added to INSTALLS.
#
# Usage: In module_name/module_name.pro:
#   BIN_XL_SOURCES = filename.xl
#   include(../serialize_xl.pri)

isEmpty(MODINSTPATH):error(Please include modules.pri before serialize_xl.pri)

!isEmpty(BIN_XL_SOURCES) {

  for(file, BIN_XL_SOURCES):BIN_XL_FILES += $$replace(file, .xl, .xl.bin)

  binxl.path = $$MODINSTPATH
  binxl.commands = for f in $$BIN_XL_SOURCES ; do $$TAOTOPSRC/xlconv/xlconv.sh <\$\$f >\$\$f.bin ; done ; cp $$BIN_XL_FILES \"$$MODINSTPATH/\"
  binxl.files = $$BIN_XL_FILES
  binxl.depends = $$BIN_XL_SOURCES
  QMAKE_EXTRA_TARGETS += binxl
  INSTALLS += binxl
  QMAKE_CLEAN += $$BIN_XL_FILES
}
