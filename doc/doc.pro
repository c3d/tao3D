# ******************************************************************************
#  doc.pro                                                       Tao project
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

DOCDIR = $$INSTROOT/doc
TAODIR = $$DOCDIR/tao_doc
HTMLDIR = $$DOCDIR/html

doc_xl.commands = mkdir -p $$TAODIR; (cd $$APPINST; echo \"generate_all_doc \\\"$$TAODIR/all_documentation.xl\\\"; exit 0 \" > genDoc.ddd; DYLD_LIBRARY_PATH=../Frameworks ./Tao -norepo -nosplash genDoc.ddd)
doc_xl.depends = $$APPINST/Tao 

doc_tao.commands = awk -v TAODOC=$$TAODIR/tao_doc.ddd -f tools/listFamilies.awk $$TAODIR/all_documentation.xl
doc_tao.depends = doc_xl

tao_theme.path = $$TAODIR
tao_theme.files = tools/theme.xl

#doc_html.commands = 
QMAKE_EXTRA_TARGETS += doc_xl doc_tao

INSTALLS += tao_theme
