// *****************************************************************************
// rng.cpp                                                         Tao3D project
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
// rng.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"

#include "rng.h"
#include "fips140.h"

#include <time.h>
#include <math.h>

NAMESPACE_BEGIN(CryptoPP)

// linear congruential generator
// originally by William S. England

// do not use for cryptographic purposes

/*
** Original_numbers are the original published m and q in the
** ACM article above.  John Burton has furnished numbers for
** a reportedly better generator.  The new numbers are now
** used in this program by default.
*/

#ifndef LCRNG_ORIGINAL_NUMBERS
const word32 LC_RNG::m=2147483647L;
const word32 LC_RNG::q=44488L;

const word16 LC_RNG::a=(unsigned int)48271L;
const word16 LC_RNG::r=3399;
#else
const word32 LC_RNG::m=2147483647L;
const word32 LC_RNG::q=127773L;

const word16 LC_RNG::a=16807;
const word16 LC_RNG::r=2836;
#endif

void LC_RNG::GenerateBlock(byte *output, size_t size)
{
	while (size--)
	{
		word32 hi = seed/q;
		word32 lo = seed%q;

		long test = a*lo - r*hi;

		if (test > 0)
			seed = test;
		else
			seed = test+ m;

		*output++ = (GETBYTE(seed, 0) ^ GETBYTE(seed, 1) ^ GETBYTE(seed, 2) ^ GETBYTE(seed, 3));
	}
}

// ********************************************************

#ifndef CRYPTOPP_IMPORTS

X917RNG::X917RNG(BlockTransformation *c, const byte *seed, const byte *deterministicTimeVector)
	: cipher(c),
	  S(cipher->BlockSize()),
	  dtbuf(S),
	  randseed(seed, S),
	  m_lastBlock(S),
	  m_deterministicTimeVector(deterministicTimeVector, deterministicTimeVector ? S : 0)
{
	if (!deterministicTimeVector)
	{
		time_t tstamp1 = time(0);
		xorbuf(dtbuf, (byte *)&tstamp1, UnsignedMin(sizeof(tstamp1), S));
		cipher->ProcessBlock(dtbuf);
		clock_t tstamp2 = clock();
		xorbuf(dtbuf, (byte *)&tstamp2, UnsignedMin(sizeof(tstamp2), S));
		cipher->ProcessBlock(dtbuf);
	}

	// for FIPS 140-2
	GenerateBlock(m_lastBlock, S);
}

void X917RNG::GenerateIntoBufferedTransformation(BufferedTransformation &target, const std::string &channel, lword size)
{
	while (size > 0)
	{
		// calculate new enciphered timestamp
		if (m_deterministicTimeVector.size())
		{
			cipher->ProcessBlock(m_deterministicTimeVector, dtbuf);
			IncrementCounterByOne(m_deterministicTimeVector, S);
		}
		else
		{
			clock_t c = clock();
			xorbuf(dtbuf, (byte *)&c, UnsignedMin(sizeof(c), S));
			time_t t = time(NULL);
			xorbuf(dtbuf+S-UnsignedMin(sizeof(t), S), (byte *)&t, UnsignedMin(sizeof(t), S));
			cipher->ProcessBlock(dtbuf);
		}

		// combine enciphered timestamp with seed
		xorbuf(randseed, dtbuf, S);

		// generate a new block of random bytes
		cipher->ProcessBlock(randseed);
		if (memcmp(m_lastBlock, randseed, S) == 0)
			throw SelfTestFailure("X917RNG: Continuous random number generator test failed.");

		// output random bytes
		size_t len = UnsignedMin(S, size);
		target.ChannelPut(channel, randseed, len);
		size -= len;

		// compute new seed vector
		memcpy(m_lastBlock, randseed, S);
		xorbuf(randseed, dtbuf, S);
		cipher->ProcessBlock(randseed);
	}
}

#endif

MaurerRandomnessTest::MaurerRandomnessTest()
	: sum(0.0), n(0)
{
	for (unsigned i=0; i<V; i++)
		tab[i] = 0;
}

size_t MaurerRandomnessTest::Put2(const byte *inString, size_t length, int messageEnd, bool blocking)
{
	while (length--)
	{
		byte inByte = *inString++;
		if (n >= Q)
			sum += log(double(n - tab[inByte]));
		tab[inByte] = n;
		n++;
	}
	return 0;
}

double MaurerRandomnessTest::GetTestValue() const
{
	if (BytesNeeded() > 0)
		throw Exception(Exception::OTHER_ERROR, "MaurerRandomnessTest: " + IntToString(BytesNeeded()) + " more bytes of input needed");

	double fTu = (sum/(n-Q))/log(2.0);	// this is the test value defined by Maurer

	double value = fTu * 0.1392;		// arbitrarily normalize it to
	return value > 1.0 ? 1.0 : value;	// a number between 0 and 1
}

NAMESPACE_END
