#ifndef CRYPTOPP_BASECODE_H
#define CRYPTOPP_BASECODE_H
// *****************************************************************************
// basecode.h                                                      Tao3D project
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

#include "filters.h"
#include "algparam.h"
#include "argnames.h"

NAMESPACE_BEGIN(CryptoPP)

//! base n encoder, where n is a power of 2
class CRYPTOPP_DLL BaseN_Encoder : public Unflushable<Filter>
{
public:
	BaseN_Encoder(BufferedTransformation *attachment=NULL)
		{Detach(attachment);}

	BaseN_Encoder(const byte *alphabet, int log2base, BufferedTransformation *attachment=NULL, int padding=-1)
	{
		Detach(attachment);
		IsolatedInitialize(MakeParameters(Name::EncodingLookupArray(), alphabet)
			(Name::Log2Base(), log2base)
			(Name::Pad(), padding != -1)
			(Name::PaddingByte(), byte(padding)));
	}

	void IsolatedInitialize(const NameValuePairs &parameters);
	size_t Put2(const byte *begin, size_t length, int messageEnd, bool blocking);

private:
	const byte *m_alphabet;
	int m_padding, m_bitsPerChar, m_outputBlockSize;
	int m_bytePos, m_bitPos;
	SecByteBlock m_outBuf;
};

//! base n decoder, where n is a power of 2
class CRYPTOPP_DLL BaseN_Decoder : public Unflushable<Filter>
{
public:
	BaseN_Decoder(BufferedTransformation *attachment=NULL)
		{Detach(attachment);}

	BaseN_Decoder(const int *lookup, int log2base, BufferedTransformation *attachment=NULL)
	{
		Detach(attachment);
		IsolatedInitialize(MakeParameters(Name::DecodingLookupArray(), lookup)(Name::Log2Base(), log2base));
	}

	void IsolatedInitialize(const NameValuePairs &parameters);
	size_t Put2(const byte *begin, size_t length, int messageEnd, bool blocking);

	static void CRYPTOPP_API InitializeDecodingLookupArray(int *lookup, const byte *alphabet, unsigned int base, bool caseInsensitive);

private:
	const int *m_lookup;
	int m_padding, m_bitsPerChar, m_outputBlockSize;
	int m_bytePos, m_bitPos;
	SecByteBlock m_outBuf;
};

//! filter that breaks input stream into groups of fixed size
class CRYPTOPP_DLL Grouper : public Bufferless<Filter>
{
public:
	Grouper(BufferedTransformation *attachment=NULL)
		{Detach(attachment);}

	Grouper(int groupSize, const std::string &separator, const std::string &terminator, BufferedTransformation *attachment=NULL)
	{
		Detach(attachment);
		IsolatedInitialize(MakeParameters(Name::GroupSize(), groupSize)
			(Name::Separator(), ConstByteArrayParameter(separator))
			(Name::Terminator(), ConstByteArrayParameter(terminator)));
	}

	void IsolatedInitialize(const NameValuePairs &parameters);
	size_t Put2(const byte *begin, size_t length, int messageEnd, bool blocking);

private:
	SecByteBlock m_separator, m_terminator;
	size_t m_groupSize, m_counter;
};

NAMESPACE_END

#endif
