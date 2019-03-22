#ifndef CRYPTOPP_THREEWAY_H
#define CRYPTOPP_THREEWAY_H
// *****************************************************************************
// 3way.h                                                          Tao3D project
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

/** \file
*/

#include "seckey.h"
#include "secblock.h"

NAMESPACE_BEGIN(CryptoPP)

//! _
struct ThreeWay_Info : public FixedBlockSize<12>, public FixedKeyLength<12>, public VariableRounds<11>
{
	static const char *StaticAlgorithmName() {return "3-Way";}
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#3-Way">3-Way</a>
class ThreeWay : public ThreeWay_Info, public BlockCipherDocumentation
{
	class CRYPTOPP_NO_VTABLE Base : public BlockCipherImpl<ThreeWay_Info>
	{
	public:
		void UncheckedSetKey(const byte *key, unsigned int length, const NameValuePairs &params);

	protected:
		unsigned int m_rounds;
		FixedSizeSecBlock<word32, 3> m_k;
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

typedef ThreeWay::Encryption ThreeWayEncryption;
typedef ThreeWay::Decryption ThreeWayDecryption;

NAMESPACE_END

#endif
