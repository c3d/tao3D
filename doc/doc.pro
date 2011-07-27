# ******************************************************************************
#  doc.pro                                                          Tao project
# ******************************************************************************
# File Description:
# Qt build file for tao documentation
#
# make doc
#    Build the client-side documentation
# make webdoc
#    Build the server-side documentation for upload to taodyne.com
#    (same as plain HTML doc but with PHP search enabled).
#    Output is in ./webhtml
# make install
#    Build and install client-side documention
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
  doc.depends = cp_examples cp_xlref version

  webdoc.commands = ( cat Doxyfile ; echo \"SERVER_BASED_SEARCH = YES\" ; echo \"HTML_OUTPUT = webhtml\" ) | doxygen -
  webdoc.depends = cp_examples cp_xlref version

  cp_examples.commands = mkdir -p html/examples ; \
                         cp ../tao/doc/examples/*.ddd html/examples/ ; \
                         for p in $$MOD_PATHS ; do cp -f \$\$p/*.ddd html/examples/ 2>/dev/null || : ; done

  cp_xlref.commands = mkdir -p html ; cp XLRef.pdf html
  cp_xlref.depends = xlref

  xlref.target = XLRef.pdf
  equals(HAS_TEXMACS, true) {
    xlref.commands = texmacs --convert XLRef.tm XLRef.pdf --quit
    xlref.depends = XLRef.tm
  }

  clean.commands = /bin/rm -rf html/ webhtml/ qch/

  install.path = $$APPINST/doc
  install.commands = mkdir -p \"$$APPINST/doc\" ; cp -R html \"$$APPINST/doc/\"
  install.depends = doc

  version.target = project_number.doxinclude
  version.commands = V=`git describe --tags --always --dirty=-dirty`; echo "PROJECT_NUMBER=\$\$V" >project_number.doxinclude
  version.depends = FORCE

  QMAKE_EXTRA_TARGETS += doc webdoc cp_examples cp_xlref xlref clean version

  INSTALLS += install

} else {

  warning(doxygen not found - will not build online documentation)

}

OTHER_FILES += XLRef.pdf \
    ../tao/doc/camera.doxy.h \
    ../tao/doc/graph.doxy.h \
    ../tao/doc/page.doxy.h \
    ../tao/doc/tao.doxy.h \
    ../tao/doc/widgets.doxy.h \
    ../tao/doc/chooser.doxy.h \
    ../tao/doc/lighting.doxy.h \
    ../tao/doc/shaders.doxy.h  \
    ../tao/doc/tao_application.doxy.h \
    ../tao/doc/code.doxy.h \
    ../tao/doc/menus.doxy.h \
    ../tao/doc/shape_handling.doxy.h \
    ../tao/doc/text.doxy.h \
    ../tao/doc/files.doxy.h \
    ../tao/doc/modules.doxy.h \
    ../tao/doc/stereoscopy.doxy.h \
    ../tao/doc/time.doxy.h \
    ../tao/doc/frame.doxy.h \
    ../tao/doc/mouse.doxy.h \
    ../tao/doc/tablelayout.doxy.h \
    ../tao/doc/transform.doxy.h

