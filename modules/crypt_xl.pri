# Encryption of XL source files
#
# Convert XL source files (.xl) into an encrypted form (.xl.crypt).
# Resulting files are automatically added to INSTALLS.
#
# Usage: In module_name/module_name.pro:
#   CRYPT_XL_SOURCES = filename.xl
#   include(crypt_xl.pri)

isEmpty(MODINSTPATH):error(Please include modules.pri before crypt_xl.pri)

!isEmpty(CRYPT_XL_SOURCES) {
  for(file, CRYPT_XL_SOURCES) {
    target = $${file}.crypt
    eval($${target}.path = \$\$MODINSTPATH)
    eval($${target}.commands = $$TAOTOPSRC/crypt/crypt.sh <$$file >$$target)
    eval($${target}.files = $$target)
    eval($${target}.depends = $$file)
    eval($${target}.CONFIG = no_check_exist)
    QMAKE_EXTRA_TARGETS *= $${target}
    INSTALLS *= $${target}
    QMAKE_CLEAN *= $${target}

    SIGN_XL_SOURCES += $$target
  }
  include(sign_xl.pri)
}

