// *****************************************************************************
// tea.cpp                                                         Tao3D project
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
// tea.cpp - modified by Wei Dai from code in the original paper

#include "pch.h"
#include "tea.h"
#include "misc.h"

NAMESPACE_BEGIN(CryptoPP)

static const word32 DELTA = 0x9e3779b9;
typedef BlockGetAndPut<word32, BigEndian> Block;

void TEA::Base::UncheckedSetKey(const byte *userKey, unsigned int length, const NameValuePairs &params)
{
	AssertValidKeyLength(length);

	GetUserKey(BIG_ENDIAN_ORDER, m_k.begin(), 4, userKey, KEYLENGTH);
	m_limit = GetRoundsAndThrowIfInvalid(params, this) * DELTA;
}

void TEA::Enc::ProcessAndXorBlock(const byte *inBlock, const byte *xorBlock, byte *outBlock) const
{
	word32 y, z;
	Block::Get(inBlock)(y)(z);

	word32 sum = 0;
	while (sum != m_limit)
	{
		sum += DELTA;
		y += (z << 4) + m_k[0] ^ z + sum ^ (z >> 5) + m_k[1];
		z += (y << 4) + m_k[2] ^ y + sum ^ (y >> 5) + m_k[3];
	}

	Block::Put(xorBlock, outBlock)(y)(z);
}

void TEA::Dec::ProcessAndXorBlock(const byte *inBlock, const byte *xorBlock, byte *outBlock) const
{
	word32 y, z;
	Block::Get(inBlock)(y)(z);

	word32 sum = m_limit;
	while (sum != 0)
	{
		z -= (y << 4) + m_k[2] ^ y + sum ^ (y >> 5) + m_k[3];
		y -= (z << 4) + m_k[0] ^ z + sum ^ (z >> 5) + m_k[1];
		sum -= DELTA;
	}

	Block::Put(xorBlock, outBlock)(y)(z);
}

void XTEA::Base::UncheckedSetKey(const byte *userKey, unsigned int length,  const NameValuePairs &params)
{
	AssertValidKeyLength(length);

	GetUserKey(BIG_ENDIAN_ORDER, m_k.begin(), 4, userKey, KEYLENGTH);
	m_limit = GetRoundsAndThrowIfInvalid(params, this) * DELTA;
}

void XTEA::Enc::ProcessAndXorBlock(const byte *inBlock, const byte *xorBlock, byte *outBlock) const
{
	word32 y, z;
	Block::Get(inBlock)(y)(z);

#ifdef __SUNPRO_CC
	// workaround needed on Sun Studio 12u1 Sun C++ 5.10 SunOS_i386 128229-02 2009/09/21
	size_t sum = 0;
	while ((sum&0xffffffff) != m_limit)
#else
	word32 sum = 0;
	while (sum != m_limit)
#endif
	{
		y += (z<<4 ^ z>>5) + z ^ sum + m_k[sum&3];
		sum += DELTA;
		z += (y<<4 ^ y>>5) + y ^ sum + m_k[sum>>11 & 3];
	}

	Block::Put(xorBlock, outBlock)(y)(z);
}

void XTEA::Dec::ProcessAndXorBlock(const byte *inBlock, const byte *xorBlock, byte *outBlock) const
{
	word32 y, z;
	Block::Get(inBlock)(y)(z);

#ifdef __SUNPRO_CC
	// workaround needed on Sun Studio 12u1 Sun C++ 5.10 SunOS_i386 128229-02 2009/09/21
	size_t sum = m_limit;
	while ((sum&0xffffffff) != 0)
#else
	word32 sum = m_limit;
	while (sum != 0)
#endif
	{
		z -= (y<<4 ^ y>>5) + y ^ sum + m_k[sum>>11 & 3];
		sum -= DELTA;
		y -= (z<<4 ^ z>>5) + z ^ sum + m_k[sum&3];
	}

	Block::Put(xorBlock, outBlock)(y)(z);
}

#define MX (z>>5^y<<2)+(y>>3^z<<4)^(sum^y)+(m_k[p&3^e]^z)

void BTEA::Enc::ProcessAndXorBlock(const byte *inBlock, const byte *xorBlock, byte *outBlock) const
{
	unsigned int n = m_blockSize / 4;
	word32 *v = (word32*)outBlock;
	ConditionalByteReverse(BIG_ENDIAN_ORDER, v, (const word32*)inBlock, m_blockSize);

	word32 y = v[0], z = v[n-1], e;
	word32 p, q = 6+52/n;
	word32 sum = 0;
	
	while (q-- > 0)
	{
		sum += DELTA;
		e = sum>>2 & 3;
		for (p = 0; p < n-1; p++)
		{
			y = v[p+1];
			z = v[p] += MX;
		}
		y = v[0];
		z = v[n-1] += MX;
	}

	ConditionalByteReverse(BIG_ENDIAN_ORDER, v, v, m_blockSize);
}

void BTEA::Dec::ProcessAndXorBlock(const byte *inBlock, const byte *xorBlock, byte *outBlock) const
{
	unsigned int n = m_blockSize / 4;
	word32 *v = (word32*)outBlock;
	ConditionalByteReverse(BIG_ENDIAN_ORDER, v, (const word32*)inBlock, m_blockSize);

	word32 y = v[0], z = v[n-1], e;
	word32 p, q = 6+52/n;
	word32 sum = q * DELTA;

	while (sum != 0)
	{
		e = sum>>2 & 3;
		for (p = n-1; p > 0; p--)
		{
			z = v[p-1];
			y = v[p] -= MX;
		}

		z = v[n-1];
		y = v[0] -= MX;
		sum -= DELTA;
	}

	ConditionalByteReverse(BIG_ENDIAN_ORDER, v, v, m_blockSize);
}

NAMESPACE_END
