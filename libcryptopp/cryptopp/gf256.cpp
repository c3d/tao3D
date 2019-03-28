// *****************************************************************************
// gf256.cpp                                                       Tao3D project
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
// gf256.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "gf256.h"

NAMESPACE_BEGIN(CryptoPP)

GF256::Element GF256::Multiply(Element a, Element b) const
{
	word result = 0, t = b;

	for (unsigned int i=0; i<8; i++)
	{
		result <<= 1;
		if (result & 0x100)
			result ^= m_modulus;

		t <<= 1;
		if (t & 0x100)
			result ^= a;
	}

	return (GF256::Element) result;
}

GF256::Element GF256::MultiplicativeInverse(Element a) const
{
	Element result = a;
	for (int i=1; i<7; i++)
		result = Multiply(Square(result), a);
	return Square(result);
}

NAMESPACE_END
