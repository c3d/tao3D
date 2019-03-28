#ifndef CRYPTOPP_CAST_H
#define CRYPTOPP_CAST_H
// *****************************************************************************
// cast.h                                                          Tao3D project
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

class CAST
{
protected:
	static const word32 S[8][256];
};

//! algorithm info
struct CAST128_Info : public FixedBlockSize<8>, public VariableKeyLength<16, 5, 16>
{
	static const char *StaticAlgorithmName() {return "CAST-128";}
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#CAST-128">CAST-128</a>
class CAST128 : public CAST128_Info, public BlockCipherDocumentation
{
	class CRYPTOPP_NO_VTABLE Base : public CAST, public BlockCipherImpl<CAST128_Info>
	{
	public:
		void UncheckedSetKey(const byte *userKey, unsigned int length, const NameValuePairs &params);

	protected:
		bool reduced;
		FixedSizeSecBlock<word32, 32> K;
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

//! algorithm info
struct CAST256_Info : public FixedBlockSize<16>, public VariableKeyLength<16, 16, 32>
{
	static const char *StaticAlgorithmName() {return "CAST-256";}
};

//! <a href="http://www.weidai.com/scan-mirror/cs.html#CAST-256">CAST-256</a>
class CAST256 : public CAST256_Info, public BlockCipherDocumentation
{
	class CRYPTOPP_NO_VTABLE Base : public CAST, public BlockCipherImpl<CAST256_Info>
	{
	public:
		void UncheckedSetKey(const byte *userKey, unsigned int length, const NameValuePairs &params);
		void ProcessAndXorBlock(const byte *inBlock, const byte *xorBlock, byte *outBlock) const;

	protected:
		static const word32 t_m[8][24];
		static const unsigned int t_r[8][24];

		static void Omega(int i, word32 kappa[8]);

		FixedSizeSecBlock<word32, 8*12> K;
	};

public:
	typedef BlockCipherFinal<ENCRYPTION, Base> Encryption;
	typedef BlockCipherFinal<DECRYPTION, Base> Decryption;
};

typedef CAST128::Encryption CAST128Encryption;
typedef CAST128::Decryption CAST128Decryption;

typedef CAST256::Encryption CAST256Encryption;
typedef CAST256::Decryption CAST256Decryption;

NAMESPACE_END

#endif
