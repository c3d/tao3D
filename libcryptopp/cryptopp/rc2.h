#ifndef CRYPTOPP_RC2_H
#define CRYPTOPP_RC2_H
// *****************************************************************************
// rc2.h                                                           Tao3D project
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

/** \file
*/

#include "seckey.h"
#include "secblock.h"
#include "algparam.h"

NAMESPACE_BEGIN(CryptoPP)

//! _
struct RC2_Info : public FixedBlockSize<8>, public VariableKeyLength<16, 1, 128>
{
	CRYPTOPP_CONSTANT(DEFAULT_EFFECTIVE_KEYLENGTH = 1024)
	CRYPTOPP_CONSTANT(MAX_EFFECTIVE_KEYLENGTH = 1024)
	static const char *StaticAlgorithmName() {return "RC2";}
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#RC2">RC2</a>
class RC2 : public RC2_Info, public BlockCipherDocumentation
{
	class CRYPTOPP_NO_VTABLE Base : public BlockCipherImpl<RC2_Info>
	{
	public:
		void UncheckedSetKey(const byte *userKey, unsigned int length, const NameValuePairs &params);
		unsigned int OptimalDataAlignment() const {return GetAlignmentOf<word16>();}

	protected:
		FixedSizeSecBlock<word16, 64> K;  // expanded key table
	};

	class CRYPTOPP_NO_VTABLE Enc : public Base
	{
	public:
		void ProcessAndXorBlock(const byte *inBlock, const byte *xorBlock, byte *outBlock) const;
	};

	class CRYPTOPP_NO_VTABLE Dec : public Base
	{
	public:
		void ProcessAndXorBlock(const byte *inBlock, const byte *xorBlock, byte *outBlock) const;
	};

public:
	class Encryption : public BlockCipherFinal<ENCRYPTION, Enc>
	{
	public:
		Encryption() {}
		Encryption(const byte *key, size_t keyLen=DEFAULT_KEYLENGTH)
			{SetKey(key, keyLen);}
		Encryption(const byte *key, size_t keyLen, int effectiveKeyLen)
			{SetKey(key, keyLen, MakeParameters("EffectiveKeyLength", effectiveKeyLen));}
	};

	class Decryption : public BlockCipherFinal<DECRYPTION, Dec>
	{
	public:
		Decryption() {}
		Decryption(const byte *key, size_t keyLen=DEFAULT_KEYLENGTH)
			{SetKey(key, keyLen);}
		Decryption(const byte *key, size_t keyLen, int effectiveKeyLen)
			{SetKey(key, keyLen, MakeParameters("EffectiveKeyLength", effectiveKeyLen));}
	};
};

typedef RC2::Encryption RC2Encryption;
typedef RC2::Decryption RC2Decryption;

NAMESPACE_END

#endif
