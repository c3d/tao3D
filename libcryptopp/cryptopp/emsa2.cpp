// *****************************************************************************
// emsa2.cpp                                                       Tao3D project
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
// emsa2.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "emsa2.h"

#ifndef CRYPTOPP_IMPORTS

NAMESPACE_BEGIN(CryptoPP)

void EMSA2Pad::ComputeMessageRepresentative(RandomNumberGenerator &rng,
	const byte *recoverableMessage, size_t recoverableMessageLength,
	HashTransformation &hash, HashIdentifier hashIdentifier, bool messageEmpty,
	byte *representative, size_t representativeBitLength) const
{
	assert(representativeBitLength >= MinRepresentativeBitLength(hashIdentifier.second, hash.DigestSize()));

	if (representativeBitLength % 8 != 7)
		throw PK_SignatureScheme::InvalidKeyLength("EMSA2: EMSA2 requires a key length that is a multiple of 8");

	size_t digestSize = hash.DigestSize();
	size_t representativeByteLength = BitsToBytes(representativeBitLength);

	representative[0] = messageEmpty ? 0x4b : 0x6b;
	memset(representative+1, 0xbb, representativeByteLength-digestSize-4);	// pad with 0xbb
	byte *afterP2 = representative+representativeByteLength-digestSize-3;
	afterP2[0] = 0xba;
	hash.Final(afterP2+1);
	representative[representativeByteLength-2] = *hashIdentifier.first;
	representative[representativeByteLength-1] = 0xcc;
}

NAMESPACE_END

#endif
