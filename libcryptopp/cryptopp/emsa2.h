#ifndef CRYPTOPP_EMSA2_H
#define CRYPTOPP_EMSA2_H
// *****************************************************************************
// emsa2.h                                                         Tao3D project
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
	This file contains various padding schemes for public key algorithms.
*/

#include "cryptlib.h"
#include "pubkey.h"

#ifdef CRYPTOPP_IS_DLL
#include "sha.h"
#endif

NAMESPACE_BEGIN(CryptoPP)

template <class H> class EMSA2HashId
{
public:
	static const byte id;
};

template <class BASE>
class EMSA2HashIdLookup : public BASE
{
public:
	struct HashIdentifierLookup
	{
		template <class H> struct HashIdentifierLookup2
		{
			static HashIdentifier Lookup()
			{
				return HashIdentifier(&EMSA2HashId<H>::id, 1);
			}
		};
	};
};

// EMSA2HashId can be instantiated with the following classes.
class SHA1;
class RIPEMD160;
class RIPEMD128;
class SHA256;
class SHA384;
class SHA512;
class Whirlpool;
class SHA224;
// end of list

#ifdef CRYPTOPP_IS_DLL
CRYPTOPP_DLL_TEMPLATE_CLASS EMSA2HashId<SHA1>;
CRYPTOPP_DLL_TEMPLATE_CLASS EMSA2HashId<SHA224>;
CRYPTOPP_DLL_TEMPLATE_CLASS EMSA2HashId<SHA256>;
CRYPTOPP_DLL_TEMPLATE_CLASS EMSA2HashId<SHA384>;
CRYPTOPP_DLL_TEMPLATE_CLASS EMSA2HashId<SHA512>;
#endif

//! _
class CRYPTOPP_DLL EMSA2Pad : public EMSA2HashIdLookup<PK_DeterministicSignatureMessageEncodingMethod>
{
public:
	static const char * CRYPTOPP_API StaticAlgorithmName() {return "EMSA2";}
	
	size_t MinRepresentativeBitLength(size_t hashIdentifierLength, size_t digestLength) const
		{return 8*digestLength + 31;}

	void ComputeMessageRepresentative(RandomNumberGenerator &rng,
		const byte *recoverableMessage, size_t recoverableMessageLength,
		HashTransformation &hash, HashIdentifier hashIdentifier, bool messageEmpty,
		byte *representative, size_t representativeBitLength) const;
};

//! EMSA2, for use with RWSS and RSA_ISO
/*! Only the following hash functions are supported by this signature standard:
	\dontinclude emsa2.h
	\skip EMSA2HashId can be instantiated
	\until end of list
*/
struct P1363_EMSA2 : public SignatureStandard
{
	typedef EMSA2Pad SignatureMessageEncodingMethod;
};

NAMESPACE_END

#endif
