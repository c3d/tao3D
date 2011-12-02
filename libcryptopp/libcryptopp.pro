# ******************************************************************************
#  libcryptopp.pro                                                  Tao project
# ******************************************************************************
# File Description:
# Qt build file for the Crypto++ library (http://www.cryptopp.com/)
# ******************************************************************************
# This software is property of Taodyne SAS - Confidential
# Ce logiciel est la propriété de Taodyne SAS - Confidentiel
# (C) 2011 Jerome Forissier <jerome@taodyne.com>
# (C) 2011 Taodyne SAS
# ******************************************************************************thornin


include (../main.pri)

TEMPLATE = lib
TARGET   = cryptopp
CONFIG  += static
QT      -= core gui

CONFIG += warn_off
QMAKE_CXXFLAGS -= -Werror

CONFIG(release, debug|release):DEFINES += NDEBUG
macx:DEFINES += CRYPTOPP_DISABLE_ASM

# Source directory (content of ZIP file downloaded from http://www.cryptopp.com/)
SRC = cryptopp

INCLUDEPATH = $${SRC}
DEPENDPATH += $${SRC}

SOURCES = \
    $${SRC}/adler32.cpp \
    $${SRC}/algparam.cpp \
    $${SRC}/asn.cpp \
    $${SRC}/authenc.cpp \
    $${SRC}/basecode.cpp \
    $${SRC}/bfinit.cpp \
    $${SRC}/cpu.cpp \
    $${SRC}/crc.cpp \
    $${SRC}/cryptlib.cpp \
    $${SRC}/des.cpp \
    $${SRC}/dessp.cpp \
    $${SRC}/dll.cpp \
    $${SRC}/dsa.cpp \
    $${SRC}/ec2n.cpp \
    $${SRC}/ecp.cpp \
    $${SRC}/filters.cpp \
    $${SRC}/fips140.cpp \
    $${SRC}/gf256.cpp \
    $${SRC}/gf2_32.cpp \
    $${SRC}/gf2n.cpp \
    $${SRC}/gfpcrypt.cpp \
    $${SRC}/hex.cpp \
    $${SRC}/hmac.cpp \
    $${SRC}/hrtimer.cpp \
    $${SRC}/integer.cpp \
    $${SRC}/iterhash.cpp \
    $${SRC}/misc.cpp \
    $${SRC}/modes.cpp \
    $${SRC}/mqueue.cpp \
    $${SRC}/nbtheory.cpp \
    $${SRC}/network.cpp \
    $${SRC}/oaep.cpp \
    $${SRC}/osrng.cpp \
    $${SRC}/pkcspad.cpp \
    $${SRC}/pssr.cpp \
    $${SRC}/pubkey.cpp \
    $${SRC}/queue.cpp \
    $${SRC}/randpool.cpp \
    $${SRC}/rdtables.cpp \
    $${SRC}/rijndael.cpp \
    $${SRC}/rng.cpp \
    $${SRC}/serpent.cpp \
    $${SRC}/sha.cpp \
    $${SRC}/tftables.cpp \
    $${SRC}/wait.cpp \
    $${SRC}/xtr.cpp \
    $${SRC}/zdeflate.cpp \
    $${SRC}/zinflate.cpp \
    $${SRC}/rsa.cpp \
    $${SRC}/gcm.cpp

# Kludge so that "make install" builds the binary
install.commands = :
install.path = .
install.depends = $$TARGET
INSTALLS += install
