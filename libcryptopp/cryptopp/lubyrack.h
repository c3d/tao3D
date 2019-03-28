// *****************************************************************************
// lubyrack.h                                                      Tao3D project
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
// lubyrack.h - written and placed in the public domain by Wei Dai

#ifndef CRYPTOPP_LUBYRACK_H
#define CRYPTOPP_LUBYRACK_H

/** \file */

#include "simple.h"
#include "secblock.h"

NAMESPACE_BEGIN(CryptoPP)

template <class T> struct DigestSizeDoubleWorkaround 	// VC60 workaround
{
	CRYPTOPP_CONSTANT(RESULT = 2*T::DIGESTSIZE)
};

//! algorithm info
template <class T>
struct LR_Info : public VariableKeyLength<16, 0, 2*(INT_MAX/2), 2>, public FixedBlockSize<DigestSizeDoubleWorkaround<T>::RESULT>
{
	static std::string StaticAlgorithmName() {return std::string("LR/")+T::StaticAlgorithmName();}
};

//! Luby-Rackoff
template <class T>
class LR : public LR_Info<T>, public BlockCipherDocumentation
{
	class CRYPTOPP_NO_VTABLE Base : public BlockCipherImpl<LR_Info<T> >
	{
	public:
		// VC60 workaround: have to define these functions within class definition
		void UncheckedSetKey(const byte *userKey, unsigned int length, const NameValuePairs &params)
		{
			this->AssertValidKeyLength(length);

			L = length/2;
			buffer.New(2*S);
			digest.New(S);
			key.Assign(userKey, 2*L);
		}

	protected:
		CRYPTOPP_CONSTANT(S=T::DIGESTSIZE)
		unsigned int L;	// key length / 2
		SecByteBlock key;

		mutable T hm;
		mutable SecByteBlock buffer, digest;
	};

	class CRYPTOPP_NO_VTABLE Enc : public Base
	{
	public:

#define KL this->key
#define KR this->key+this->L
#define BL this->buffer
#define BR this->buffer+this->S
#define IL inBlock
#define IR inBlock+this->S
#define OL outBlock
#define OR outBlock+this->S

		void ProcessAndXorBlock(const byte *inBlock, const byte *xorBlock, byte *outBlock) const
		{
			this->hm.Update(KL, this->L);
			this->hm.Update(IL, this->S);
			this->hm.Final(BR);
			xorbuf(BR, IR, this->S);

			this->hm.Update(KR, this->L);
			this->hm.Update(BR, this->S);
			this->hm.Final(BL);
			xorbuf(BL, IL, this->S);

			this->hm.Update(KL, this->L);
			this->hm.Update(BL, this->S);
			this->hm.Final(this->digest);
			xorbuf(BR, this->digest, this->S);

			this->hm.Update(KR, this->L);
			this->hm.Update(OR, this->S);
			this->hm.Final(this->digest);
			xorbuf(BL, this->digest, this->S);

			if (xorBlock)
				xorbuf(outBlock, xorBlock, this->buffer, 2*this->S);
			else
				memcpy_s(outBlock, 2*this->S, this->buffer, 2*this->S);
		}
	};

	class CRYPTOPP_NO_VTABLE Dec : public Base
	{
	public:
		void ProcessAndXorBlock(const byte *inBlock, const byte *xorBlock, byte *outBlock) const
		{
			this->hm.Update(KR, this->L);
			this->hm.Update(IR, this->S);
			this->hm.Final(BL);
			xorbuf(BL, IL, this->S);

			this->hm.Update(KL, this->L);
			this->hm.Update(BL, this->S);
			this->hm.Final(BR);
			xorbuf(BR, IR, this->S);

			this->hm.Update(KR, this->L);
			this->hm.Update(BR, this->S);
			this->hm.Final(this->digest);
			xorbuf(BL, this->digest, this->S);

			this->hm.Update(KL, this->L);
			this->hm.Update(OL, this->S);
			this->hm.Final(this->digest);
			xorbuf(BR, this->digest, this->S);

			if (xorBlock)
				xorbuf(outBlock, xorBlock, this->buffer, 2*this->S);
			else
				memcpy(outBlock, this->buffer, 2*this->S);
		}
#undef KL
#undef KR
#undef BL
#undef BR
#undef IL
#undef IR
#undef OL
#undef OR
	};

public:
	typedef BlockCipherFinal<ENCRYPTION, Enc> Encryption;
	typedef BlockCipherFinal<DECRYPTION, Dec> Decryption;
};

NAMESPACE_END

#endif
