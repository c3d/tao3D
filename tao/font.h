#ifndef FONT_H
#define FONT_H
// ****************************************************************************
//  font.h                                                          Tao project
// ****************************************************************************
//
//   File Description:
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
// ****************************************************************************
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Jérôme Forissier <jerome@taodyne.com>
//  (C) 2010 Catherine Burvelle <cathy@taodyne.com>
//  (C) 2010 Lionel Schaffhauser <lionel@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "tao_tree.h"
#include "action.h"
#include <QFont>

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

