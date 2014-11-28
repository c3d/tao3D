#ifndef TAO_UTF8_H
#define TAO_UTF8_H
// ****************************************************************************
//  tao_utf8.h                                                      Tao project
// ****************************************************************************
//
//   File Description:
//
//    Special management of UTF8 for Tao
//
//
//
//
//
//
//
//
// ****************************************************************************
// This software is licensed under the GNU General Public License v3
// See file COPYING for details.
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Jérôme Forissier <jerome@taodyne.com>
//  (C) 2010 Catherine Burvelle <cathy@taodyne.com>
//  (C) 2010 Lionel Schaffhauser <lionel@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "tao.h"
#include "utf8.h"
#include <string>
#include <QString>

TAO_BEGIN

inline QString operator +(text s)
// ----------------------------------------------------------------------------
//   Quickly convert from text to QString
// ----------------------------------------------------------------------------
{
    return QString::fromUtf8(s.data(), s.length());
}


inline text operator +(QString s)
// ----------------------------------------------------------------------------
//   Quickly convert from QString to text
// ----------------------------------------------------------------------------
{
    return text(s.toUtf8().constData());
}

TAO_END

// Print a string for debug purpose
extern void pqs(const QString &qs);

#endif // TAO_UTF8_H
