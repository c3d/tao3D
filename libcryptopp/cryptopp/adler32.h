#ifndef CRYPTOPP_ADLER32_H
#define CRYPTOPP_ADLER32_H
// *****************************************************************************
// adler32.h                                                       Tao3D project
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
// Tao3D is free software: you can r redistribute it and/or modify
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

//! ADLER-32 checksum calculations
class Adler32 : public HashTransformation
{
public:
	CRYPTOPP_CONSTANT(DIGESTSIZE = 4)
	Adler32() {Reset();}
	void Update(const byte *input, size_t length);
	void TruncatedFinal(byte *hash, size_t size);
	unsigned int DigestSize() const {return DIGESTSIZE;}
    static const char * StaticAlgorithmName() {return "Adler32";}
    std::string AlgorithmName() const {return StaticAlgorithmName();}

private:
	void Reset() {m_s1 = 1; m_s2 = 0;}

	word16 m_s1, m_s2;
};

NAMESPACE_END

#endif
