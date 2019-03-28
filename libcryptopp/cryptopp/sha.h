#ifndef CRYPTOPP_SHA_H
#define CRYPTOPP_SHA_H
// *****************************************************************************
// sha.h                                                           Tao3D project
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

#include "iterhash.h"

NAMESPACE_BEGIN(CryptoPP)

/// <a href="http://www.weidai.com/scan-mirror/md.html#SHA-1">SHA-1</a>
class CRYPTOPP_DLL SHA1 : public IteratedHashWithStaticTransform<word32, BigEndian, 64, 20, SHA1>
{
public:
	static void CRYPTOPP_API InitState(HashWordType *state);
	static void CRYPTOPP_API Transform(word32 *digest, const word32 *data);
	static const char * CRYPTOPP_API StaticAlgorithmName() {return "SHA-1";}
};

typedef SHA1 SHA;	// for backwards compatibility

//! implements the SHA-256 standard
class CRYPTOPP_DLL SHA256 : public IteratedHashWithStaticTransform<word32, BigEndian, 64, 32, SHA256, 32, true>
{
public:
#if defined(CRYPTOPP_X86_ASM_AVAILABLE) || defined(CRYPTOPP_X64_MASM_AVAILABLE)
	size_t HashMultipleBlocks(const word32 *input, size_t length);
#endif
	static void CRYPTOPP_API InitState(HashWordType *state);
	static void CRYPTOPP_API Transform(word32 *digest, const word32 *data);
	static const char * CRYPTOPP_API StaticAlgorithmName() {return "SHA-256";}
};

//! implements the SHA-224 standard
class CRYPTOPP_DLL SHA224 : public IteratedHashWithStaticTransform<word32, BigEndian, 64, 32, SHA224, 28, true>
{
public:
#if defined(CRYPTOPP_X86_ASM_AVAILABLE) || defined(CRYPTOPP_X64_MASM_AVAILABLE)
	size_t HashMultipleBlocks(const word32 *input, size_t length);
#endif
	static void CRYPTOPP_API InitState(HashWordType *state);
	static void CRYPTOPP_API Transform(word32 *digest, const word32 *data) {SHA256::Transform(digest, data);}
	static const char * CRYPTOPP_API StaticAlgorithmName() {return "SHA-224";}
};

//! implements the SHA-512 standard
class CRYPTOPP_DLL SHA512 : public IteratedHashWithStaticTransform<word64, BigEndian, 128, 64, SHA512, 64, CRYPTOPP_BOOL_X86>
{
public:
	static void CRYPTOPP_API InitState(HashWordType *state);
	static void CRYPTOPP_API Transform(word64 *digest, const word64 *data);
	static const char * CRYPTOPP_API StaticAlgorithmName() {return "SHA-512";}
};

//! implements the SHA-384 standard
class CRYPTOPP_DLL SHA384 : public IteratedHashWithStaticTransform<word64, BigEndian, 128, 64, SHA384, 48, CRYPTOPP_BOOL_X86>
{
public:
	static void CRYPTOPP_API InitState(HashWordType *state);
	static void CRYPTOPP_API Transform(word64 *digest, const word64 *data) {SHA512::Transform(digest, data);}
	static const char * CRYPTOPP_API StaticAlgorithmName() {return "SHA-384";}
};

NAMESPACE_END

#endif
