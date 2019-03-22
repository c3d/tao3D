#ifndef DOCUMENT_SIGNATURE_H
#define DOCUMENT_SIGNATURE_H
// *****************************************************************************
// document_signature.h                                            Tao3D project
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

public:
    enum Status { SI_UNKNOWN, SI_FILEERR, SI_NOTSIGNED, SI_VALID, SI_INVALID };

public:
    Status            loadAndCheckSignature();
#ifndef TAO_PLAYER
    QString           signFileWithDocKey();
#endif

protected:
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
