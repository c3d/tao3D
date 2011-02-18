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

DOCDIR = doc
TAODIR = $$DOCDIR/xl
HTMLDIR = $$DOCDIR/html
DOXYGEN = doxygen/doxygen

doc_xl.commands = (cd $$APPINST; echo \"generate_all_doc \\\"$$TAODIR/all_documentation.xl\\\"; exit 0 \" > genDoc.ddd; DYLD_LIBRARY_PATH=../Frameworks ./Tao -norepo -nosplash genDoc.ddd; /bin/rm genDoc.ddd)
doc_xl.depends = $$APPINST/Tao create_dirs

doc_tao.commands = awk -v TAODOC=$$TAODIR/tao_doc.ddd -f tools/listFamilies.awk $$TAODIR/all_documentation.xl ; cp -R src $$DOCDIR ; cp  tools/theme.xl $$TAODIR
doc_tao.depends = doc_xl

doc_html.commands = $$DOXYGEN tools/tao_doc.cfg
doc_html.depends = doc_tao $$DOXYGEN tools/tao_doc.cfg

create_dirs.commands = mkdir -p c_files;  mkdir -p $$TAODIR

doc.depends = doc_html

clean.commands = /bin/rm -r c_files/ doc/; (cd doxygen-1.7.1 && make clean)

QMAKE_EXTRA_TARGETS += doc_xl doc_tao doc_html doc doxy create_dirs clean
