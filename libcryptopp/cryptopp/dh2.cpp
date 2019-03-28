// *****************************************************************************
// dh2.cpp                                                         Tao3D project
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
// dh2.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "dh2.h"

NAMESPACE_BEGIN(CryptoPP)

void DH2_TestInstantiations()
{
	DH2 dh(*(SimpleKeyAgreementDomain*)NULL);
}

bool DH2::Agree(byte *agreedValue,
		const byte *staticSecretKey, const byte *ephemeralSecretKey,
		const byte *staticOtherPublicKey, const byte *ephemeralOtherPublicKey,
		bool validateStaticOtherPublicKey) const
{
	return d1.Agree(agreedValue, staticSecretKey, staticOtherPublicKey, validateStaticOtherPublicKey)
		&& d2.Agree(agreedValue+d1.AgreedValueLength(), ephemeralSecretKey, ephemeralOtherPublicKey, true);
}

NAMESPACE_END
