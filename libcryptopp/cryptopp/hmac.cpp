// *****************************************************************************
// hmac.cpp                                                        Tao3D project
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
// hmac.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"

#ifndef CRYPTOPP_IMPORTS

#include "hmac.h"

NAMESPACE_BEGIN(CryptoPP)

void HMAC_Base::UncheckedSetKey(const byte *userKey, unsigned int keylength, const NameValuePairs &)
{
	AssertValidKeyLength(keylength);

	Restart();

	HashTransformation &hash = AccessHash();
	unsigned int blockSize = hash.BlockSize();

	if (!blockSize)
		throw InvalidArgument("HMAC: can only be used with a block-based hash function");

	m_buf.resize(2*AccessHash().BlockSize() + AccessHash().DigestSize());

	if (keylength <= blockSize)
		memcpy(AccessIpad(), userKey, keylength);
	else
	{
		AccessHash().CalculateDigest(AccessIpad(), userKey, keylength);
		keylength = hash.DigestSize();
	}

	assert(keylength <= blockSize);
	memset(AccessIpad()+keylength, 0, blockSize-keylength);

	for (unsigned int i=0; i<blockSize; i++)
	{
		AccessOpad()[i] = AccessIpad()[i] ^ 0x5c;
		AccessIpad()[i] ^= 0x36;
	}
}

void HMAC_Base::KeyInnerHash()
{
	assert(!m_innerHashKeyed);
	HashTransformation &hash = AccessHash();
	hash.Update(AccessIpad(), hash.BlockSize());
	m_innerHashKeyed = true;
}

void HMAC_Base::Restart()
{
	if (m_innerHashKeyed)
	{
		AccessHash().Restart();
		m_innerHashKeyed = false;
	}
}

void HMAC_Base::Update(const byte *input, size_t length)
{
	if (!m_innerHashKeyed)
		KeyInnerHash();
	AccessHash().Update(input, length);
}

void HMAC_Base::TruncatedFinal(byte *mac, size_t size)
{
	ThrowIfInvalidTruncatedSize(size);

	HashTransformation &hash = AccessHash();

	if (!m_innerHashKeyed)
		KeyInnerHash();
	hash.Final(AccessInnerHash());

	hash.Update(AccessOpad(), hash.BlockSize());
	hash.Update(AccessInnerHash(), hash.DigestSize());
	hash.TruncatedFinal(mac, size);

	m_innerHashKeyed = false;
}

NAMESPACE_END

#endif
