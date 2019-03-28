// *****************************************************************************
// ttmac.h                                                         Tao3D project
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
// ttmac.h - written and placed in the public domain by Kevin Springle

#ifndef CRYPTOPP_TTMAC_H
#define CRYPTOPP_TTMAC_H

#include "seckey.h"
#include "iterhash.h"

NAMESPACE_BEGIN(CryptoPP)

//! _
class CRYPTOPP_NO_VTABLE TTMAC_Base : public FixedKeyLength<20>, public IteratedHash<word32, LittleEndian, 64, MessageAuthenticationCode>
{
public:
	static std::string StaticAlgorithmName() {return std::string("Two-Track-MAC");}
	CRYPTOPP_CONSTANT(DIGESTSIZE=20)

	unsigned int DigestSize() const {return DIGESTSIZE;};
	void UncheckedSetKey(const byte *userKey, unsigned int keylength, const NameValuePairs &params);
	void TruncatedFinal(byte *mac, size_t size);

protected:
	static void Transform (word32 *digest, const word32 *X, bool last);
	void HashEndianCorrectedBlock(const word32 *data) {Transform(m_digest, data, false);}
	void Init();
	word32* StateBuf() {return m_digest;}

	FixedSizeSecBlock<word32, 10> m_digest;
	FixedSizeSecBlock<word32, 5> m_key;
};

//! <a href="http://www.weidai.com/scan-mirror/mac.html#TTMAC">Two-Track-MAC</a>
/*! 160 Bit MAC with 160 Bit Key */
DOCUMENTED_TYPEDEF(MessageAuthenticationCodeFinal<TTMAC_Base>, TTMAC)

NAMESPACE_END

#endif
