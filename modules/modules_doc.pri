# ******************************************************************************
# modules_doc.pri                                                  Tao3D project
# ******************************************************************************
#
# File description:
# Common project include file to build documentation for Tao modules
#
#
#
#
#
#
#
# ******************************************************************************
# This software is licensed under the GNU General Public License v3
# (C) 2014,2019, Christophe de Dinechin <christophe@dinechin.org>
# (C) 2011-2012, Jérôme Forissier <jerome@taodyne.com>
# ******************************************************************************
# This file is part of Tao3D
#
# Tao3D is free software: you can redistribute it and/or modify
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

include(modules_defs.pri)

!isEmpty(DOXYFILE) {

  include(../doxygen.pri)

  equals(HAS_DOXYGEN, 1) {

    DOC_DIR = $$dirname(DOXYFILE)
    DOXY = $$basename(DOXYFILE)
    isEmpty(DOXYLANG):DOXYLANG=en
    moddoc.target = doc
    isEmpty(DOXYADDFILES) {
        moddoc.commands = (cd $$DOC_DIR ; export DOXYLANG=$$DOXYLANG ; $$DOXYGEN $$DOXY)
    } else {
        # Example: if DOXYADDFILES=doc/file1 doc/file2 then QHP_ADDFILES=file1,file2
        for(file, DOXYADDFILES):BASENAMES += $$basename(file)
        QHP_ADDFILES = $$join(BASENAMES, ',')
        moddoc.commands = (cd $$DOC_DIR ; export DOXYLANG=$$DOXYLANG ; export QHP_ADDFILES=\'$$QHP_ADDFILES\' ; $$DOXYGEN $$DOXY)
    }
    moddoc.depends = cpadditionalfiles FORCE
    QMAKE_EXTRA_TARGETS += moddoc

    !isEmpty(DOXYADDFILES):cpadditionalfiles.commands = IFS=, ; langs='$$DOXYLANG' ; for f in \$\$langs ; do mkdir -p $$DOC_DIR/output/\$\$f/html/ ; cp $$DOXYADDFILES $$DOC_DIR/output/\$\$f/html/ ; done
    QMAKE_EXTRA_TARGETS += cpadditionalfiles

    docinstall.path = $${MODINSTPATH}/doc
    docinstall.commands = mkdir -p \"$$MODINSTPATH/doc\" ; cp -R $$DOC_DIR/output/* \"$$MODINSTPATH/doc\"
    docinstall.depends = moddoc
    QMAKE_EXTRA_TARGETS += docinstall
    INSTALLS += docinstall

    docclean.commands = rm -rf $$DOC_DIR/output
    distclean.depends = docclean
    QMAKE_EXTRA_TARGETS += docclean distclean

  }
}
