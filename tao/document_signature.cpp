// ****************************************************************************
//  document_signature.cpp                                         Tao project
// ****************************************************************************
//
//   File Description:
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
// ****************************************************************************
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 2013 Jerome Forissier <jerome@taodyne.com>
//  (C) 2013 Taodyne SAS
// ****************************************************************************


#include "document_signature.h"

#include "base.h"
#include "tao_utf8.h"

#include "cryptopp/dsa.h"
#include "cryptopp/osrng.h"
#include "cryptopp/base64.h"

#include "public_key_dsa.h"
#include "doc_public_key_dsa.h"

#include <QFileInfo>
#include <QFile>
#include <QSettings>

TAO_BEGIN


SignatureInfo::SignatureInfo(text path)
// ----------------------------------------------------------------------------
//   Load signature for an existing file
// ----------------------------------------------------------------------------
    : path(path)
{
    status = checkSignature();
}


SignatureInfo::Status SignatureInfo::checkSignature()
// ----------------------------------------------------------------------------
//   Check if file is signed and if signature is valid
// ----------------------------------------------------------------------------
{
    QString path = +this->path;
    QFile file(path);
    if (!file.open(QFile::ReadOnly))
    {
        IFTRACE2(lic,fileload)
            debug() << +path << " not readable\n";
        return SI_FILEERR;
    }

    QString sigPath = path + ".sig";

    if (!QFileInfo(sigPath).isReadable())
    {
        IFTRACE2(lic,fileload)
            debug() << +path << " not signed\n";
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
        debug() << +path << " signed, " << (ok ? "" : "in") << "valid\n";

    return ok ? SI_VALID : SI_INVALID;
}


static byte pubKeyTao[] = TAO_DSA_PUBLIC_KEY;
static byte pubKeyDoc[] = DOC_DSA_PUBLIC_KEY;
//#ifndef TAO_PLAYER
//static byte privKeyDoc[] = DOC_DSA_PRIVATE_KEY;
//#endif

using namespace CryptoPP;

bool SignatureInfo::verify(QByteArray content, QByteArray pubKey,
                           QByteArray signature)
// ----------------------------------------------------------------------------
//   True if signature is valid and pubKey is one of the known keys
// ----------------------------------------------------------------------------
{
#define BA(x) QByteArray((const char *)x, sizeof(x))

    // Check key
    if (pubKey != BA(pubKeyTao) && pubKey != BA(pubKeyDoc))
    {
        IFTRACE2(lic, fileload)
            debug() << path << " unexpected public key\n";
        return false;
    }
#undef BA

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


std::ostream & SignatureInfo::debug()
// ----------------------------------------------------------------------------
//   Convenience method to log with a common prefix
// ----------------------------------------------------------------------------
{
    std::cerr << "[DocumentSignature] ";
    return std::cerr;
}

TAO_END
