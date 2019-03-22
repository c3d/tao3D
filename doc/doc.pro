# ******************************************************************************
# doc.pro                                                          Tao3D project
# ******************************************************************************
#
# File description:
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
# This software is licensed under the GNU General Public License v3
# (C) 2011, Catherine Burvelle <catherine@taodyne.com>
# (C) 2014-2015,2019, Christophe de Dinechin <christophe@dinechin.org>
# (C) 2011-2013, Jérôme Forissier <jerome@taodyne.com>
# ******************************************************************************
# This file is part of Tao3D
#
# Tao3D is free software: you can r redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Tao3D is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Tao3D, in a file named COPYING.
# If not, see <https://www.gnu.org/licenses/>.
# ******************************************************************************

TEMPLATE = subdirs


include(../doxygen.pri)
system(bash -c \"texmacs -h >/dev/null 2>&1\"):HAS_TEXMACS=true

equals(HAS_DOXYGEN, 1) {

  include (../main_defs.pri)
  include (../version.pri)
  include (../gitrev.pri)

  PROJECT_NUMBER = $$system(bash -c \"../app/updaterev.sh -n\")
  macx:TAO_ICON_FOR_QHCP = ../app/tao.icns
  win32:TAO_ICON_FOR_QHCP = ../app/tao.ico
  !macx:!win32:TAO_ICON_FOR_QHCP = ../app/tao.png
  QMAKE_SUBSTITUTES = Doxyfile.in \
                      DoxyfileWebdoc.in \
                      Tao3D.qhcp.in
  QMAKE_DISTCLEAN += $$replace(QMAKE_SUBSTITUTES, .in, )

  include (../modules/module_list.pri)
  MOD_PATHS=$$join(MODULES, "/doc ../modules/", "../modules/", "/doc")

  DOXYLANG=en,fr
  doc.commands = EXAMPLES=`cd ../app/doc; echo examples/*.ddd` ; export DOXYLANG=$$DOXYLANG ; export QHP_ADDFILES=\"Taodyne_logo.png \$\$EXAMPLES\"; $$DOXYGEN
  doc.depends = cp_examples cp_logo version xlref

  webdoc.commands = export DOXYLANG=$$DOXYLANG ; export DOXYOUTPUT=webhtml ; $$DOXYGEN DoxyfileWebdoc
  webdoc.depends = cp_examples_webdoc cp_logo_webdoc version xlref

  LANGUAGES=$$replace(DOXYLANG, ',', ' ')
  cp_examples.commands = for l in $$LANGUAGES ; do \
                           mkdir -p output/\$\$l/html/examples ; \
                           cp ../app/doc/examples/*.ddd output/\$\$l/html/examples/ ; \
                         done

  cp_examples_webdoc.commands = for l in $$LANGUAGES ; do \
                                  mkdir -p webhtml/\$\$l/html/examples ; \
                                  cp ../app/doc/examples/*.ddd webhtml/\$\$l/html/examples/ ; \
                                  for p in $$MOD_PATHS ; do cp -f \$\$p/*.ddd webhtml/\$\$l/html/ 2>/dev/null || : ; done ; \
                                done

  cp_logo.commands = for l in $$LANGUAGES ; do mkdir -p output/\$\$l/html ; cp images/Taodyne_logo.png output/\$\$l/html ; done

  cp_logo_webdoc.commands = for l in $$LANGUAGES ; do mkdir -p webhtml/\$\$l/html ; cp images/Taodyne_logo.png webhtml/\$\$l/html ; done

  xlref.target = XLRef.html
  equals(HAS_TEXMACS, true) {
    xlref.commands = texmacs --convert ../app/xlr/xlr/doc/XLRef.tm XLRef.tmp.html --quit && sed -f texmacs_html.sed XLRef.tmp.html > XLRef.html && rm XLRef.tmp.html
    xlref.depends = ../app/xlr/xlr/doc/XLRef.tm
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

  qhc.commands = qcollectiongenerator Tao3D.qhcp -o Tao3D.qhc
  qhc.depends = doc
  QMAKE_EXTRA_TARGETS += qhc
  QMAKE_CLEAN += Tao3D.qhc
  QMAKE_DISTCLEAN += Tao3D.qhc

  install_qhc.path = $$APPINST/doc
  install_qhc.commands = mkdir -p \"$$APPINST/doc\" ; cp Tao3D.qhc \"$$APPINST/doc/\"
  install_qhc.depends = qhc
  QMAKE_EXTRA_TARGETS += install_qhc
  INSTALLS += install_qhc

  rmoutput.commands = rm -rf output
  distclean.depends = rmoutput
  QMAKE_EXTRA_TARGETS += rmoutput distclean

}

OTHER_FILES += XLRef.pdf \
    ../app/doc/camera.doxy.h \
    ../app/doc/graph.doxy.h \
    ../app/doc/page.doxy.h \
    ../app/doc/tao.doxy.h \
    ../app/doc/widgets.doxy.h \
    ../app/doc/chooser.doxy.h \
    ../app/doc/lighting.doxy.h \
    ../app/doc/shaders.doxy.h  \
    ../app/doc/tao_application.doxy.h \
    ../app/doc/code.doxy.h \
    ../app/doc/menus.doxy.h \
    ../app/doc/shape_handling.doxy.h \
    ../app/doc/text.doxy.h \
    ../app/doc/files.doxy.h \
    ../app/doc/modules.doxy.h \
    ../app/doc/stereoscopy.doxy.h \
    ../app/doc/time.doxy.h \
    ../app/doc/frame.doxy.h \
    ../app/doc/mouse.doxy.h \
    ../app/doc/tablelayout.doxy.h \
    ../app/doc/transform.doxy.h
