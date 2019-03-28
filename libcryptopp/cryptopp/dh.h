#ifndef CRYPTOPP_DH_H
#define CRYPTOPP_DH_H
// *****************************************************************************
// dh.h                                                            Tao3D project
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

#include "gfpcrypt.h"

NAMESPACE_BEGIN(CryptoPP)

//! ,
template <class GROUP_PARAMETERS, class COFACTOR_OPTION = CPP_TYPENAME GROUP_PARAMETERS::DefaultCofactorOption>
class DH_Domain : public DL_SimpleKeyAgreementDomainBase<typename GROUP_PARAMETERS::Element>
{
	typedef DL_SimpleKeyAgreementDomainBase<typename GROUP_PARAMETERS::Element> Base;

public:
	typedef GROUP_PARAMETERS GroupParameters;
	typedef typename GroupParameters::Element Element;
	typedef DL_KeyAgreementAlgorithm_DH<Element, COFACTOR_OPTION> DH_Algorithm;
	typedef DH_Domain<GROUP_PARAMETERS, COFACTOR_OPTION> Domain;

	DH_Domain() {}

	DH_Domain(const GroupParameters &params)
		: m_groupParameters(params) {}

	DH_Domain(BufferedTransformation &bt)
		{m_groupParameters.BERDecode(bt);}

	template <class T2>
	DH_Domain(RandomNumberGenerator &v1, const T2 &v2)
		{m_groupParameters.Initialize(v1, v2);}
	
	template <class T2, class T3>
	DH_Domain(RandomNumberGenerator &v1, const T2 &v2, const T3 &v3)
		{m_groupParameters.Initialize(v1, v2, v3);}
	
	template <class T2, class T3, class T4>
	DH_Domain(RandomNumberGenerator &v1, const T2 &v2, const T3 &v3, const T4 &v4)
		{m_groupParameters.Initialize(v1, v2, v3, v4);}

	template <class T1, class T2>
	DH_Domain(const T1 &v1, const T2 &v2)
		{m_groupParameters.Initialize(v1, v2);}
	
	template <class T1, class T2, class T3>
	DH_Domain(const T1 &v1, const T2 &v2, const T3 &v3)
		{m_groupParameters.Initialize(v1, v2, v3);}
	
	template <class T1, class T2, class T3, class T4>
	DH_Domain(const T1 &v1, const T2 &v2, const T3 &v3, const T4 &v4)
		{m_groupParameters.Initialize(v1, v2, v3, v4);}

	const GroupParameters & GetGroupParameters() const {return m_groupParameters;}
	GroupParameters & AccessGroupParameters() {return m_groupParameters;}

	void GeneratePublicKey(RandomNumberGenerator &rng, const byte *privateKey, byte *publicKey) const
	{
		Base::GeneratePublicKey(rng, privateKey, publicKey);

		if (FIPS_140_2_ComplianceEnabled())
		{
			SecByteBlock privateKey2(this->PrivateKeyLength());
			this->GeneratePrivateKey(rng, privateKey2);

			SecByteBlock publicKey2(this->PublicKeyLength());
			Base::GeneratePublicKey(rng, privateKey2, publicKey2);

			SecByteBlock agreedValue(this->AgreedValueLength()), agreedValue2(this->AgreedValueLength());
			bool agreed1 = this->Agree(agreedValue, privateKey, publicKey2);
			bool agreed2 = this->Agree(agreedValue2, privateKey2, publicKey);

			if (!agreed1 || !agreed2 || agreedValue != agreedValue2)
				throw SelfTestFailure(this->AlgorithmName() + ": pairwise consistency test failed");
		}
	}

	static std::string CRYPTOPP_API StaticAlgorithmName()
		{return GroupParameters::StaticAlgorithmNamePrefix() + DH_Algorithm::StaticAlgorithmName();}
	std::string AlgorithmName() const {return StaticAlgorithmName();}

private:
	const DL_KeyAgreementAlgorithm<Element> & GetKeyAgreementAlgorithm() const
		{return Singleton<DH_Algorithm>().Ref();}
	DL_GroupParameters<Element> & AccessAbstractGroupParameters()
		{return m_groupParameters;}

	GroupParameters m_groupParameters;
};

CRYPTOPP_DLL_TEMPLATE_CLASS DH_Domain<DL_GroupParameters_GFP_DefaultSafePrime>;

//! <a href="http://www.weidai.com/scan-mirror/ka.html#DH">Diffie-Hellman</a> in GF(p) with key validation
typedef DH_Domain<DL_GroupParameters_GFP_DefaultSafePrime> DH;

NAMESPACE_END

#endif
