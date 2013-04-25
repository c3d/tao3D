// ****************************************************************************
//  crypto.cpp                                                    Tao project
// ****************************************************************************
//
//   File Description:
//
//     Crypto functions used by Tao and external tools.
//
//
//
//
//
//
//
// ****************************************************************************
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 2011, 2013 Jérôme Forissier <jerome@taodyne.com>
//  (C) 2011, 2013 Taodyne SAS
// ****************************************************************************

#include "crypto.h"
#include "private_key_rsa.h"
#include "cryptopp/rsa.h"
#include "cryptopp/gcm.h"
#include "cryptopp/osrng.h"
#include <string>
#include <stdint.h>
#ifdef CONFIG_MINGW
#include <winsock2.h>
#else
#include <arpa/inet.h>
#endif

using namespace CryptoPP;

namespace Tao {

std::string Crypto::Decrypt(std::string &ciphered)
// ----------------------------------------------------------------------------
//   Decrypt data, return "" on error
// ----------------------------------------------------------------------------
{
    std::string plain;

    try
    {
        const char *pdata = (const char *)ciphered.data();

        // Read encrypted session key and iv
        std::string eKey, eIv;
        uint16_t len;
        byte *plen = (byte *)&len;
        int remaining = ciphered.size();
        remaining -= 2;
        if (remaining <= 0)
            return "";
        memcpy(plen, pdata, 2);
        pdata += 2;
        len = ntohs(len);
        remaining -= len;
        if (remaining <= 0)
            return "";
        eKey.append(pdata, len);
        pdata += len;
        remaining -= 2;
        if (remaining <= 0)
            return "";
        memcpy(plen, pdata, 2);
        pdata += 2;
        len = ntohs(len);
        remaining -= len;
        if (remaining <= 0)
            return "";
        eIv.append(pdata, len);
        pdata += len;

        // Load private key
        byte key[] = TAO_RSA_PRIVATE_KEY;
        RSA::PrivateKey privateKey;
        privateKey.Load(StringSource(key, sizeof(key), true).Ref());

        // Decrypt session key and iv using RSA
        std::string sKey, sIv;
        AutoSeededRandomPool rng;
        RSAES_OAEP_SHA_Decryptor rsa(privateKey );
        StringSource(eKey, true,
                     new PK_DecryptorFilter(rng, rsa,
                                            new StringSink(sKey)));
        StringSource(eIv, true,
                     new PK_DecryptorFilter(rng, rsa,
                                            new StringSink(sIv)));

        // Decrypt message using AES
        ciphered = ciphered.substr(pdata - ciphered.data());
        const int TAG_SIZE = 12;
        GCM< AES >::Decryption aes;
        aes.SetKeyWithIV((const byte *)sKey.data(), sKey.length(),
                         (const byte *)sIv.data(), sIv.length());

        AuthenticatedDecryptionFilter df(aes,
                                         new StringSink(plain),
                                         AuthenticatedDecryptionFilter::DEFAULT_FLAGS,
                                         TAG_SIZE);
        StringSource(ciphered, true, new Redirector(df));

        // If the object does not throw, here's the only
        //  opportunity to check the data's integrity
        bool ok = df.GetLastResult();
        if (!ok)
        {
            plain = "";
        }
    }
    catch (...)
    {
        plain = "";
    }

    return plain;
}

}
