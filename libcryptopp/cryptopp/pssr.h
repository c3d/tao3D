#ifndef CRYPTOPP_PSSR_H
#define CRYPTOPP_PSSR_H
// *****************************************************************************
// pssr.h                                                          Tao3D project
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
#include "emsa2.h"

#ifdef CRYPTOPP_IS_DLL
#include "sha.h"
#endif

NAMESPACE_BEGIN(CryptoPP)

class CRYPTOPP_DLL PSSR_MEM_Base : public PK_RecoverableSignatureMessageEncodingMethod
{
	virtual bool AllowRecovery() const =0;
	virtual size_t SaltLen(size_t hashLen) const =0;
	virtual size_t MinPadLen(size_t hashLen) const =0;
	virtual const MaskGeneratingFunction & GetMGF() const =0;

public:
	size_t MinRepresentativeBitLength(size_t hashIdentifierLength, size_t digestLength) const;
	size_t MaxRecoverableLength(size_t representativeBitLength, size_t hashIdentifierLength, size_t digestLength) const;
	bool IsProbabilistic() const;
	bool AllowNonrecoverablePart() const;
	bool RecoverablePartFirst() const;
	void ComputeMessageRepresentative(RandomNumberGenerator &rng,
		const byte *recoverableMessage, size_t recoverableMessageLength,
		HashTransformation &hash, HashIdentifier hashIdentifier, bool messageEmpty,
		byte *representative, size_t representativeBitLength) const;
	DecodingResult RecoverMessageFromRepresentative(
		HashTransformation &hash, HashIdentifier hashIdentifier, bool messageEmpty,
		byte *representative, size_t representativeBitLength,
		byte *recoverableMessage) const;
};

template <bool USE_HASH_ID> class PSSR_MEM_BaseWithHashId;
template<> class PSSR_MEM_BaseWithHashId<true> : public EMSA2HashIdLookup<PSSR_MEM_Base> {};
template<> class PSSR_MEM_BaseWithHashId<false> : public PSSR_MEM_Base {};

template <bool ALLOW_RECOVERY, class MGF=P1363_MGF1, int SALT_LEN=-1, int MIN_PAD_LEN=0, bool USE_HASH_ID=false>
class PSSR_MEM : public PSSR_MEM_BaseWithHashId<USE_HASH_ID>
{
	virtual bool AllowRecovery() const {return ALLOW_RECOVERY;}
	virtual size_t SaltLen(size_t hashLen) const {return SALT_LEN < 0 ? hashLen : SALT_LEN;}
	virtual size_t MinPadLen(size_t hashLen) const {return MIN_PAD_LEN < 0 ? hashLen : MIN_PAD_LEN;}
	virtual const MaskGeneratingFunction & GetMGF() const {static MGF mgf; return mgf;}

public:
	static std::string CRYPTOPP_API StaticAlgorithmName() {return std::string(ALLOW_RECOVERY ? "PSSR-" : "PSS-") + MGF::StaticAlgorithmName();}
};

//! <a href="http://www.weidai.com/scan-mirror/sig.html#sem_PSSR-MGF1">PSSR-MGF1</a>
struct PSSR : public SignatureStandard
{
	typedef PSSR_MEM<true> SignatureMessageEncodingMethod;
};

//! <a href="http://www.weidai.com/scan-mirror/sig.html#sem_PSS-MGF1">PSS-MGF1</a>
struct PSS : public SignatureStandard
{
	typedef PSSR_MEM<false> SignatureMessageEncodingMethod;
};

NAMESPACE_END

#endif
