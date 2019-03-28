#ifndef CRYPTOPP_SAFER_H
#define CRYPTOPP_SAFER_H
// *****************************************************************************
// safer.h                                                         Tao3D project
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

/// base class, do not use directly
class SAFER
{
public:
	class CRYPTOPP_NO_VTABLE Base : public BlockCipher
	{
	public:
		unsigned int OptimalDataAlignment() const {return 1;}
		void UncheckedSetKey(const byte *userkey, unsigned int length, const NameValuePairs &params);

	protected:
		virtual bool Strengthened() const =0;

		SecByteBlock keySchedule;
		static const byte exp_tab[256];
		static const byte log_tab[256];
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
};

template <class BASE, class INFO, bool STR>
class CRYPTOPP_NO_VTABLE SAFER_Impl : public BlockCipherImpl<INFO, BASE>
{
protected:
	bool Strengthened() const {return STR;}
};

//! _
struct SAFER_K_Info : public FixedBlockSize<8>, public VariableKeyLength<16, 8, 16, 8>, public VariableRounds<10, 1, 13>
{
	static const char *StaticAlgorithmName() {return "SAFER-K";}
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#SAFER-K">SAFER-K</a>
class SAFER_K : public SAFER_K_Info, public SAFER, public BlockCipherDocumentation
{
public:
	typedef BlockCipherFinal<ENCRYPTION, SAFER_Impl<Enc, SAFER_K_Info, false> > Encryption;
	typedef BlockCipherFinal<DECRYPTION, SAFER_Impl<Dec, SAFER_K_Info, false> > Decryption;
};

//! _
struct SAFER_SK_Info : public FixedBlockSize<8>, public VariableKeyLength<16, 8, 16, 8>, public VariableRounds<10, 1, 13>
{
	static const char *StaticAlgorithmName() {return "SAFER-SK";}
};

/// <a href="http://www.weidai.com/scan-mirror/cs.html#SAFER-SK">SAFER-SK</a>
class SAFER_SK : public SAFER_SK_Info, public SAFER, public BlockCipherDocumentation
{
public:
	typedef BlockCipherFinal<ENCRYPTION, SAFER_Impl<Enc, SAFER_SK_Info, true> > Encryption;
	typedef BlockCipherFinal<DECRYPTION, SAFER_Impl<Dec, SAFER_SK_Info, true> > Decryption;
};

typedef SAFER_K::Encryption SAFER_K_Encryption;
typedef SAFER_K::Decryption SAFER_K_Decryption;

typedef SAFER_SK::Encryption SAFER_SK_Encryption;
typedef SAFER_SK::Decryption SAFER_SK_Decryption;

NAMESPACE_END

#endif
