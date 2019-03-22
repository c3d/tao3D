#ifndef CRYPTOPP_CCM_H
#define CRYPTOPP_CCM_H
// *****************************************************************************
// ccm.h                                                           Tao3D project
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

#include "authenc.h"
#include "modes.h"

NAMESPACE_BEGIN(CryptoPP)

//! .
class CRYPTOPP_DLL CRYPTOPP_NO_VTABLE CCM_Base : public AuthenticatedSymmetricCipherBase
{
public:
	CCM_Base()
		: m_digestSize(0), m_L(0) {}

	// AuthenticatedSymmetricCipher
	std::string AlgorithmName() const
		{return GetBlockCipher().AlgorithmName() + std::string("/CCM");}
	size_t MinKeyLength() const
		{return GetBlockCipher().MinKeyLength();}
	size_t MaxKeyLength() const
		{return GetBlockCipher().MaxKeyLength();}
	size_t DefaultKeyLength() const
		{return GetBlockCipher().DefaultKeyLength();}
	size_t GetValidKeyLength(size_t n) const
		{return GetBlockCipher().GetValidKeyLength(n);}
	bool IsValidKeyLength(size_t n) const
		{return GetBlockCipher().IsValidKeyLength(n);}
	unsigned int OptimalDataAlignment() const
		{return GetBlockCipher().OptimalDataAlignment();}
	IV_Requirement IVRequirement() const
		{return UNIQUE_IV;}
	unsigned int IVSize() const
		{return 8;}
	unsigned int MinIVLength() const
		{return 7;}
	unsigned int MaxIVLength() const
		{return 13;}
	unsigned int DigestSize() const
		{return m_digestSize;}
	lword MaxHeaderLength() const
		{return W64LIT(0)-1;}
	lword MaxMessageLength() const
		{return m_L<8 ? (W64LIT(1)<<(8*m_L))-1 : W64LIT(0)-1;}
	bool NeedsPrespecifiedDataLengths() const
		{return true;}
	void UncheckedSpecifyDataLengths(lword headerLength, lword messageLength, lword footerLength);

protected:
	// AuthenticatedSymmetricCipherBase
	bool AuthenticationIsOnPlaintext() const
		{return true;}
	unsigned int AuthenticationBlockSize() const
		{return GetBlockCipher().BlockSize();}
	void SetKeyWithoutResync(const byte *userKey, size_t keylength, const NameValuePairs &params);
	void Resync(const byte *iv, size_t len);
	size_t AuthenticateBlocks(const byte *data, size_t len);
	void AuthenticateLastHeaderBlock();
	void AuthenticateLastConfidentialBlock();
	void AuthenticateLastFooterBlock(byte *mac, size_t macSize);
	SymmetricCipher & AccessSymmetricCipher() {return m_ctr;}

	virtual BlockCipher & AccessBlockCipher() =0;
	virtual int DefaultDigestSize() const =0;

	const BlockCipher & GetBlockCipher() const {return const_cast<CCM_Base *>(this)->AccessBlockCipher();};
	byte *CBC_Buffer() {return m_buffer+REQUIRED_BLOCKSIZE;}

	enum {REQUIRED_BLOCKSIZE = 16};
	int m_digestSize, m_L;
	word64 m_messageLength, m_aadLength;
	CTR_Mode_ExternalCipher::Encryption m_ctr;
};

//! .
template <class T_BlockCipher, int T_DefaultDigestSize, bool T_IsEncryption>
class CCM_Final : public CCM_Base
{
public:
	static std::string StaticAlgorithmName()
		{return T_BlockCipher::StaticAlgorithmName() + std::string("/CCM");}
	bool IsForwardTransformation() const
		{return T_IsEncryption;}

private:
	BlockCipher & AccessBlockCipher() {return m_cipher;}
	int DefaultDigestSize() const {return T_DefaultDigestSize;}
	typename T_BlockCipher::Encryption m_cipher;
};

/// <a href="http://www.cryptolounge.org/wiki/CCM">CCM</a>
template <class T_BlockCipher, int T_DefaultDigestSize = 16>
struct CCM : public AuthenticatedSymmetricCipherDocumentation
{
	typedef CCM_Final<T_BlockCipher, T_DefaultDigestSize, true> Encryption;
	typedef CCM_Final<T_BlockCipher, T_DefaultDigestSize, false> Decryption;
};

NAMESPACE_END

#endif
