# ******************************************************************************
# libcryptopp.pro                                                  Tao3D project
# ******************************************************************************
#
# File description:
# Qt build file for the Crypto++ library (http://www.cryptopp.com/)
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
# (C) 2011-2013, Jérôme Forissier <jerome@taodyne.com>
# ******************************************************************************
# This file is part of Tao3D
#
# Tao3D is free software: you can r redistribute it and/or modify
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
    $${SRC}/base64.cpp \
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
    $${SRC}/gcm.cpp \
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
    $${SRC}/rsa.cpp \
    $${SRC}/serpent.cpp \
    $${SRC}/sha.cpp \
    $${SRC}/tftables.cpp \
    $${SRC}/wait.cpp \
    $${SRC}/xtr.cpp \
    $${SRC}/zdeflate.cpp \
    $${SRC}/zinflate.cpp

# Kludge so that "make install" builds the binary
install.commands = :
install.path = .
install.depends = $$TARGET
INSTALLS += install
