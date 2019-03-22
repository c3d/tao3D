#ifndef FONT_H
#define FONT_H
// *****************************************************************************
// font.h                                                          Tao3D project
// *****************************************************************************
//
// File description:
//
//    Parsing of font descriptions and management of fonts
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
// (C) 2010,2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2012, Jérôme Forissier <jerome@taodyne.com>
// (C) 2010, Lionel Schaffhauser <lionel@taodyne.com>
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

#include "tao_tree.h"
#include "action.h"
#include <QFont>
#include <QMap>
#include <QPair>

TAO_BEGIN

struct FontParsingAction : XL::Action
// ----------------------------------------------------------------------------
//   Parse a font description
// ----------------------------------------------------------------------------
{
    FontParsingAction(Context *context, const QFont &font)
        : context(context), font(font), exactMatch(false) {}

    virtual Tree *Do (Tree *what);
    virtual Tree *DoInteger(Integer *what);
    virtual Tree *DoReal(Real *what);
    virtual Tree *DoText(Text *what);
    virtual Tree *DoName(Name *what);
    virtual Tree *DoPrefix(Prefix *what);
    virtual Tree *DoPostfix(Postfix *what);
    virtual Tree *DoInfix(Infix *what);
    virtual Tree *DoBlock(Block *what);

    bool          SetAttribute(Name *name, Tree *value);

public:
    Context_p   context;
    QFont       font;
    bool        exactMatch;

public:
    static QMap<QPair<QString, QString>, bool> exactMatchCache;
};


inline double fontSizeAdjust(double size)
// ----------------------------------------------------------------------------
//   Adjust font size for Windows
// ----------------------------------------------------------------------------
{
#ifndef CONFIG_MACOSX
    size *= 72.0 / 96.0;
#endif
    return size;
}

TAO_END

#endif // FONT_H

