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


include(main_defs.pri)

DEFINES            += DEBUG
macx:DEFINES       += CONFIG_MACOSX
win32:DEFINES      += CONFIG_MINGW
linux-g++*:DEFINES += CONFIG_LINUX

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

# No -p by default on Windows-mingw. Our make install needs -p.
win32:QMAKE_MKDIR = mkdir -p
