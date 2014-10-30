# Signature of XL source files with a Taodyne key
#
# Create a signature file (.xl.sig) from XL source files (.xl).
# Resulting files are automatically added to INSTALLS.
#
# Usage: In module_name/module_name.pro:
#   SIGN_XL_SOURCES += filename.xl
#   include(sign_xl.pri)

isEmpty(SIGN_XL_INSTPATH):SIGN_XL_INSTPATH=$$MODINSTPATH
isEmpty(SIGN_XL_INSTPATH):error(Please define SIGN_XL_INSTPATH or include modules.pri before sign_xl.pri)

!isEmpty(SIGN_XL_SOURCES):isEmpty(NO_DOC_SIGNATURE) {
  for(file, SIGN_XL_SOURCES) {
    target = $${file}.sig
    eval($${target}.path = \$\$SIGN_XL_INSTPATH)
    exists(../tao_sign/tao_sign.sh) {
      eval($${target}.commands = $$TAOTOPSRC/tao_sign/tao_sign.sh -r $$file)
    }
    eval($${target}.files = $$target)
    eval($${target}.depends = $$file)
    eval($${target}.CONFIG = no_check_exist)
    QMAKE_EXTRA_TARGETS *= $${target}
    INSTALLS *= $${target}
    QMAKE_CLEAN *= $${target}
  }
}
