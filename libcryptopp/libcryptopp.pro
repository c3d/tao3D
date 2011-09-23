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

HEADERS = \
    $${SRC}/3way.h \
    $${SRC}/adler32.h \
    $${SRC}/aes.h \
    $${SRC}/algebra.h \
    $${SRC}/algparam.h \
    $${SRC}/arc4.h \
    $${SRC}/argnames.h \
    $${SRC}/asn.h \
    $${SRC}/authenc.h \
    $${SRC}/base32.h \
    $${SRC}/base64.h \
    $${SRC}/basecode.h \
    $${SRC}/bench.h \
    $${SRC}/blowfish.h \
    $${SRC}/blumshub.h \
    $${SRC}/camellia.h \
    $${SRC}/cast.h \
    $${SRC}/cbcmac.h \
    $${SRC}/ccm.h \
    $${SRC}/channels.h \
    $${SRC}/cmac.h \
    $${SRC}/config.h \
    $${SRC}/cpu.h \
    $${SRC}/crc.h \
    $${SRC}/cryptlib.h \
    $${SRC}/default.h \
    $${SRC}/des.h \
    $${SRC}/dh.h \
    $${SRC}/dh2.h \
    $${SRC}/dll.h \
    $${SRC}/dmac.h \
    $${SRC}/dsa.h \
    $${SRC}/eax.h \
    $${SRC}/ec2n.h \
    $${SRC}/eccrypto.h \
    $${SRC}/ecp.h \
    $${SRC}/elgamal.h \
    $${SRC}/emsa2.h \
    $${SRC}/eprecomp.h \
    $${SRC}/esign.h \
    $${SRC}/factory.h \
    $${SRC}/files.h \
    $${SRC}/filters.h \
    $${SRC}/fips140.h \
    $${SRC}/fltrimpl.h \
    $${SRC}/gcm.h \
    $${SRC}/gf256.h \
    $${SRC}/gf2_32.h \
    $${SRC}/gf2n.h \
    $${SRC}/gfpcrypt.h \
    $${SRC}/gost.h \
    $${SRC}/gzip.h \
    $${SRC}/hex.h \
    $${SRC}/hmac.h \
    $${SRC}/hrtimer.h \
    $${SRC}/ida.h \
    $${SRC}/idea.h \
    $${SRC}/integer.h \
    $${SRC}/iterhash.h \
    $${SRC}/lubyrack.h \
    $${SRC}/luc.h \
    $${SRC}/mars.h \
    $${SRC}/md2.h \
    $${SRC}/md4.h \
    $${SRC}/md5.h \
    $${SRC}/mdc.h \
    $${SRC}/misc.h \
    $${SRC}/modarith.h \
    $${SRC}/modes.h \
    $${SRC}/modexppc.h \
    $${SRC}/mqueue.h \
    $${SRC}/mqv.h \
    $${SRC}/nbtheory.h \
    $${SRC}/network.h \
    $${SRC}/nr.h \
    $${SRC}/oaep.h \
    $${SRC}/oids.h \
    $${SRC}/osrng.h \
    $${SRC}/panama.h \
    $${SRC}/pch.h \
    $${SRC}/pkcspad.h \
    $${SRC}/polynomi.h \
    $${SRC}/pssr.h \
    $${SRC}/pubkey.h \
    $${SRC}/pwdbased.h \
    $${SRC}/queue.h \
    $${SRC}/rabin.h \
    $${SRC}/randpool.h \
    $${SRC}/rc2.h \
    $${SRC}/rc5.h \
    $${SRC}/rc6.h \
    $${SRC}/resource.h \
    $${SRC}/rijndael.h \
    $${SRC}/ripemd.h \
    $${SRC}/rng.h \
    $${SRC}/rsa.h \
    $${SRC}/rw.h \
    $${SRC}/safer.h \
    $${SRC}/salsa.h \
    $${SRC}/seal.h \
    $${SRC}/secblock.h \
    $${SRC}/seckey.h \
    $${SRC}/seed.h \
    $${SRC}/serpent.h \
    $${SRC}/serpentp.h \
    $${SRC}/sha.h \
    $${SRC}/shacal2.h \
    $${SRC}/shark.h \
    $${SRC}/simple.h \
    $${SRC}/skipjack.h \
    $${SRC}/smartptr.h \
    $${SRC}/socketft.h \
    $${SRC}/sosemanuk.h \
    $${SRC}/square.h \
    $${SRC}/stdcpp.h \
    $${SRC}/strciphr.h \
    $${SRC}/tea.h \
    $${SRC}/tiger.h \
    $${SRC}/trdlocal.h \
    $${SRC}/trunhash.h \
    $${SRC}/ttmac.h \
    $${SRC}/twofish.h \
    $${SRC}/validate.h \
    $${SRC}/vmac.h \
    $${SRC}/wait.h \
    $${SRC}/wake.h \
    $${SRC}/whrlpool.h \
    $${SRC}/winpipes.h \
    $${SRC}/words.h \
    $${SRC}/xtr.h \
    $${SRC}/xtrcrypt.h \
    $${SRC}/zdeflate.h \
    $${SRC}/zinflate.h \
    $${SRC}/zlib.h

SOURCES = \
    $${SRC}/3way.cpp \
    $${SRC}/adler32.cpp \
    $${SRC}/algebra.cpp \
    $${SRC}/algparam.cpp \
    $${SRC}/arc4.cpp \
    $${SRC}/asn.cpp \
    $${SRC}/authenc.cpp \
    $${SRC}/base32.cpp \
    $${SRC}/base64.cpp \
    $${SRC}/basecode.cpp \
    $${SRC}/bfinit.cpp \
    $${SRC}/blowfish.cpp \
    $${SRC}/blumshub.cpp \
    $${SRC}/camellia.cpp \
    $${SRC}/cast.cpp \
    $${SRC}/casts.cpp \
    $${SRC}/cbcmac.cpp \
    $${SRC}/ccm.cpp \
    $${SRC}/channels.cpp \
    $${SRC}/cmac.cpp \
    $${SRC}/cpu.cpp \
    $${SRC}/crc.cpp \
    $${SRC}/cryptlib.cpp \
    $${SRC}/cryptlib_bds.cpp \
    $${SRC}/default.cpp \
    $${SRC}/des.cpp \
    $${SRC}/dessp.cpp \
    $${SRC}/dh.cpp \
    $${SRC}/dh2.cpp \
    $${SRC}/dll.cpp \
    $${SRC}/dsa.cpp \
    $${SRC}/eax.cpp \
    $${SRC}/ec2n.cpp \
    $${SRC}/eccrypto.cpp \
    $${SRC}/ecp.cpp \
    $${SRC}/elgamal.cpp \
    $${SRC}/emsa2.cpp \
    $${SRC}/eprecomp.cpp \
    $${SRC}/esign.cpp \
    $${SRC}/files.cpp \
    $${SRC}/filters.cpp \
    $${SRC}/fips140.cpp \
    $${SRC}/gcm.cpp \
    $${SRC}/gf256.cpp \
    $${SRC}/gf2_32.cpp \
    $${SRC}/gf2n.cpp \
    $${SRC}/gfpcrypt.cpp \
    $${SRC}/gost.cpp \
    $${SRC}/gzip.cpp \
    $${SRC}/hex.cpp \
    $${SRC}/hmac.cpp \
    $${SRC}/hrtimer.cpp \
    $${SRC}/ida.cpp \
    $${SRC}/idea.cpp \
    $${SRC}/integer.cpp \
    $${SRC}/iterhash.cpp \
    $${SRC}/luc.cpp \
    $${SRC}/mars.cpp \
    $${SRC}/marss.cpp \
    $${SRC}/md2.cpp \
    $${SRC}/md4.cpp \
    $${SRC}/md5.cpp \
    $${SRC}/misc.cpp \
    $${SRC}/modes.cpp \
    $${SRC}/mqueue.cpp \
    $${SRC}/mqv.cpp \
    $${SRC}/nbtheory.cpp \
    $${SRC}/network.cpp \
    $${SRC}/oaep.cpp \
    $${SRC}/osrng.cpp \
    $${SRC}/panama.cpp \
    $${SRC}/pch.cpp \
    $${SRC}/pkcspad.cpp \
    $${SRC}/polynomi.cpp \
    $${SRC}/pssr.cpp \
    $${SRC}/pubkey.cpp \
    $${SRC}/queue.cpp \
    $${SRC}/rabin.cpp \
    $${SRC}/randpool.cpp \
    $${SRC}/rc2.cpp \
    $${SRC}/rc5.cpp \
    $${SRC}/rc6.cpp \
    $${SRC}/rdtables.cpp \
    $${SRC}/rijndael.cpp \
    $${SRC}/ripemd.cpp \
    $${SRC}/rng.cpp \
    $${SRC}/rsa.cpp \
    $${SRC}/rw.cpp \
    $${SRC}/safer.cpp \
    $${SRC}/salsa.cpp \
    $${SRC}/seal.cpp \
    $${SRC}/seed.cpp \
    $${SRC}/serpent.cpp \
    $${SRC}/sha.cpp \
    $${SRC}/shacal2.cpp \
    $${SRC}/shark.cpp \
    $${SRC}/sharkbox.cpp \
    $${SRC}/simple.cpp \
    $${SRC}/skipjack.cpp \
    $${SRC}/socketft.cpp \
    $${SRC}/sosemanuk.cpp \
    $${SRC}/square.cpp \
    $${SRC}/squaretb.cpp \
    $${SRC}/strciphr.cpp \
    $${SRC}/tea.cpp \
    $${SRC}/tftables.cpp \
    $${SRC}/tiger.cpp \
    $${SRC}/tigertab.cpp \
    $${SRC}/trdlocal.cpp \
    $${SRC}/ttmac.cpp \
    $${SRC}/twofish.cpp \
    $${SRC}/vmac.cpp \
    $${SRC}/wait.cpp \
    $${SRC}/wake.cpp \
    $${SRC}/whrlpool.cpp \
    $${SRC}/winpipes.cpp \
    $${SRC}/xtr.cpp \
    $${SRC}/xtrcrypt.cpp \
    $${SRC}/zdeflate.cpp \
    $${SRC}/zinflate.cpp \
    $${SRC}/zlib.cpp

