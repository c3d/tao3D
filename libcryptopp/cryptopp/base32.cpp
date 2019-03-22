// *****************************************************************************
// base32.cpp                                                      Tao3D project
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
// base32.cpp - written and placed in the public domain by Frank Palazzolo, based on hex.cpp by Wei Dai

#include "pch.h"
#include "base32.h"

NAMESPACE_BEGIN(CryptoPP)

static const byte s_vecUpper[] = "ABCDEFGHIJKMNPQRSTUVWXYZ23456789";
static const byte s_vecLower[] = "abcdefghijkmnpqrstuvwxyz23456789";

void Base32Encoder::IsolatedInitialize(const NameValuePairs &parameters)
{
	bool uppercase = parameters.GetValueWithDefault(Name::Uppercase(), true);
	m_filter->Initialize(CombinedNameValuePairs(
		parameters,
		MakeParameters(Name::EncodingLookupArray(), uppercase ? &s_vecUpper[0] : &s_vecLower[0], false)(Name::Log2Base(), 5, true)));
}

void Base32Decoder::IsolatedInitialize(const NameValuePairs &parameters)
{
	BaseN_Decoder::Initialize(CombinedNameValuePairs(
		parameters,
		MakeParameters(Name::DecodingLookupArray(), GetDefaultDecodingLookupArray(), false)(Name::Log2Base(), 5, true)));
}

const int *Base32Decoder::GetDefaultDecodingLookupArray()
{
	static volatile bool s_initialized = false;
	static int s_array[256];

	if (!s_initialized)
	{
		InitializeDecodingLookupArray(s_array, s_vecUpper, 32, true);
		s_initialized = true;
	}
	return s_array;
}

NAMESPACE_END
