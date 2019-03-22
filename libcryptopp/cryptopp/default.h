#ifndef CRYPTOPP_DEFAULT_H
#define CRYPTOPP_DEFAULT_H
// *****************************************************************************
// default.h                                                       Tao3D project
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

#include "sha.h"
#include "hmac.h"
#include "des.h"
#include "filters.h"
#include "modes.h"

NAMESPACE_BEGIN(CryptoPP)

typedef DES_EDE2 Default_BlockCipher;
typedef SHA DefaultHashModule;
typedef HMAC<DefaultHashModule> DefaultMAC;

//! Password-Based Encryptor using DES-EDE2
class DefaultEncryptor : public ProxyFilter
{
public:
	DefaultEncryptor(const char *passphrase, BufferedTransformation *attachment = NULL);
	DefaultEncryptor(const byte *passphrase, size_t passphraseLength, BufferedTransformation *attachment = NULL);

protected:
	void FirstPut(const byte *);
	void LastPut(const byte *inString, size_t length);

private:
	SecByteBlock m_passphrase;
	CBC_Mode<Default_BlockCipher>::Encryption m_cipher;
};

//! Password-Based Decryptor using DES-EDE2
class DefaultDecryptor : public ProxyFilter
{
public:
	DefaultDecryptor(const char *passphrase, BufferedTransformation *attachment = NULL, bool throwException=true);
	DefaultDecryptor(const byte *passphrase, size_t passphraseLength, BufferedTransformation *attachment = NULL, bool throwException=true);

	class Err : public Exception
	{
	public:
		Err(const std::string &s)
			: Exception(DATA_INTEGRITY_CHECK_FAILED, s) {}
	};
	class KeyBadErr : public Err {public: KeyBadErr() : Err("DefaultDecryptor: cannot decrypt message with this passphrase") {}};

	enum State {WAITING_FOR_KEYCHECK, KEY_GOOD, KEY_BAD};
	State CurrentState() const {return m_state;}

protected:
	void FirstPut(const byte *inString);
	void LastPut(const byte *inString, size_t length);

	State m_state;

private:
	void CheckKey(const byte *salt, const byte *keyCheck);

	SecByteBlock m_passphrase;
	CBC_Mode<Default_BlockCipher>::Decryption m_cipher;
	member_ptr<FilterWithBufferedInput> m_decryptor;
	bool m_throwException;
};

//! Password-Based Encryptor using DES-EDE2 and HMAC/SHA-1
class DefaultEncryptorWithMAC : public ProxyFilter
{
public:
	DefaultEncryptorWithMAC(const char *passphrase, BufferedTransformation *attachment = NULL);
	DefaultEncryptorWithMAC(const byte *passphrase, size_t passphraseLength, BufferedTransformation *attachment = NULL);

protected:
	void FirstPut(const byte *inString) {}
	void LastPut(const byte *inString, size_t length);

private:
	member_ptr<DefaultMAC> m_mac;
};

//! Password-Based Decryptor using DES-EDE2 and HMAC/SHA-1
class DefaultDecryptorWithMAC : public ProxyFilter
{
public:
	class MACBadErr : public DefaultDecryptor::Err {public: MACBadErr() : DefaultDecryptor::Err("DefaultDecryptorWithMAC: MAC check failed") {}};

	DefaultDecryptorWithMAC(const char *passphrase, BufferedTransformation *attachment = NULL, bool throwException=true);
	DefaultDecryptorWithMAC(const byte *passphrase, size_t passphraseLength, BufferedTransformation *attachment = NULL, bool throwException=true);

	DefaultDecryptor::State CurrentState() const;
	bool CheckLastMAC() const;

protected:
	void FirstPut(const byte *inString) {}
	void LastPut(const byte *inString, size_t length);

private:
	member_ptr<DefaultMAC> m_mac;
	HashVerifier *m_hashVerifier;
	bool m_throwException;
};

NAMESPACE_END

#endif
