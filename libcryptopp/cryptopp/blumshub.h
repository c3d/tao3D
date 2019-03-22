#ifndef CRYPTOPP_BLUMSHUB_H
#define CRYPTOPP_BLUMSHUB_H
// *****************************************************************************
// blumshub.h                                                      Tao3D project
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

#include "modarith.h"

NAMESPACE_BEGIN(CryptoPP)

class BlumGoldwasserPublicKey;
class BlumGoldwasserPrivateKey;

//! BlumBlumShub without factorization of the modulus
class PublicBlumBlumShub : public RandomNumberGenerator,
						   public StreamTransformation
{
public:
	PublicBlumBlumShub(const Integer &n, const Integer &seed);

	unsigned int GenerateBit();
	byte GenerateByte();
	void GenerateBlock(byte *output, size_t size);
	void ProcessData(byte *outString, const byte *inString, size_t length);

	bool IsSelfInverting() const {return true;}
	bool IsForwardTransformation() const {return true;}

protected:
	ModularArithmetic modn;
	word maxBits, bitsLeft;
	Integer current;

	friend class BlumGoldwasserPublicKey;
	friend class BlumGoldwasserPrivateKey;
};

//! BlumBlumShub with factorization of the modulus
class BlumBlumShub : public PublicBlumBlumShub
{
public:
	// Make sure p and q are both primes congruent to 3 mod 4 and at least 512 bits long,
	// seed is the secret key and should be about as big as p*q
	BlumBlumShub(const Integer &p, const Integer &q, const Integer &seed);
	
	bool IsRandomAccess() const {return true;}
	void Seek(lword index);

protected:
	const Integer p, q;
	const Integer x0;
};

NAMESPACE_END

#endif
