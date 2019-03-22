// *****************************************************************************
// dsa.cpp                                                         Tao3D project
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
// (C) 2013, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
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
// dsa.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"

#ifndef CRYPTOPP_IMPORTS

#include "dsa.h"
#include "nbtheory.h"

NAMESPACE_BEGIN(CryptoPP)

size_t DSAConvertSignatureFormat(byte *buffer, size_t bufferSize, DSASignatureFormat toFormat, const byte *signature, size_t signatureLen, DSASignatureFormat fromFormat)
{
	Integer r, s;
	StringStore store(signature, signatureLen);
	ArraySink sink(buffer, bufferSize);

	switch (fromFormat)
	{
	case DSA_P1363:
		r.Decode(store, signatureLen/2);
		s.Decode(store, signatureLen/2);
		break;
	case DSA_DER:
	{
		BERSequenceDecoder seq(store);
		r.BERDecode(seq);
		s.BERDecode(seq);
		seq.MessageEnd();
		break;
	}
	case DSA_OPENPGP:
		r.OpenPGPDecode(store);
		s.OpenPGPDecode(store);
		break;
	}

	switch (toFormat)
	{
	case DSA_P1363:
		r.Encode(sink, bufferSize/2);
		s.Encode(sink, bufferSize/2);
		break;
	case DSA_DER:
	{
		DERSequenceEncoder seq(sink);
		r.DEREncode(seq);
		s.DEREncode(seq);
		seq.MessageEnd();
		break;
	}
	case DSA_OPENPGP:
		r.OpenPGPEncode(sink);
		s.OpenPGPEncode(sink);
		break;
	}

	return (size_t)sink.TotalPutLength();
}

NAMESPACE_END

#endif
