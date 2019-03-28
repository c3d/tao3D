// *****************************************************************************
// blumshub.cpp                                                    Tao3D project
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
// blumshub.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "blumshub.h"

NAMESPACE_BEGIN(CryptoPP)

PublicBlumBlumShub::PublicBlumBlumShub(const Integer &n, const Integer &seed)
	: modn(n),
	  maxBits(BitPrecision(n.BitCount())-1)
{
	current = modn.Square(modn.Square(seed));
	bitsLeft = maxBits;
}

unsigned int PublicBlumBlumShub::GenerateBit()
{
	if (bitsLeft==0)
	{
		current = modn.Square(current);
		bitsLeft = maxBits;
	}

	return current.GetBit(--bitsLeft);
}

byte PublicBlumBlumShub::GenerateByte()
{
	byte b=0;
	for (int i=0; i<8; i++)
		b = (b << 1) | PublicBlumBlumShub::GenerateBit();
	return b;
}

void PublicBlumBlumShub::GenerateBlock(byte *output, size_t size)
{
	while (size--)
		*output++ = PublicBlumBlumShub::GenerateByte();
}

void PublicBlumBlumShub::ProcessData(byte *outString, const byte *inString, size_t length)
{
	while (length--)
		*outString++ = *inString++ ^ PublicBlumBlumShub::GenerateByte();
}

BlumBlumShub::BlumBlumShub(const Integer &p, const Integer &q, const Integer &seed)
	: PublicBlumBlumShub(p*q, seed),
	  p(p), q(q),
	  x0(modn.Square(seed))
{
}

void BlumBlumShub::Seek(lword index)
{
	Integer i(Integer::POSITIVE, index);
	i *= 8;
	Integer e = a_exp_b_mod_c (2, i / maxBits + 1, (p-1)*(q-1));
	current = modn.Exponentiate(x0, e);
	bitsLeft = maxBits - i % maxBits;
}

NAMESPACE_END
