# ******************************************************************************
#  main_defs.pri                                                    Tao project
# ******************************************************************************
# File Description:
# Global project definitions for Tao (Tao-specific variables only)
# ******************************************************************************
# This software is property of Taodyne SAS - Confidential
# Ce logiciel est la propriété de Taodyne SAS - Confidentiel
# (C) 2010 Jerome Forissier <jerome@taodyne.com>
# (C) 2010 Taodyne SAS
# ******************************************************************************


INSTROOT = $$PWD/install
macx {
APPINST = "$$INSTROOT/Tao Presentations.app/Contents/MacOS"
LIBINST = "$$INSTROOT/Tao Presentations.app/Contents/Frameworks"
} else {
APPINST = $$INSTROOT
LIBINST = $$INSTROOT
}
