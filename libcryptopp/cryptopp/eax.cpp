// *****************************************************************************
// eax.cpp                                                         Tao3D project
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
// eax.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "eax.h"

NAMESPACE_BEGIN(CryptoPP)

void EAX_Base::SetKeyWithoutResync(const byte *userKey, size_t keylength, const NameValuePairs &params)
{
	AccessMAC().SetKey(userKey, keylength, params);
	m_buffer.New(2*AccessMAC().TagSize());
}

void EAX_Base::Resync(const byte *iv, size_t len)
{
	MessageAuthenticationCode &mac = AccessMAC();
	unsigned int blockSize = mac.TagSize();

	memset(m_buffer, 0, blockSize);
	mac.Update(m_buffer, blockSize);
	mac.CalculateDigest(m_buffer+blockSize, iv, len);

	m_buffer[blockSize-1] = 1;
	mac.Update(m_buffer, blockSize);

	m_ctr.SetCipherWithIV(AccessMAC().AccessCipher(), m_buffer+blockSize, blockSize);
}

size_t EAX_Base::AuthenticateBlocks(const byte *data, size_t len)
{
	AccessMAC().Update(data, len);
	return 0;
}

void EAX_Base::AuthenticateLastHeaderBlock()
{
	assert(m_bufferedDataLength == 0);
	MessageAuthenticationCode &mac = AccessMAC();
	unsigned int blockSize = mac.TagSize();

	mac.Final(m_buffer);
	xorbuf(m_buffer+blockSize, m_buffer, blockSize);

	memset(m_buffer, 0, blockSize);
	m_buffer[blockSize-1] = 2;
	mac.Update(m_buffer, blockSize);
}

void EAX_Base::AuthenticateLastFooterBlock(byte *tag, size_t macSize)
{
	assert(m_bufferedDataLength == 0);
	MessageAuthenticationCode &mac = AccessMAC();
	unsigned int blockSize = mac.TagSize();

	mac.TruncatedFinal(m_buffer, macSize);
	xorbuf(tag, m_buffer, m_buffer+blockSize, macSize);
}

NAMESPACE_END
