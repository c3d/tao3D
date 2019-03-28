#ifndef CRYPTOPP_GF2_32_H
#define CRYPTOPP_GF2_32_H
// *****************************************************************************
// gf2_32.h                                                        Tao3D project
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

#include "cryptlib.h"

NAMESPACE_BEGIN(CryptoPP)

//! GF(2^32) with polynomial basis
class GF2_32
{
public:
	typedef word32 Element;
	typedef int RandomizationParameter;

	GF2_32(word32 modulus=0x0000008D) : m_modulus(modulus) {}

	Element RandomElement(RandomNumberGenerator &rng, int ignored = 0) const
		{return rng.GenerateWord32();}

	bool Equal(Element a, Element b) const
		{return a==b;}

	Element Identity() const
		{return 0;}

	Element Add(Element a, Element b) const
		{return a^b;}

	Element& Accumulate(Element &a, Element b) const
		{return a^=b;}

	Element Inverse(Element a) const
		{return a;}

	Element Subtract(Element a, Element b) const
		{return a^b;}

	Element& Reduce(Element &a, Element b) const
		{return a^=b;}

	Element Double(Element a) const
		{return 0;}

	Element MultiplicativeIdentity() const
		{return 1;}

	Element Multiply(Element a, Element b) const;

	Element Square(Element a) const
		{return Multiply(a, a);}

	bool IsUnit(Element a) const
		{return a != 0;}

	Element MultiplicativeInverse(Element a) const;

	Element Divide(Element a, Element b) const
		{return Multiply(a, MultiplicativeInverse(b));}

private:
	word32 m_modulus;
};

NAMESPACE_END

#endif
