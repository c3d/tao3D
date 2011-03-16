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

equals(HAS_DOXYGEN, true) {

  include (../main_defs.pri)
  include (../version.pri)

  QMAKE_SUBSTITUTES = Doxyfile.in 

  include (../modules/module_list.pri)
  MOD_PATHS=$$join(MODULES, "/doc ../modules/", "../modules/", "/doc")

  dox.target = doc
  dox.commands = doxygen 
  dox.depends = cp_examples

  cp_examples.target = examples
  cp_examples.commands = mkdir -p html/examples ; \
                         cp ../tao/doc/examples/*.ddd html/examples/ ; \
                         for p in $$MOD_PATHS ; do cp -f \$\$p/*.ddd html/examples/ 2>/dev/null || : ; done

  clean.commands = /bin/rm -rf html/ qch/

  install.target = install
  install.commands = mkdir -p \"$$APPINST/doc\" ; cp -R html \"$$APPINST/doc/\"
  install.depends = dox

  QMAKE_EXTRA_TARGETS += dox cp_examples install clean

} else {

  warning(doxygen not found - will not build online documentation)

}
