// *****************************************************************************
// cbcmac.cpp                                                      Tao3D project
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
#include "pch.h"

#ifndef CRYPTOPP_IMPORTS

#include "cbcmac.h"

NAMESPACE_BEGIN(CryptoPP)

void CBC_MAC_Base::UncheckedSetKey(const byte *key, unsigned int length, const NameValuePairs &params)
{
	AccessCipher().SetKey(key, length, params);
	m_reg.CleanNew(AccessCipher().BlockSize());
	m_counter = 0;
}

void CBC_MAC_Base::Update(const byte *input, size_t length)
{
	unsigned int blockSize = AccessCipher().BlockSize();

	while (m_counter && length)
	{
		m_reg[m_counter++] ^= *input++;
		if (m_counter == blockSize)
			ProcessBuf();
		length--;
	}

	if (length >= blockSize)
	{
		size_t leftOver = AccessCipher().AdvancedProcessBlocks(m_reg, input, m_reg, length, BlockTransformation::BT_DontIncrementInOutPointers|BlockTransformation::BT_XorInput);
		input += (length - leftOver);
		length = leftOver;
	}

	while (length--)
	{
		m_reg[m_counter++] ^= *input++;
		if (m_counter == blockSize)
			ProcessBuf();
	}
}

void CBC_MAC_Base::TruncatedFinal(byte *mac, size_t size)
{
	ThrowIfInvalidTruncatedSize(size);

	if (m_counter)
		ProcessBuf();

	memcpy(mac, m_reg, size);
	memset(m_reg, 0, AccessCipher().BlockSize());
}

void CBC_MAC_Base::ProcessBuf()
{
	AccessCipher().ProcessBlock(m_reg);
	m_counter = 0;
}

NAMESPACE_END

#endif
