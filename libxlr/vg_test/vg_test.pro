# ******************************************************************************
# vg_test.pro                                                      Tao3D project
# ******************************************************************************
#
# File description:
# Qt build file for valgrind tests for libxlr
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
# (C) 2011, Jérôme Forissier <jerome@taodyne.com>
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


include (../../main.pri)

TEMPLATE = app
TARGET   = vg_test

macx:CONFIG -= app_bundle

SOURCES = vg_test.cpp
INC = ../../libxlr/xlr/xlr/include
DEPENDPATH += $$INC
INCLUDEPATH += $$INC

LIBS += -L.. -lxlr

macx {
  ENV = export DYLD_LIBRARY_PATH=..
  VALGRIND = valgrind --dsymutil=yes
}
VG = $$ENV ; $$VALGRIND \$\$VGOPT ./$$TARGET

ALL_TESTS = noerr_t ir_t leak_t perf_t

ir_t.commands = $$VG -ir_test
ir_t.depends = $$TARGET

noerr_t.commands = VGOPT=\"--leak-check=full\" ; $$VG -noerr_test
noerr_t.depends = $$TARGET

leak_t.commands = VGOPT=\"--leak-check=full\" ; $$VG -leak_test
leak_t.depends = $$TARGET

perf_t.commands = $$ENV ; ./$$TARGET -perf_test
perf_t.depends = $$TARGET

test.depends = $$ALL_TESTS
QMAKE_EXTRA_TARGETS += test $$ALL_TESTS

macx {
  clean.depends = clean_dsym
  clean_dsym.commands = rm -rf ./$${TARGET}.dSYM ../libxlr.*.dylib.dSYM
  QMAKE_EXTRA_TARGETS += clean clean_dsym
}
