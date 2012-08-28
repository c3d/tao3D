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
// This file may be used in accordance with the terms and conditions contained
// in the Tao Presentations license agreement, or alternatively, in a signed
// license agreement between you and Taodyne SAS.
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Jérôme Forissier <jerome@taodyne.com>
//  (C) 2010 Catherine Burvelle <cathy@taodyne.com>
//  (C) 2010 Lionel Schaffhauser <lionel@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "tao.h"
#include "utf8.h"
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

#endif // TAO_UTF8_H
