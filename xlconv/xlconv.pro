# ******************************************************************************
#  xlconv.pro                                                       Tao project
# ******************************************************************************
# File Description:
# 
#    Qt program description for the xlconv tool. Converts from text-based XL
#    source format to binary-encoded XL source format.
#
#
#
# ******************************************************************************
# This software is property of Taodyne SAS - Confidential
# Ce logiciel est la propriété de Taodyne SAS - Confidentiel
# (C) 2011 Jérôme Forissier <jerome@taodyne.com> 
# (C) 2011 Taodyne SAS
# ******************************************************************************

include(../main.pri)

TEMPLATE = app
TARGET   = xlconv
SOURCES += xlconv.cpp
CONFIG  += console static
CONFIG  -= app_bundle
QT += core

INC = . ../tao/xlr/xlr/include
DEPENDPATH += $$INC
INCLUDEPATH += $$INC
LIBS += -L../libxlr/\$(DESTDIR) -lxlr

# Create convenience script to run program
macx:XLCONV_CMD  = export DYLD_LIBRARY_PATH=$$PWD/../libxlr ; cd $$PWD; ./xlconv \\\"\\\$$@\\\"
linux-g++*:XLCONV_CMD = export LD_LIBRARY_PATH=$$PWD/../libxlr ; cd $$PWD; ./xlconv \\\"\\\$$@\\\"
win32 {
  HERE = $$system(bash -c 'pwd | sed \'s@\\([a-zA-Z]\\):@/\\1@\'')
  XLCONV_CMD = export PATH=\\\"\\\$$PATH:$$HERE/../libxlr/\\\"\$(DESTDIR); cd \\\"$$HERE/\\\"; \$(DESTDIR_TARGET) \\\"\\\$$@\\\"
}
QMAKE_CLEAN += xlconv.sh
# XLR FIXME: xl.syntax and xl.stylesheet MUST be present in the program's
# current directory. Hence the copies (below) and the "cd" in XLCONV_CMD.
# NOTE: ../tao/xl.stylesheet inserts unwanted spaces:
# e.g., "translatez - 100" instead of "translatez -100"
# git.stylesheet is OK.
QMAKE_POST_LINK = cp ../tao/xl.syntax . ; cp ../tao/git.stylesheet xl.stylesheet ; echo \"$$XLCONV_CMD\" > xlconv.sh && chmod +x xlconv.sh

QMAKE_CLEAN += xl.syntax xl.stylesheet

# Kludge so that "make install" builds the binary
win32:DEP = \$(DESTDIR_TARGET)
!win32:DEP = $$TARGET
install.commands = :
install.path = .
install.depends = $$DEP
INSTALLS += install
