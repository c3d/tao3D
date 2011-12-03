# Encryption of XL source files
#
# Convert XL source files (.xl) into an encrypted form (.xl.crypt).
# Resulting files are automatically added to INSTALLS.
#
# Usage: In module_name/module_name.pro:
#   CRYPT_XL_SOURCES = filename.xl
#   include(../crypt_xl.pri)

isEmpty(MODINSTPATH):error(Please include modules.pri before crypt_xl.pri)

!isEmpty(CRYPT_XL_SOURCES) {

  for(file, CRYPT_XL_SOURCES):CRYPT_XL_FILES += $$replace(file, .xl, .xl.crypt)

  cryptxl.path = $$MODINSTPATH
  cryptxl.commands = for f in $$CRYPT_XL_SOURCES ; do $$TAOTOPSRC/crypt/crypt.sh <\$\$f >\$\$f.crypt ; done ; cp $$CRYPT_XL_FILES \"$$MODINSTPATH/\"
  cryptxl.files = $$CRYPT_XL_FILES
  cryptxl.depends = $$CRYPT_XL_SOURCES
  QMAKE_EXTRA_TARGETS += cryptxl
  INSTALLS += cryptxl
  QMAKE_CLEAN += $$CRYPT_XL_FILES
}
