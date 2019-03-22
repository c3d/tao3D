// *****************************************************************************
// adler32.cpp                                                     Tao3D project
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
// adler32.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "adler32.h"

NAMESPACE_BEGIN(CryptoPP)

void Adler32::Update(const byte *input, size_t length)
{
	const unsigned long BASE = 65521;

	unsigned long s1 = m_s1;
	unsigned long s2 = m_s2;

	if (length % 8 != 0)
	{
		do
		{
			s1 += *input++;
			s2 += s1;
			length--;
		} while (length % 8 != 0);

		if (s1 >= BASE)
			s1 -= BASE;
		s2 %= BASE;
	}

	while (length > 0)
	{
		s1 += input[0]; s2 += s1;
		s1 += input[1]; s2 += s1;
		s1 += input[2]; s2 += s1;
		s1 += input[3]; s2 += s1;
		s1 += input[4]; s2 += s1;
		s1 += input[5]; s2 += s1;
		s1 += input[6]; s2 += s1;
		s1 += input[7]; s2 += s1;

		length -= 8;
		input += 8;

		if (s1 >= BASE)
			s1 -= BASE;
		if (length % 0x8000 == 0)
			s2 %= BASE;
	}

	assert(s1 < BASE);
	assert(s2 < BASE);

	m_s1 = (word16)s1;
	m_s2 = (word16)s2;
}

void Adler32::TruncatedFinal(byte *hash, size_t size)
{
	ThrowIfInvalidTruncatedSize(size);

	switch (size)
	{
	default:
		hash[3] = byte(m_s1);
	case 3:
		hash[2] = byte(m_s1 >> 8);
	case 2:
		hash[1] = byte(m_s2);
	case 1:
		hash[0] = byte(m_s2 >> 8);
	case 0:
		;
	}

	Reset();
}

NAMESPACE_END
