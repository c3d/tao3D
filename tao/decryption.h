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
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 2011 Jérôme Forissier <jerome@taodyne.com>
//  (C) 2011 Taodyne SAS
// ****************************************************************************

#include <string>

namespace Tao {

struct Decryption
{
    static std::string Decrypt(std::string &encrypted);
};

}
