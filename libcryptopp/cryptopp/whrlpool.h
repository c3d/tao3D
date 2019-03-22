#ifndef CRYPTOPP_WHIRLPOOL_H
#define CRYPTOPP_WHIRLPOOL_H
// *****************************************************************************
// whrlpool.h                                                      Tao3D project
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

#include "config.h"
#include "iterhash.h"

NAMESPACE_BEGIN(CryptoPP)

//! <a href="http://www.cryptolounge.org/wiki/Whirlpool">Whirlpool</a>
class Whirlpool : public IteratedHashWithStaticTransform<word64, BigEndian, 64, 64, Whirlpool>
{
public:
	static void InitState(HashWordType *state);
	static void Transform(word64 *digest, const word64 *data);
	void TruncatedFinal(byte *hash, size_t size);
	static const char * StaticAlgorithmName() {return "Whirlpool";}
};

NAMESPACE_END

#endif
