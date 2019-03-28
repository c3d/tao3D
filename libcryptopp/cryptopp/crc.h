#ifndef CRYPTOPP_CRC32_H
#define CRYPTOPP_CRC32_H
// *****************************************************************************
// crc.h                                                           Tao3D project
// *****************************************************************************
//
// File description:
//
//
//
//
//
//
//
//
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2011, Jérôme Forissier <jerome@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Tao3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Tao3D, in a file named COPYING.
// If not, see <https://www.gnu.org/licenses/>.
// *****************************************************************************

#include "cryptlib.h"

NAMESPACE_BEGIN(CryptoPP)

const word32 CRC32_NEGL = 0xffffffffL;

#ifdef IS_LITTLE_ENDIAN
#define CRC32_INDEX(c) (c & 0xff)
#define CRC32_SHIFTED(c) (c >> 8)
#else
#define CRC32_INDEX(c) (c >> 24)
#define CRC32_SHIFTED(c) (c << 8)
#endif

//! CRC Checksum Calculation
class CRC32 : public HashTransformation
{
public:
	CRYPTOPP_CONSTANT(DIGESTSIZE = 4)
	CRC32();
	void Update(const byte *input, size_t length);
	void TruncatedFinal(byte *hash, size_t size);
	unsigned int DigestSize() const {return DIGESTSIZE;}
    static const char * StaticAlgorithmName() {return "CRC32";}
    std::string AlgorithmName() const {return StaticAlgorithmName();}

	void UpdateByte(byte b) {m_crc = m_tab[CRC32_INDEX(m_crc) ^ b] ^ CRC32_SHIFTED(m_crc);}
	byte GetCrcByte(size_t i) const {return ((byte *)&(m_crc))[i];}

private:
	void Reset() {m_crc = CRC32_NEGL;}
	
	static const word32 m_tab[256];
	word32 m_crc;
};

NAMESPACE_END

#endif
