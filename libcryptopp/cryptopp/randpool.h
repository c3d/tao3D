#ifndef CRYPTOPP_RANDPOOL_H
#define CRYPTOPP_RANDPOOL_H
// *****************************************************************************
// randpool.h                                                      Tao3D project
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

#include "cryptlib.h"
#include "filters.h"

NAMESPACE_BEGIN(CryptoPP)

//! Randomness Pool
/*! This class can be used to generate cryptographic quality
	pseudorandom bytes after seeding the pool with IncorporateEntropy() */
class CRYPTOPP_DLL RandomPool : public RandomNumberGenerator, public NotCopyable
{
public:
	RandomPool();

	bool CanIncorporateEntropy() const {return true;}
	void IncorporateEntropy(const byte *input, size_t length);
	void GenerateIntoBufferedTransformation(BufferedTransformation &target, const std::string &channel, lword size);

	// for backwards compatibility. use RandomNumberSource, RandomNumberStore, and RandomNumberSink for other BufferTransformation functionality
	void Put(const byte *input, size_t length) {IncorporateEntropy(input, length);}

private:
	FixedSizeSecBlock<byte, 32> m_key;
	FixedSizeSecBlock<byte, 16> m_seed;
	member_ptr<BlockCipher> m_pCipher;
	bool m_keySet;
};

NAMESPACE_END

#endif
