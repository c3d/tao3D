# ******************************************************************************
#  main.pri                                                         Tao project
# ******************************************************************************
# File Description:
# Global project definitions for Tao
# ******************************************************************************
# This document is released under the GNU General Public License.
# See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
# (C) 2010 Jerome Forissier <jerome@taodyne.com>
# (C) 2010 Taodyne SAS
# ******************************************************************************


INSTROOT = $$PWD/install
macx {
APPINST = $$INSTROOT/Tao.app/Contents/MacOS
LIBINST = $$INSTROOT/Tao.app/Contents/Frameworks
} else {
APPINST = $$INSTROOT
LIBINST = $$INSTROOT
}

QMAKE_DISTCLEAN += $$INSTROOT  # FIXME: need recursive delete

DEFINES           += DEBUG
macx:DEFINES      += CONFIG_MACOSX
win32:DEFINES     += CONFIG_MINGW
linux-g++:DEFINES += CONFIG_LINUX

QMAKE_CFLAGS           += -Werror
QMAKE_CXXFLAGS         += -Werror -ggdb
QMAKE_CXXFLAGS_RELEASE += -g \$(CXXFLAGS_\$%)

# Adding 'c++tbl' option with lowered optimization level
c++tbl.output = ${QMAKE_FILE_BASE}.o
c++tbl.commands = $(CXX) \
    -c \
    $(CXXFLAGS:-O2=) \
    $(INCPATH) \
    ${QMAKE_FILE_NAME} \
    -o \
    ${QMAKE_FILE_OUT}
c++tbl.dependency_type = TYPE_C
c++tbl.input = CXXTBL_SOURCES
QMAKE_EXTRA_COMPILERS += c++tbl
