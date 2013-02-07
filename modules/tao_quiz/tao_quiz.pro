
MODINSTDIR = tao_quiz

include(../modules.pri)

OTHER_FILES = tao_quiz.xl
OTHER_FILES += doc/tao_quiz.doxy.h doc/Doxyfile.in
OTHER_FILES += tao_quiz2.xl

CRYPT_XL_SOURCES = tao_quiz2.xl
include(../crypt_xl.pri)

INSTALLS    += thismod_icon
INSTALLS    -= thismod_bin


QMAKE_SUBSTITUTES = doc/Doxyfile.in
QMAKE_DISTCLEAN = doc/Doxyfile
DOXYFILE = doc/Doxyfile
DOXYLANG = en,fr
include(../modules_doc.pri)
