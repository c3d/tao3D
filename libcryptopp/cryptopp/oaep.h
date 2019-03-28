#ifndef CRYPTOPP_OAEP_H
#define CRYPTOPP_OAEP_H
// *****************************************************************************
// oaep.h                                                          Tao3D project
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

#include "pubkey.h"
#include "sha.h"

NAMESPACE_BEGIN(CryptoPP)

//! _
class CRYPTOPP_DLL OAEP_Base : public PK_EncryptionMessageEncodingMethod
{
public:
	bool ParameterSupported(const char *name) const {return strcmp(name, Name::EncodingParameters()) == 0;}
	size_t MaxUnpaddedLength(size_t paddedLength) const;
	void Pad(RandomNumberGenerator &rng, const byte *raw, size_t inputLength, byte *padded, size_t paddedLength, const NameValuePairs &parameters) const;
	DecodingResult Unpad(const byte *padded, size_t paddedLength, byte *raw, const NameValuePairs &parameters) const;

protected:
	virtual unsigned int DigestSize() const =0;
	virtual HashTransformation * NewHash() const =0;
	virtual MaskGeneratingFunction * NewMGF() const =0;
};

//! <a href="http://www.weidai.com/scan-mirror/ca.html#cem_OAEP-MGF1">EME-OAEP</a>, for use with classes derived from TF_ES
template <class H, class MGF=P1363_MGF1>
class OAEP : public OAEP_Base, public EncryptionStandard
{
public:
	static std::string CRYPTOPP_API StaticAlgorithmName() {return std::string("OAEP-") + MGF::StaticAlgorithmName() + "(" + H::StaticAlgorithmName() + ")";}
	typedef OAEP<H, MGF> EncryptionMessageEncodingMethod;

protected:
	unsigned int DigestSize() const {return H::DIGESTSIZE;}
	HashTransformation * NewHash() const {return new H;}
	MaskGeneratingFunction * NewMGF() const {return new MGF;}
};

CRYPTOPP_DLL_TEMPLATE_CLASS OAEP<SHA>;

NAMESPACE_END

#endif
