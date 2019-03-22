#ifndef CRYPTOPP_CAMELLIA_H
#define CRYPTOPP_CAMELLIA_H
// *****************************************************************************
// camellia.h                                                      Tao3D project
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

#include "config.h"

/** \file
*/

#include "seckey.h"
#include "secblock.h"

NAMESPACE_BEGIN(CryptoPP)

//! _
struct Camellia_Info : public FixedBlockSize<16>, public VariableKeyLength<16, 16, 32, 8>
{
	static const char *StaticAlgorithmName() {return "Camellia";}
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#Camellia">Camellia</a>
class Camellia : public Camellia_Info, public BlockCipherDocumentation
{
	class CRYPTOPP_NO_VTABLE Base : public BlockCipherImpl<Camellia_Info>
	{
	public:
		void UncheckedSetKey(const byte *key, unsigned int keylen, const NameValuePairs &params);
		void ProcessAndXorBlock(const byte *inBlock, const byte *xorBlock, byte *outBlock) const;

	protected:
		static const byte s1[256];
		static const word32 SP[4][256];

		unsigned int m_rounds;
		SecBlock<word32> m_key;
	};

public:
	typedef BlockCipherFinal<ENCRYPTION, Base> Encryption;
	typedef BlockCipherFinal<DECRYPTION, Base> Decryption;
};

typedef Camellia::Encryption CamelliaEncryption;
typedef Camellia::Decryption CamelliaDecryption;

NAMESPACE_END

#endif
