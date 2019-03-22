#ifndef CRYPTOPP_DSA_H
#define CRYPTOPP_DSA_H
// *****************************************************************************
// dsa.h                                                           Tao3D project
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

/** \file
*/

#include "gfpcrypt.h"

NAMESPACE_BEGIN(CryptoPP)

/*! The DSA signature format used by Crypto++ is as defined by IEEE P1363.
  Java uses the DER format, and OpenPGP uses the OpenPGP format. */
enum DSASignatureFormat {DSA_P1363, DSA_DER, DSA_OPENPGP};
/** This function converts between these formats, and returns length of signature in the target format.
	If toFormat == DSA_P1363, bufferSize must equal publicKey.SignatureLength() */
size_t DSAConvertSignatureFormat(byte *buffer, size_t bufferSize, DSASignatureFormat toFormat,
	const byte *signature, size_t signatureLen, DSASignatureFormat fromFormat);

#ifdef CRYPTOPP_MAINTAIN_BACKWARDS_COMPATIBILITY

typedef DSA::Signer DSAPrivateKey;
typedef DSA::Verifier DSAPublicKey;

const int MIN_DSA_PRIME_LENGTH = DSA::MIN_PRIME_LENGTH;
const int MAX_DSA_PRIME_LENGTH = DSA::MAX_PRIME_LENGTH;
const int DSA_PRIME_LENGTH_MULTIPLE = DSA::PRIME_LENGTH_MULTIPLE;

inline bool GenerateDSAPrimes(const byte *seed, size_t seedLength, int &counter, Integer &p, unsigned int primeLength, Integer &q)
	{return DSA::GeneratePrimes(seed, seedLength, counter, p, primeLength, q);}

#endif

NAMESPACE_END

#endif
