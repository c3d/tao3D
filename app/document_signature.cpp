// *****************************************************************************
// document_signature.cpp                                          Tao3D project
// *****************************************************************************
//
// File description:
//
//    Sign XL files used in a document with the document private key.
//    Verify signatures with the Tao public key or the document public key.
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
// (C) 2013, Jérôme Forissier <jerome@taodyne.com>
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


#include "document_signature.h"

#include "application.h"
#include "base.h"
#include "crypto.h"
#include "tao_utf8.h"

#include "cryptopp/dsa.h"
#include "cryptopp/osrng.h"
#include "cryptopp/base64.h"

#include "public_key_dsa.h"
#include "doc_public_key_dsa.h"
#ifndef TAO_PLAYER
#include "doc_private_key_dsa.h"
#endif

#include <QFileInfo>
#include <QFile>
#include <QSettings>

#define BA(x) QByteArray((const char *)x, sizeof(x))


TAO_BEGIN


extern const char * GITREV_;
extern const char * GITSHA1_;

static byte pubKeyTao[] = TAO_DSA_PUBLIC_KEY;
static byte pubKeyDoc[] = DOC_DSA_PUBLIC_KEY;
#ifndef TAO_PLAYER
static byte privKeyDoc[] = DOC_DSA_PRIVATE_KEY;
#endif



SignatureInfo::SignatureInfo(text path)
// ----------------------------------------------------------------------------
//   Load signature for an existing file
// ----------------------------------------------------------------------------
    : path(path), status(SI_UNKNOWN)
{}


SignatureInfo::Status SignatureInfo::loadAndCheckSignature()
// ----------------------------------------------------------------------------
//   Check if file is signed and if signature is valid
// ----------------------------------------------------------------------------
{
    QString path = +this->path;
    QFile file(path);
    if (!file.open(QFile::ReadOnly))
    {
        IFTRACE2(lic,fileload)
            debug() << +path << " is not readable\n";
        return SI_FILEERR;
    }

    QString sigPath = path + ".sig";

    if (!QFileInfo(sigPath).isReadable())
    {
        IFTRACE2(lic,fileload)
            debug() << +path << " is not signed\n";
        return SI_NOTSIGNED;
    }
    QSettings settings(sigPath, QSettings::IniFormat);

    QByteArray content = file.readAll();
    QByteArray pubKey = QByteArray::fromBase64(settings.value("pubkey")
                                    .toByteArray());
    QByteArray signature = QByteArray::fromBase64(settings.value("signature")
                                       .toByteArray());

    bool ok = verify(content, pubKey, signature);
    IFTRACE2(lic,fileload)
        debug() << +path << " is signed (.sig " << (ok ? "valid":"INVALID")
                << ")\n";

    return ok ? SI_VALID : SI_INVALID;
}

using namespace CryptoPP;

bool SignatureInfo::verify(QByteArray content, QByteArray pubKey,
                           QByteArray signature)
// ----------------------------------------------------------------------------
//   True if signature is valid and pubKey is one of the known keys
// ----------------------------------------------------------------------------
{
    // Check key
    if (pubKey != BA(pubKeyTao) && pubKey != BA(pubKeyDoc))
    {
        IFTRACE2(lic, fileload)
            debug() << path << " unexpected public key\n";
        return false;
    }

    // Hash file content
    SHA1 hash;
    byte cHash[SHA1::DIGESTSIZE];
    hash.Update((const byte *)content.data(), content.size());
    hash.Final(cHash);

    // Load public key
    DSA::Verifier verifier;
    PublicKey &publicKey = verifier.AccessPublicKey();
    publicKey.Load(StringSource((const byte *)pubKey.data(),
                                pubKey.size(), true).Ref());

    // Verify signature
    bool result = verifier.VerifyMessage(cHash, sizeof(cHash),
                                         (const byte *)signature.data(),
                                         signature.size());
    return result;
}


#ifndef TAO_PLAYER
QString SignatureInfo::signFileWithDocKey()
// ----------------------------------------------------------------------------
//   Sign the file content (on disk) with the document private key
// ----------------------------------------------------------------------------
{
    IFTRACE2(lic, fileload)
        debug() << "Signing " << path << " with doc key\n";

    QByteArray qpubKeyDoc = BA(pubKeyDoc);
    QByteArray qprivKeyDoc = BA(privKeyDoc);

    QString ident = "; Signed by: Tao3D " +
                       Application::editionStr() + "\n"
                    "; version " + GITREV_ + " (" + GITSHA1_ + ")\n";

    status = SI_VALID;
    QString err = Tao::Crypto::Sign(+path, qpubKeyDoc, qprivKeyDoc, ident);
    if (!err.isEmpty())
    {
        IFTRACE2(lic, fileload)
            debug() << "Error: " << +err << "\n";
        status = SI_FILEERR;
    }
    return err;
}
#endif

std::ostream & SignatureInfo::debug()
// ----------------------------------------------------------------------------
//   Convenience method to log with a common prefix
// ----------------------------------------------------------------------------
{
    std::cerr << "[SignatureInfo] ";
    return std::cerr;
}

TAO_END
