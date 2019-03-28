#ifndef CRYPTOPP_TWOFISH_H
#define CRYPTOPP_TWOFISH_H
// *****************************************************************************
// twofish.h                                                       Tao3D project
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
struct Twofish_Info : public FixedBlockSize<16>, public VariableKeyLength<16, 0, 32>, FixedRounds<16>
{
	static const char *StaticAlgorithmName() {return "Twofish";}
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#Twofish">Twofish</a>
class Twofish : public Twofish_Info, public BlockCipherDocumentation
{
	class CRYPTOPP_NO_VTABLE Base : public BlockCipherImpl<Twofish_Info>
	{
	public:
		void UncheckedSetKey(const byte *userKey, unsigned int length, const NameValuePairs &params);

	protected:
		static word32 h0(word32 x, const word32 *key, unsigned int kLen);
		static word32 h(word32 x, const word32 *key, unsigned int kLen);

		static const byte q[2][256];
		static const word32 mds[4][256];

		FixedSizeSecBlock<word32, 40> m_k;
		FixedSizeSecBlock<word32, 4*256> m_s;
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
	typedef BlockCipherFinal<ENCRYPTION, Enc> Encryption;
	typedef BlockCipherFinal<DECRYPTION, Dec> Decryption;
};

typedef Twofish::Encryption TwofishEncryption;
typedef Twofish::Decryption TwofishDecryption;

NAMESPACE_END

#endif
