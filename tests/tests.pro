TEMPLATE = subdirs


#tests.target = results.ddd
tests.commands = ./runAllTest.sh

QMAKE_EXTRA_TARGETS += tests

OTHER_FILES += \
    runAllTest.sh \
    result_style.xl
