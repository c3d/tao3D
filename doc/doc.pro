# ******************************************************************************
#  doc.pro                                                          Tao project
# ******************************************************************************
# File Description:
# Qt build file for tao documentation
# ******************************************************************************
# This software is property of Taodyne SAS - Confidential
# Ce logiciel est la propriété de Taodyne SAS - Confidentiel
# (C) 2011 Catherine Burvelle <catherine@taodyne.com>
# (C) 2011 Jerome Forissier <jerome@taodyne.com>
# (C) 2011 Taodyne SAS
# ******************************************************************************

TEMPLATE = subdirs


system(bash -c \"doxygen --version >/dev/null 2>&1\"):HAS_DOXYGEN=true
system(bash -c \"texmacs -h >/dev/null 2>&1\"):HAS_TEXMACS=true

equals(HAS_DOXYGEN, true) {

  include (../main_defs.pri)
  include (../version.pri)

  QMAKE_SUBSTITUTES = Doxyfile.in 

  include (../modules/module_list.pri)
  MOD_PATHS=$$join(MODULES, "/doc ../modules/", "../modules/", "/doc")

  doc.commands = doxygen
  doc.depends = cp_examples cp_xlref

  cp_examples.commands = mkdir -p html/examples ; \
                         cp ../tao/doc/examples/*.ddd html/examples/ ; \
                         for p in $$MOD_PATHS ; do cp -f \$\$p/*.ddd html/examples/ 2>/dev/null || : ; done

  cp_xlref.commands = cp XLRef.pdf html
  cp_xlref.depends = xlref

  xlref.target = XLRef.pdf
  equals(HAS_TEXMACS, true) {
    xlref.commands = texmacs --convert XLRef.tm XLRef.pdf --quit
    xlref.depends = XLRef.tm
  }

  clean.commands = /bin/rm -rf html/ qch/

  install.path = $$APPINST/doc
  install.commands = mkdir -p \"$$APPINST/doc\" ; cp -R html \"$$APPINST/doc/\"
  install.depends = doc

  QMAKE_EXTRA_TARGETS += doc cp_examples cp_xlref xlref clean

  INSTALLS += install

} else {

  warning(doxygen not found - will not build online documentation)

}
