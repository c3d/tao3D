#ifndef CRYPTOPP_BLOWFISH_H
#define CRYPTOPP_BLOWFISH_H
// *****************************************************************************
// blowfish.h                                                      Tao3D project
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

/** \file */

#include "seckey.h"
#include "secblock.h"

NAMESPACE_BEGIN(CryptoPP)

//! _
struct Blowfish_Info : public FixedBlockSize<8>, public VariableKeyLength<16, 4, 56>, public FixedRounds<16>
{
	static const char *StaticAlgorithmName() {return "Blowfish";}
};

//! <a href="http://www.weidai.com/scan-mirror/cs.html#Blowfish">Blowfish</a>
class Blowfish : public Blowfish_Info, public BlockCipherDocumentation
{
	class CRYPTOPP_NO_VTABLE Base : public BlockCipherImpl<Blowfish_Info>
	{
	public:
		void ProcessAndXorBlock(const byte *inBlock, const byte *xorBlock, byte *outBlock) const;
		void UncheckedSetKey(const byte *key_string, unsigned int keylength, const NameValuePairs &params);

	private:
		void crypt_block(const word32 in[2], word32 out[2]) const;

		static const word32 p_init[ROUNDS+2];
		static const word32 s_init[4*256];

		FixedSizeSecBlock<word32, ROUNDS+2> pbox;
		FixedSizeSecBlock<word32, 4*256> sbox;
	};

public:
	typedef BlockCipherFinal<ENCRYPTION, Base> Encryption;
	typedef BlockCipherFinal<DECRYPTION, Base> Decryption;
};

typedef Blowfish::Encryption BlowfishEncryption;
typedef Blowfish::Decryption BlowfishDecryption;

NAMESPACE_END

#endif
