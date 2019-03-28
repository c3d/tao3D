#ifndef CRYPTOPP_RIPEMD_H
#define CRYPTOPP_RIPEMD_H
// *****************************************************************************
// ripemd.h                                                        Tao3D project
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

//! <a href="http://www.weidai.com/scan-mirror/md.html#RIPEMD-160">RIPEMD-160</a>
/*! Digest Length = 160 bits */
class RIPEMD160 : public IteratedHashWithStaticTransform<word32, LittleEndian, 64, 20, RIPEMD160>
{
public:
	static void InitState(HashWordType *state);
	static void Transform(word32 *digest, const word32 *data);
	static const char * StaticAlgorithmName() {return "RIPEMD-160";}
};

/*! Digest Length = 320 bits, Security is similar to RIPEMD-160 */
class RIPEMD320 : public IteratedHashWithStaticTransform<word32, LittleEndian, 64, 40, RIPEMD320>
{
public:
	static void InitState(HashWordType *state);
	static void Transform(word32 *digest, const word32 *data);
	static const char * StaticAlgorithmName() {return "RIPEMD-320";}
};

/*! \warning RIPEMD-128 is considered insecure, and should not be used
	unless you absolutely need it for compatibility. */
class RIPEMD128 : public IteratedHashWithStaticTransform<word32, LittleEndian, 64, 16, RIPEMD128>
{
public:
	static void InitState(HashWordType *state);
	static void Transform(word32 *digest, const word32 *data);
	static const char * StaticAlgorithmName() {return "RIPEMD-128";}
};

/*! \warning RIPEMD-256 is considered insecure, and should not be used
	unless you absolutely need it for compatibility. */
class RIPEMD256 : public IteratedHashWithStaticTransform<word32, LittleEndian, 64, 32, RIPEMD256>
{
public:
	static void InitState(HashWordType *state);
	static void Transform(word32 *digest, const word32 *data);
	static const char * StaticAlgorithmName() {return "RIPEMD-256";}
};

NAMESPACE_END

#endif
