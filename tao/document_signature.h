#ifndef DOCUMENT_SIGNATURE_H
#define DOCUMENT_SIGNATURE_H
// ****************************************************************************
//  document_signature.h                                           Tao project
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

#include "info.h"
#include "tao.h"
#include <QByteArray>
#include <QString>
#include <iostream>

TAO_BEGIN

class SignatureInfo : public XL::Info
// ----------------------------------------------------------------------------
//   Sign files or verify signatures
// ----------------------------------------------------------------------------
{

public:
    SignatureInfo(text path);
    ~SignatureInfo() {}

protected:
    enum Status { SI_FILEERR, SI_NOTSIGNED, SI_VALID, SI_INVALID };

protected:
    Status            checkSignature();
    bool              verify(QByteArray content, QByteArray pubKey,
                             QByteArray signature);

protected:
    std::ostream &    debug();

private:
    text              path;
    Status            status;
};

TAO_END

#endif // DOCUMENT_SIGNATURE_H
