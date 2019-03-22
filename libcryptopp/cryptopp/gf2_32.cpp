// *****************************************************************************
// gf2_32.cpp                                                      Tao3D project
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
// gf2_32.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "misc.h"
#include "gf2_32.h"

NAMESPACE_BEGIN(CryptoPP)

GF2_32::Element GF2_32::Multiply(Element a, Element b) const
{
	word32 table[4];
	table[0] = 0;
	table[1] = m_modulus;
	if (a & 0x80000000)
	{
		table[2] = m_modulus ^ (a<<1);
		table[3] = a<<1;
	}
	else
	{
		table[2] = a<<1;
		table[3] = m_modulus ^ (a<<1);
	}

#if CRYPTOPP_FAST_ROTATE(32)
	b = rotrFixed(b, 30U);
	word32 result = table[b&2];

	for (int i=29; i>=0; --i)
	{
		b = rotlFixed(b, 1U);
		result = (result<<1) ^ table[(b&2) + (result>>31)];
	}

	return (b&1) ? result ^ a : result;
#else
	word32 result = table[(b>>30) & 2];

	for (int i=29; i>=0; --i)
		result = (result<<1) ^ table[((b>>i)&2) + (result>>31)];

	return (b&1) ? result ^ a : result;
#endif
}

GF2_32::Element GF2_32::MultiplicativeInverse(Element a) const
{
	if (a <= 1)		// 1 is a special case
		return a;

	// warning - don't try to adapt this algorithm for another situation
	word32 g0=m_modulus, g1=a, g2=a;
	word32 v0=0, v1=1, v2=1;

	assert(g1);

	while (!(g2 & 0x80000000))
	{
		g2 <<= 1;
		v2 <<= 1;
	}

	g2 <<= 1;
	v2 <<= 1;

	g0 ^= g2;
	v0 ^= v2;

	while (g0 != 1)
	{
		if (g1 < g0 || ((g0^g1) < g0 && (g0^g1) < g1))
		{
			assert(BitPrecision(g1) <= BitPrecision(g0));
			g2 = g1;
			v2 = v1;
		}
		else
		{
			assert(BitPrecision(g1) > BitPrecision(g0));
			g2 = g0; g0 = g1; g1 = g2;
			v2 = v0; v0 = v1; v1 = v2;
		}

		while ((g0^g2) >= g2)
		{
			assert(BitPrecision(g0) > BitPrecision(g2));
			g2 <<= 1;
			v2 <<= 1;
		}

		assert(BitPrecision(g0) == BitPrecision(g2));
		g0 ^= g2;
		v0 ^= v2;
	}

	return v0;
}

NAMESPACE_END
