#ifndef CRYPTOPP_SKIPJACK_H
#define CRYPTOPP_SKIPJACK_H
// *****************************************************************************
// skipjack.h                                                      Tao3D project
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

NAMESPACE_BEGIN(CryptoPP)

//! _
struct SKIPJACK_Info : public FixedBlockSize<8>, public FixedKeyLength<10>
{
	CRYPTOPP_DLL static const char * CRYPTOPP_API StaticAlgorithmName() {return "SKIPJACK";}
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#SKIPJACK">SKIPJACK</a>
class SKIPJACK : public SKIPJACK_Info, public BlockCipherDocumentation
{
	class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE Base : public BlockCipherImpl<SKIPJACK_Info>
	{
	public:
		void UncheckedSetKey(const byte *userKey, unsigned int length, const NameValuePairs &params);
		unsigned int OptimalDataAlignment() const {return GetAlignmentOf<word16>();}

	protected:
		static const byte fTable[256];

		FixedSizeSecBlock<byte, 10*256> tab;
	};

	class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE Enc : public Base
	{
	public:
		void ProcessAndXorBlock(const byte *inBlock, const byte *xorBlock, byte *outBlock) const;
	private:
		static const byte Se[256];
		static const word32 Te[4][256];
	};

	class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE Dec : public Base
	{
	public:
		void ProcessAndXorBlock(const byte *inBlock, const byte *xorBlock, byte *outBlock) const;
	private:
		static const byte Sd[256];
		static const word32 Td[4][256];
	};

public:
	typedef BlockCipherFinal<ENCRYPTION, Enc> Encryption;
	typedef BlockCipherFinal<DECRYPTION, Dec> Decryption;
};

typedef SKIPJACK::Encryption SKIPJACKEncryption;
typedef SKIPJACK::Decryption SKIPJACKDecryption;

NAMESPACE_END

#endif
