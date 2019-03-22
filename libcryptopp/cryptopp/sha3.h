// *****************************************************************************
// sha3.h                                                          Tao3D project
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
// (C) 2013, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
// (C) 2013,2019, Christophe de Dinechin <christophe@dinechin.org>
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
// sha3.h - written and placed in the public domain by Wei Dai

#ifndef CRYPTOPP_SHA3_H
#define CRYPTOPP_SHA3_H

#include "cryptlib.h"
#include "secblock.h"

NAMESPACE_BEGIN(CryptoPP)

/// <a href="http://en.wikipedia.org/wiki/SHA-3">SHA-3</a>
class SHA3 : public HashTransformation
{
public:
	SHA3(unsigned int digestSize) : m_digestSize(digestSize) {Restart();}
	unsigned int DigestSize() const {return m_digestSize;}
	std::string AlgorithmName() const {return "SHA-3-" + IntToString(m_digestSize*8);}
	unsigned int OptimalDataAlignment() const {return GetAlignmentOf<word64>();}

	void Update(const byte *input, size_t length);
	void Restart();
	void TruncatedFinal(byte *hash, size_t size);

protected:
	inline unsigned int r() const {return 200 - 2 * m_digestSize;}

	FixedSizeSecBlock<word64, 25> m_state;
	unsigned int m_digestSize, m_counter;
};

class SHA3_224 : public SHA3
{
public:
	CRYPTOPP_CONSTANT(DIGESTSIZE = 28)
	SHA3_224() : SHA3(DIGESTSIZE) {}
	static const char * StaticAlgorithmName() {return "SHA-3-224";}
};

class SHA3_256 : public SHA3
{
public:
	CRYPTOPP_CONSTANT(DIGESTSIZE = 32)
	SHA3_256() : SHA3(DIGESTSIZE) {}
	static const char * StaticAlgorithmName() {return "SHA-3-256";}
};

class SHA3_384 : public SHA3
{
public:
	CRYPTOPP_CONSTANT(DIGESTSIZE = 48)
	SHA3_384() : SHA3(DIGESTSIZE) {}
	static const char * StaticAlgorithmName() {return "SHA-3-384";}
};

class SHA3_512 : public SHA3
{
public:
	CRYPTOPP_CONSTANT(DIGESTSIZE = 64)
	SHA3_512() : SHA3(DIGESTSIZE) {}
	static const char * StaticAlgorithmName() {return "SHA-3-512";}
};

NAMESPACE_END

#endif
