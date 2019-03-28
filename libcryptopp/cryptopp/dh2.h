#ifndef CRYPTOPP_DH2_H
#define CRYPTOPP_DH2_H
// *****************************************************************************
// dh2.h                                                           Tao3D project
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

/** \file
*/

#include "cryptlib.h"

NAMESPACE_BEGIN(CryptoPP)

/// <a href="http://www.weidai.com/scan-mirror/ka.html#DH2">Unified Diffie-Hellman</a>
class DH2 : public AuthenticatedKeyAgreementDomain
{
public:
	DH2(SimpleKeyAgreementDomain &domain)
		: d1(domain), d2(domain) {}
	DH2(SimpleKeyAgreementDomain &staticDomain, SimpleKeyAgreementDomain &ephemeralDomain)
		: d1(staticDomain), d2(ephemeralDomain) {}

	CryptoParameters & AccessCryptoParameters() {return d1.AccessCryptoParameters();}

	unsigned int AgreedValueLength() const
		{return d1.AgreedValueLength() + d2.AgreedValueLength();}

	unsigned int StaticPrivateKeyLength() const
		{return d1.PrivateKeyLength();}
	unsigned int StaticPublicKeyLength() const
		{return d1.PublicKeyLength();}
	void GenerateStaticPrivateKey(RandomNumberGenerator &rng, byte *privateKey) const
		{d1.GeneratePrivateKey(rng, privateKey);}
	void GenerateStaticPublicKey(RandomNumberGenerator &rng, const byte *privateKey, byte *publicKey) const
		{d1.GeneratePublicKey(rng, privateKey, publicKey);}
	void GenerateStaticKeyPair(RandomNumberGenerator &rng, byte *privateKey, byte *publicKey) const
		{d1.GenerateKeyPair(rng, privateKey, publicKey);}

	unsigned int EphemeralPrivateKeyLength() const
		{return d2.PrivateKeyLength();}
	unsigned int EphemeralPublicKeyLength() const
		{return d2.PublicKeyLength();}
	void GenerateEphemeralPrivateKey(RandomNumberGenerator &rng, byte *privateKey) const
		{d2.GeneratePrivateKey(rng, privateKey);}
	void GenerateEphemeralPublicKey(RandomNumberGenerator &rng, const byte *privateKey, byte *publicKey) const
		{d2.GeneratePublicKey(rng, privateKey, publicKey);}
	void GenerateEphemeralKeyPair(RandomNumberGenerator &rng, byte *privateKey, byte *publicKey) const
		{d2.GenerateKeyPair(rng, privateKey, publicKey);}

	bool Agree(byte *agreedValue,
		const byte *staticPrivateKey, const byte *ephemeralPrivateKey,
		const byte *staticOtherPublicKey, const byte *ephemeralOtherPublicKey,
		bool validateStaticOtherPublicKey=true) const;

protected:
	SimpleKeyAgreementDomain &d1, &d2;
};

NAMESPACE_END

#endif
