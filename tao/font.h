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
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Jérôme Forissier <jerome@taodyne.com>
//  (C) 2010 Catherine Burvelle <cathy@taodyne.com>
//  (C) 2010 Lionel Schaffhauser <lionel@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "tao_tree.h"
#include <QFont>

TAO_BEGIN

struct FontParsingAction : Action
// ----------------------------------------------------------------------------
//   Parse a font description
// ----------------------------------------------------------------------------
{
    FontParsingAction(Symbols *symbols, const QFont &font)
        : symbols(symbols), font(font), exactMatch(false) {}

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
    Symbols_p   symbols;
    QFont       font;
    bool        exactMatch;
};


TAO_END

#endif // FONT_H

