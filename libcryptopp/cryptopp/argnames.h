#ifndef CRYPTOPP_ARGNAMES_H
#define CRYPTOPP_ARGNAMES_H
// *****************************************************************************
// argnames.h                                                      Tao3D project
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

DOCUMENTED_NAMESPACE_BEGIN(Name)

#define CRYPTOPP_DEFINE_NAME_STRING(name)	inline const char *name() {return #name;}

CRYPTOPP_DEFINE_NAME_STRING(ValueNames)			//!< string, a list of value names with a semicolon (';') after each name
CRYPTOPP_DEFINE_NAME_STRING(Version)			//!< int
CRYPTOPP_DEFINE_NAME_STRING(Seed)				//!< ConstByteArrayParameter
CRYPTOPP_DEFINE_NAME_STRING(Key)				//!< ConstByteArrayParameter
CRYPTOPP_DEFINE_NAME_STRING(IV)					//!< ConstByteArrayParameter, also accepts const byte * for backwards compatibility
CRYPTOPP_DEFINE_NAME_STRING(StolenIV)			//!< byte *
CRYPTOPP_DEFINE_NAME_STRING(Rounds)				//!< int
CRYPTOPP_DEFINE_NAME_STRING(FeedbackSize)		//!< int
CRYPTOPP_DEFINE_NAME_STRING(WordSize)			//!< int, in bytes
CRYPTOPP_DEFINE_NAME_STRING(BlockSize)			//!< int, in bytes
CRYPTOPP_DEFINE_NAME_STRING(EffectiveKeyLength)	//!< int, in bits
CRYPTOPP_DEFINE_NAME_STRING(KeySize)			//!< int, in bits
CRYPTOPP_DEFINE_NAME_STRING(ModulusSize)		//!< int, in bits
CRYPTOPP_DEFINE_NAME_STRING(SubgroupOrderSize)	//!< int, in bits
CRYPTOPP_DEFINE_NAME_STRING(PrivateExponentSize)//!< int, in bits
CRYPTOPP_DEFINE_NAME_STRING(Modulus)			//!< Integer
CRYPTOPP_DEFINE_NAME_STRING(PublicExponent)		//!< Integer
CRYPTOPP_DEFINE_NAME_STRING(PrivateExponent)	//!< Integer
CRYPTOPP_DEFINE_NAME_STRING(PublicElement)		//!< Integer
CRYPTOPP_DEFINE_NAME_STRING(SubgroupOrder)		//!< Integer
CRYPTOPP_DEFINE_NAME_STRING(Cofactor)			//!< Integer
CRYPTOPP_DEFINE_NAME_STRING(SubgroupGenerator)	//!< Integer, ECP::Point, or EC2N::Point
CRYPTOPP_DEFINE_NAME_STRING(Curve)				//!< ECP or EC2N
CRYPTOPP_DEFINE_NAME_STRING(GroupOID)			//!< OID
CRYPTOPP_DEFINE_NAME_STRING(PointerToPrimeSelector)		//!< const PrimeSelector *
CRYPTOPP_DEFINE_NAME_STRING(Prime1)				//!< Integer
CRYPTOPP_DEFINE_NAME_STRING(Prime2)				//!< Integer
CRYPTOPP_DEFINE_NAME_STRING(ModPrime1PrivateExponent)	//!< Integer
CRYPTOPP_DEFINE_NAME_STRING(ModPrime2PrivateExponent)	//!< Integer
CRYPTOPP_DEFINE_NAME_STRING(MultiplicativeInverseOfPrime2ModPrime1)	//!< Integer
CRYPTOPP_DEFINE_NAME_STRING(QuadraticResidueModPrime1)	//!< Integer
CRYPTOPP_DEFINE_NAME_STRING(QuadraticResidueModPrime2)	//!< Integer
CRYPTOPP_DEFINE_NAME_STRING(PutMessage)			//!< bool
CRYPTOPP_DEFINE_NAME_STRING(TruncatedDigestSize)	//!< int
CRYPTOPP_DEFINE_NAME_STRING(BlockPaddingScheme)	//!< StreamTransformationFilter::BlockPaddingScheme
CRYPTOPP_DEFINE_NAME_STRING(HashVerificationFilterFlags)		//!< word32
CRYPTOPP_DEFINE_NAME_STRING(AuthenticatedDecryptionFilterFlags)	//!< word32
CRYPTOPP_DEFINE_NAME_STRING(SignatureVerificationFilterFlags)	//!< word32
CRYPTOPP_DEFINE_NAME_STRING(InputBuffer)		//!< ConstByteArrayParameter
CRYPTOPP_DEFINE_NAME_STRING(OutputBuffer)		//!< ByteArrayParameter
CRYPTOPP_DEFINE_NAME_STRING(InputFileName)		//!< const char *
CRYPTOPP_DEFINE_NAME_STRING(InputFileNameWide)	//!< const wchar_t *
CRYPTOPP_DEFINE_NAME_STRING(InputStreamPointer)	//!< std::istream *
CRYPTOPP_DEFINE_NAME_STRING(InputBinaryMode)	//!< bool
CRYPTOPP_DEFINE_NAME_STRING(OutputFileName)		//!< const char *
CRYPTOPP_DEFINE_NAME_STRING(OutputFileNameWide)	//!< const wchar_t *
CRYPTOPP_DEFINE_NAME_STRING(OutputStreamPointer)	//!< std::ostream *
CRYPTOPP_DEFINE_NAME_STRING(OutputBinaryMode)	//!< bool
CRYPTOPP_DEFINE_NAME_STRING(EncodingParameters)	//!< ConstByteArrayParameter
CRYPTOPP_DEFINE_NAME_STRING(KeyDerivationParameters)	//!< ConstByteArrayParameter
CRYPTOPP_DEFINE_NAME_STRING(Separator)			//< ConstByteArrayParameter
CRYPTOPP_DEFINE_NAME_STRING(Terminator)			//< ConstByteArrayParameter
CRYPTOPP_DEFINE_NAME_STRING(Uppercase)			//< bool
CRYPTOPP_DEFINE_NAME_STRING(GroupSize)			//< int
CRYPTOPP_DEFINE_NAME_STRING(Pad)				//< bool
CRYPTOPP_DEFINE_NAME_STRING(PaddingByte)		//< byte
CRYPTOPP_DEFINE_NAME_STRING(Log2Base)			//< int
CRYPTOPP_DEFINE_NAME_STRING(EncodingLookupArray)	//< const byte *
CRYPTOPP_DEFINE_NAME_STRING(DecodingLookupArray)	//< const byte *
CRYPTOPP_DEFINE_NAME_STRING(InsertLineBreaks)	//< bool
CRYPTOPP_DEFINE_NAME_STRING(MaxLineLength)		//< int
CRYPTOPP_DEFINE_NAME_STRING(DigestSize)			//!< int, in bytes
CRYPTOPP_DEFINE_NAME_STRING(L1KeyLength)		//!< int, in bytes
CRYPTOPP_DEFINE_NAME_STRING(TableSize)			//!< int, in bytes

DOCUMENTED_NAMESPACE_END

NAMESPACE_END

#endif
