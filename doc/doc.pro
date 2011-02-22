# ******************************************************************************
#  doc.pro                                                          Tao project
# ******************************************************************************
# File Description:
# Qt build file for tao documentation
# ******************************************************************************
# This document is released under the GNU General Public License.
# See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
# (C) 2011 Catherine Burvelle <catherine@taodyne.com>
# (C) 2011 Taodyne SAS
# ******************************************************************************

include (../main.pri)

TEMPLATE = subdirs
macx {

  DOCDIR = doc
  TAODIR = $$DOCDIR/xl
  HTMLDIR = $$DOCDIR/html
  DOXYGEN = doxygen/doxygen
  ABSOLU_TAODIR = $$PWD/$$TAODIR
  #message($$ABSOLU_TAODIR)

  QMAKE_SUBSTITUTES = genDoc.ddd.in

  include (../modules/module_list.pri)
  contains(MODULES, object_loader):IMPORTS += "import ObjectLoader \"0.2\""
  contains(MODULES, lorem_ipsum):IMPORTS +="import LoremIpsum \"0.1\""
  contains(MODULES, tao_visuals):IMPORTS +="import TaoVisuals \"0.1\""
  contains(MODULES, digital_clock):IMPORTS +="import DigitalClock \"0.1\""
  IMPORTS=$$join(IMPORTS, $$escape_expand(\\n))

  doc_xl.commands = (cd $$APPINST; DYLD_LIBRARY_PATH=../Frameworks ./Tao -norepo -nosplash $$PWD/genDoc.ddd)
  doc_xl.depends = $$APPINST/Tao create_dirs genDoc.ddd

  doc_tao.commands = awk -v TAODOC=$$TAODIR/tao_doc.ddd -f tools/listFamilies.awk $$TAODIR/all_documentation.xl ; cp -R src $$DOCDIR ; cp  tools/theme.xl $$TAODIR
  doc_tao.depends = doc_xl

  doc_html.commands = $$DOXYGEN tools/tao_doc.cfg && cp src/* doc/html
  doc_html.depends = doc_tao $$DOXYGEN tools/tao_doc.cfg

  create_dirs.commands = mkdir -p c_files;  mkdir -p $$TAODIR

  doc.depends = doc_html

  clean.commands = /bin/rm -rf c_files/ doc/

  QMAKE_EXTRA_TARGETS += doc_xl doc_tao doc_html doc doxy create_dirs clean
}

!macx {
  message("Documentation is only build on MacOSX systems.
}
