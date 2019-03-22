// *****************************************************************************
// hmac.h                                                          Tao3D project
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
// hmac.h - written and placed in the public domain by Wei Dai

#ifndef CRYPTOPP_HMAC_H
#define CRYPTOPP_HMAC_H

#include "seckey.h"
#include "secblock.h"

NAMESPACE_BEGIN(CryptoPP)

//! _
class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE HMAC_Base : public VariableKeyLength<16, 0, INT_MAX>, public MessageAuthenticationCode
{
public:
	HMAC_Base() : m_innerHashKeyed(false) {}
	void UncheckedSetKey(const byte *userKey, unsigned int keylength, const NameValuePairs &params);

	void Restart();
	void Update(const byte *input, size_t length);
	void TruncatedFinal(byte *mac, size_t size);
	unsigned int OptimalBlockSize() const {return const_cast<HMAC_Base*>(this)->AccessHash().OptimalBlockSize();}
	unsigned int DigestSize() const {return const_cast<HMAC_Base*>(this)->AccessHash().DigestSize();}

protected:
	virtual HashTransformation & AccessHash() =0;
	byte * AccessIpad() {return m_buf;}
	byte * AccessOpad() {return m_buf + AccessHash().BlockSize();}
	byte * AccessInnerHash() {return m_buf + 2*AccessHash().BlockSize();}

private:
	void KeyInnerHash();

	SecByteBlock m_buf;
	bool m_innerHashKeyed;
};

//! <a href="http://www.weidai.com/scan-mirror/mac.html#HMAC">HMAC</a>
/*! HMAC(K, text) = H(K XOR opad, H(K XOR ipad, text)) */
template <class T>
class HMAC : public MessageAuthenticationCodeImpl<HMAC_Base, HMAC<T> >
{
public:
	CRYPTOPP_CONSTANT(DIGESTSIZE=T::DIGESTSIZE)
	CRYPTOPP_CONSTANT(BLOCKSIZE=T::BLOCKSIZE)

	HMAC() {}
	HMAC(const byte *key, size_t length=HMAC_Base::DEFAULT_KEYLENGTH)
		{this->SetKey(key, length);}

	static std::string StaticAlgorithmName() {return std::string("HMAC(") + T::StaticAlgorithmName() + ")";}
	std::string AlgorithmName() const {return std::string("HMAC(") + m_hash.AlgorithmName() + ")";}

private:
	HashTransformation & AccessHash() {return m_hash;}

	T m_hash;
};

NAMESPACE_END

#endif
