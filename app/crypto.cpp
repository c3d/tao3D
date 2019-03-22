// *****************************************************************************
// crypto.cpp                                                      Tao3D project
// *****************************************************************************
//
// File description:
//
//     Crypto functions used by Tao and external tools.
//
//
//
//
//
//
//
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2011,2013, Jérôme Forissier <jerome@taodyne.com>
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

#include "crypto.h"
#include "private_key_rsa.h"
#include "cryptopp/base64.h"
#include "cryptopp/dsa.h"
#include "cryptopp/rsa.h"
#include "cryptopp/gcm.h"
#include "cryptopp/osrng.h"

#include <QFile>
#include <QTextStream>

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


QString Crypto::Sign(QString path, QByteArray pubKey, QByteArray privKey,
                     QString ident)
// ----------------------------------------------------------------------------
//   Sign path with privKey (create .sig file). Write pubKey and ident.
// ----------------------------------------------------------------------------
{
    QFile file(path);
    if (!file.open(QFile::ReadOnly))
    {
        return path + ": " + file.errorString();
    }
    QByteArray content = file.readAll();

    try
    {
        std::string b64_pubkey, b64_signature;
        QString qpub, qsig;

        StringSource((const byte *)pubKey.data(), pubKey.size(), true,
                     new Base64Encoder(
                         new StringSink(b64_pubkey)));
        qpub.append(b64_pubkey.data());
        qpub = qpub.trimmed();

        DSA::Signer signer;
        PrivateKey &privateKey = signer.AccessPrivateKey();
        privateKey.Load(StringSource((const byte *)privKey.data(),
                                     privKey.size(), true).Ref());
        AutoSeededRandomPool rng;
        CryptoPP::SHA1 hash;
        StringSource((const byte*)content.data(), content.size(), true,
                   new HashFilter(hash,
                       new SignerFilter(rng, signer,
                           new Base64Encoder(
                               new StringSink(b64_signature)))));
        qsig.append(b64_signature.data());
        qsig = qsig.trimmed();

        QString outPath = QString(path) + QString(".sig");
        QFile outFile(outPath);
        if (!outFile.open(QFile::WriteOnly))
        {
            return outPath + ": "  + outFile.errorString();
        }
        QTextStream out(&outFile);
        out.setCodec("UTF-8");
        out << ident << "\n";
        out << "signature=\"" << qsig << "\"\n";
        out << "pubkey=\"\n" << qpub << "\n\"\n";
    }
    catch (Exception &e)
    {
        return  QString(e.what());
    }

    return QString();
}

}
