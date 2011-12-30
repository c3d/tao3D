# ******************************************************************************
#  modules_doc.pri                                                  Tao project
# ******************************************************************************
# File Description:
# Common project include file to build documentation for Tao modules
# ******************************************************************************
# This software is property of Taodyne SAS - Confidential
# Ce logiciel est la propriété de Taodyne SAS - Confidentiel
# (C) 2011 Jerome Forissier <jerome@taodyne.com>
# (C) 2011 Taodyne SAS
# ******************************************************************************

include(modules_defs.pri)

!isEmpty(DOXYFILE) {

  include(../doxygen.pri)

  equals(HAS_DOXYGEN, 1) {

    DOC_DIR = $$dirname(DOXYFILE)
    DOXY = $$basename(DOXYFILE)
    moddoc.target = doc
    moddoc.commands = (cd $$DOC_DIR ; $$DOXYGEN $$DOXY)
    moddoc.depends = FORCE
    QMAKE_EXTRA_TARGETS += moddoc

    docinstall.path = $${MODINSTPATH}/doc
    docinstall.commands = mkdir -p \"$$MODINSTPATH/doc\" ; cp -R $$DOC_DIR/output/* \"$$MODINSTPATH/doc\"
    docinstall.depends = moddoc
    QMAKE_EXTRA_TARGETS += docinstall
    INSTALLS += docinstall

    docclean.commands = rm -rf $$DOC_DIR/output
    distclean.depends = docclean
    QMAKE_EXTRA_TARGETS += docclean distclean

  } else {

    warning(doxygen not found - will not build online documentation)

  }
}
