# ******************************************************************************
#  modules_doc.pri                                                  Tao project
# ******************************************************************************
# File Description:
# Common project include file to build documentation for Tao modules
# ******************************************************************************
# This software is licensed under the GNU General Public License v3
# See file COPYING for details.
# (C) 2011 Jerome Forissier <jerome@taodyne.com>
# (C) 2011 Taodyne SAS
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
