# ******************************************************************************
#  vg_test.pro                                                      Tao project
# ******************************************************************************
# File Description:
# Qt build file for valgrind tests for libxlr
# ******************************************************************************
# This software is property of Taodyne SAS - Confidential
# Ce logiciel est la propriété de Taodyne SAS - Confidentiel
# (C) 2011 Jerome Forissier <jerome@taodyne.com>
# (C) 2011 Taodyne SAS
# ******************************************************************************


include (../../main.pri)

TEMPLATE = app
TARGET   = vg_test

macx:CONFIG -= app_bundle

SOURCES = vg_test.cpp
INC = ../../tao/xlr/xlr/include
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
