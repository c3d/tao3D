#ifndef CRYPTOPP_XTRCRYPT_H
#define CRYPTOPP_XTRCRYPT_H
// *****************************************************************************
// xtrcrypt.h                                                      Tao3D project
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
	"The XTR public key system" by Arjen K. Lenstra and Eric R. Verheul
*/

#include "xtr.h"

NAMESPACE_BEGIN(CryptoPP)

//! XTR-DH with key validation

class XTR_DH : public SimpleKeyAgreementDomain, public CryptoParameters
{
	typedef XTR_DH ThisClass;
	
public:
	XTR_DH(const Integer &p, const Integer &q, const GFP2Element &g);
	XTR_DH(RandomNumberGenerator &rng, unsigned int pbits, unsigned int qbits);
	XTR_DH(BufferedTransformation &domainParams);

	void DEREncode(BufferedTransformation &domainParams) const;

	bool Validate(RandomNumberGenerator &rng, unsigned int level) const;
	bool GetVoidValue(const char *name, const std::type_info &valueType, void *pValue) const;
	void AssignFrom(const NameValuePairs &source);
	CryptoParameters & AccessCryptoParameters() {return *this;}
	unsigned int AgreedValueLength() const {return 2*m_p.ByteCount();}
	unsigned int PrivateKeyLength() const {return m_q.ByteCount();}
	unsigned int PublicKeyLength() const {return 2*m_p.ByteCount();}

	void GeneratePrivateKey(RandomNumberGenerator &rng, byte *privateKey) const;
	void GeneratePublicKey(RandomNumberGenerator &rng, const byte *privateKey, byte *publicKey) const;
	bool Agree(byte *agreedValue, const byte *privateKey, const byte *otherPublicKey, bool validateOtherPublicKey=true) const;

	const Integer &GetModulus() const {return m_p;}
	const Integer &GetSubgroupOrder() const {return m_q;}
	const GFP2Element &GetSubgroupGenerator() const {return m_g;}

	void SetModulus(const Integer &p) {m_p = p;}
	void SetSubgroupOrder(const Integer &q) {m_q = q;}
	void SetSubgroupGenerator(const GFP2Element &g) {m_g = g;}

private:
	unsigned int ExponentBitLength() const;

	Integer m_p, m_q;
	GFP2Element m_g;
};

NAMESPACE_END

#endif
