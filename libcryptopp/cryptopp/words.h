#ifndef CRYPTOPP_WORDS_H
#define CRYPTOPP_WORDS_H
// *****************************************************************************
// words.h                                                         Tao3D project
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

#include "misc.h"

NAMESPACE_BEGIN(CryptoPP)

inline size_t CountWords(const word *X, size_t N)
{
	while (N && X[N-1]==0)
		N--;
	return N;
}

inline void SetWords(word *r, word a, size_t n)
{
	for (size_t i=0; i<n; i++)
		r[i] = a;
}

inline void CopyWords(word *r, const word *a, size_t n)
{
	if (r != a)
		memcpy(r, a, n*WORD_SIZE);
}

inline void XorWords(word *r, const word *a, const word *b, size_t n)
{
	for (size_t i=0; i<n; i++)
		r[i] = a[i] ^ b[i];
}

inline void XorWords(word *r, const word *a, size_t n)
{
	for (size_t i=0; i<n; i++)
		r[i] ^= a[i];
}

inline void AndWords(word *r, const word *a, const word *b, size_t n)
{
	for (size_t i=0; i<n; i++)
		r[i] = a[i] & b[i];
}

inline void AndWords(word *r, const word *a, size_t n)
{
	for (size_t i=0; i<n; i++)
		r[i] &= a[i];
}

inline word ShiftWordsLeftByBits(word *r, size_t n, unsigned int shiftBits)
{
	assert (shiftBits<WORD_BITS);
	word u, carry=0;
	if (shiftBits)
		for (size_t i=0; i<n; i++)
		{
			u = r[i];
			r[i] = (u << shiftBits) | carry;
			carry = u >> (WORD_BITS-shiftBits);
		}
	return carry;
}

inline word ShiftWordsRightByBits(word *r, size_t n, unsigned int shiftBits)
{
	assert (shiftBits<WORD_BITS);
	word u, carry=0;
	if (shiftBits)
		for (size_t i=n; i>0; i--)
		{
			u = r[i-1];
			r[i-1] = (u >> shiftBits) | carry;
			carry = u << (WORD_BITS-shiftBits);
		}
	return carry;
}

inline void ShiftWordsLeftByWords(word *r, size_t n, size_t shiftWords)
{
	shiftWords = STDMIN(shiftWords, n);
	if (shiftWords)
	{
		for (size_t i=n-1; i>=shiftWords; i--)
			r[i] = r[i-shiftWords];
		SetWords(r, 0, shiftWords);
	}
}

inline void ShiftWordsRightByWords(word *r, size_t n, size_t shiftWords)
{
	shiftWords = STDMIN(shiftWords, n);
	if (shiftWords)
	{
		for (size_t i=0; i+shiftWords<n; i++)
			r[i] = r[i+shiftWords];
		SetWords(r+n-shiftWords, 0, shiftWords);
	}
}

NAMESPACE_END

#endif
