#ifndef CRYPTO_H
#define CRYPTO_H
// ****************************************************************************
//  decrypt.h                                                      Tao project
// ****************************************************************************
//
//   File Description:
//
//     Decrypt encrypted XL files
//
//
//
//
//
//
//
// ****************************************************************************
// This software is licensed under the GNU General Public License v3.
// See file COPYING for details.
//  (C) 2011 Jérôme Forissier <jerome@taodyne.com>
//  (C) 2011 Taodyne SAS
// ****************************************************************************

#ifndef CFG_NO_CRYPT
#include <QString>
#include <QByteArray>
#include <string>


namespace Tao {

struct Crypto
{
    static std::string Decrypt(std::string &encrypted);
    static QString Sign(QString path, QByteArray pubKey, QByteArray privKey,
                        QString ident);
};

}

#endif // CFG_NO_CRYPT
#endif // CRYPTO_H
