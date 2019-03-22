// *****************************************************************************
// rng.h                                                           Tao3D project
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
// rng.h - misc RNG related classes, see also osrng.h, randpool.h

#ifndef CRYPTOPP_RNG_H
#define CRYPTOPP_RNG_H

#include "cryptlib.h"
#include "filters.h"

NAMESPACE_BEGIN(CryptoPP)

//! linear congruential generator
/*! originally by William S. England, do not use for cryptographic purposes */
class LC_RNG : public RandomNumberGenerator
{
public:
	LC_RNG(word32 init_seed)
		: seed(init_seed) {}

	void GenerateBlock(byte *output, size_t size);

	word32 GetSeed() {return seed;}

private:
	word32 seed;

	static const word32 m;
	static const word32 q;
	static const word16 a;
	static const word16 r;
};

//! RNG derived from ANSI X9.17 Appendix C

class CRYPTOPP_DLL X917RNG : public RandomNumberGenerator, public NotCopyable
{
public:
	// cipher will be deleted by destructor, deterministicTimeVector = 0 means obtain time vector from system
	X917RNG(BlockTransformation *cipher, const byte *seed, const byte *deterministicTimeVector = 0);

	void GenerateIntoBufferedTransformation(BufferedTransformation &target, const std::string &channel, lword size);

private:
	member_ptr<BlockTransformation> cipher;
	unsigned int S;			// blocksize of cipher
	SecByteBlock dtbuf; 	// buffer for enciphered timestamp
	SecByteBlock randseed, m_lastBlock, m_deterministicTimeVector;
};

/** This class implements Maurer's Universal Statistical Test for Random Bit Generators
    it is intended for measuring the randomness of *PHYSICAL* RNGs.
    For more details see his paper in Journal of Cryptology, 1992. */

class MaurerRandomnessTest : public Bufferless<Sink>
{
public:
	MaurerRandomnessTest();

	size_t Put2(const byte *inString, size_t length, int messageEnd, bool blocking);

	// BytesNeeded() returns how many more bytes of input is needed by the test
	// GetTestValue() should not be called before BytesNeeded()==0
	unsigned int BytesNeeded() const {return n >= (Q+K) ? 0 : Q+K-n;}

	// returns a number between 0.0 and 1.0, describing the quality of the
	// random numbers entered
	double GetTestValue() const;

private:
	enum {L=8, V=256, Q=2000, K=2000};
	double sum;
	unsigned int n;
	unsigned int tab[V];
};

NAMESPACE_END

#endif
