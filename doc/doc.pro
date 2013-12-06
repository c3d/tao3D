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


include(../doxygen.pri)
system(bash -c \"texmacs -h >/dev/null 2>&1\"):HAS_TEXMACS=true

equals(HAS_DOXYGEN, 1) {

  include (../main_defs.pri)
  include (../version.pri)
  include (../gitrev.pri)

  PROJECT_NUMBER = $$system(bash -c \"../tao/updaterev.sh -n\")
  macx:TAO_ICON_FOR_QHCP = ../tao/tao.icns
  win32:TAO_ICON_FOR_QHCP = ../tao/tao.ico
  !macx:!win32:TAO_ICON_FOR_QHCP = ../tao/tao.png
  QMAKE_SUBSTITUTES = Doxyfile.in \
                      DoxyfileWebdoc.in \
                      TaoPresentations.qhcp.in
  QMAKE_DISTCLEAN += $$replace(QMAKE_SUBSTITUTES, .in, )

  include (../modules/module_list.pri)
  MOD_PATHS=$$join(MODULES, "/doc ../modules/", "../modules/", "/doc")

  DOXYLANG=en,fr
  doc.commands = EXAMPLES=`cd ../tao/doc; echo examples/*.ddd` ; export DOXYLANG=$$DOXYLANG ; export QHP_ADDFILES=\"Taodyne_logo.png \$\$EXAMPLES\"; $$DOXYGEN
  doc.depends = cp_examples cp_logo version xlref

  webdoc.commands = export DOXYLANG=$$DOXYLANG ; export DOXYOUTPUT=webhtml ; $$DOXYGEN DoxyfileWebdoc
  webdoc.depends = cp_examples_webdoc cp_logo_webdoc version xlref

  LANGUAGES=$$replace(DOXYLANG, ',', ' ')
  cp_examples.commands = for l in $$LANGUAGES ; do \
                           mkdir -p output/\$\$l/html/examples ; \
                           cp ../tao/doc/examples/*.ddd output/\$\$l/html/examples/ ; \
                         done

  cp_examples_webdoc.commands = for l in $$LANGUAGES ; do \
                                  mkdir -p webhtml/\$\$l/html/examples ; \
                                  cp ../tao/doc/examples/*.ddd webhtml/\$\$l/html/examples/ ; \
                                  for p in $$MOD_PATHS ; do cp -f \$\$p/*.ddd webhtml/\$\$l/html/ 2>/dev/null || : ; done ; \
                                done

  cp_logo.commands = for l in $$LANGUAGES ; do mkdir -p output/\$\$l/html ; cp images/Taodyne_logo.png output/\$\$l/html ; done

  cp_logo_webdoc.commands = for l in $$LANGUAGES ; do mkdir -p webhtml/\$\$l/html ; cp images/Taodyne_logo.png webhtml/\$\$l/html ; done

  xlref.target = XLRef.html
  equals(HAS_TEXMACS, true) {
    xlref.commands = texmacs --convert ../tao/xlr/xlr/doc/XLRef.tm XLRef.tmp.html --quit && sed -f texmacs_html.sed XLRef.tmp.html > XLRef.html && rm XLRef.tmp.html
    xlref.depends = ../tao/xlr/xlr/doc/XLRef.tm
  }

  cleandocdirs.commands = /bin/rm -rf html/ webhtml/ qch/
  QMAKE_EXTRA_TARGETS += cleandocdirs
  clean.depends = cleandocdirs

  install_doc.path = $$APPINST/doc
  install_doc.commands = mkdir -p \"$$APPINST/doc\" ; cp -R output/* \"$$APPINST/doc/\"
  install_doc.depends = doc

  version.target = project_number.doxinclude
  version.commands = V=$$PROJECT_NUMBER; echo "PROJECT_NUMBER=\$\$V" >project_number.doxinclude
  version.depends = FORCE
  QMAKE_CLEAN += project_number.doxinclude
  QMAKE_DISTCLEAN += project_number.doxinclude

  QMAKE_EXTRA_TARGETS += doc webdoc cp_examples cp_examples_webdoc cp_xlref cp_logo cp_logo_webdoc xlref clean version install_html

  INSTALLS += install_doc

  qhc.commands = qcollectiongenerator TaoPresentations.qhcp -o TaoPresentations.qhc
  qhc.depends = doc
  QMAKE_EXTRA_TARGETS += qhc
  QMAKE_CLEAN += TaoPresentations.qhc
  QMAKE_DISTCLEAN += TaoPresentations.qhc

  install_qhc.path = $$APPINST/doc
  install_qhc.commands = mkdir -p \"$$APPINST/doc\" ; cp TaoPresentations.qhc \"$$APPINST/doc/\"
  install_qhc.depends = qhc
  QMAKE_EXTRA_TARGETS += install_qhc
  INSTALLS += install_qhc

  rmoutput.commands = rm -rf output
  distclean.depends = rmoutput
  QMAKE_EXTRA_TARGETS += rmoutput distclean

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

