#ifndef TAO_UTF8_H
#define TAO_UTF8_H
// *****************************************************************************
// tao_utf8.h                                                      Tao3D project
// *****************************************************************************
//
// File description:
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
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2010, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010-2011,2013-2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2012, Jérôme Forissier <jerome@taodyne.com>
// (C) 2010, Lionel Schaffhauser <lionel@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can redistribute it and/or modify
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


inline kstring operator +(QString s)
// ----------------------------------------------------------------------------
//   Quickly convert from QString to kstring (can also be used for text)
// ----------------------------------------------------------------------------
{
    return s.toUtf8().constData();
}


TAO_END

inline uintptr_t _recorder_arg(const QString &arg)\
// ----------------------------------------------------------------------------
//    Store data from a QString into the recorder
// ----------------------------------------------------------------------------
{
    return (uintptr_t) arg.toUtf8().constData();
}


// Print a string for debug purpose
extern void pqs(const QString &qs);

#endif // TAO_UTF8_H
