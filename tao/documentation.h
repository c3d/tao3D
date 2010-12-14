#ifndef DOCUMENTATION_H
#define DOCUMENTATION_H
// ****************************************************************************
//  documentation.h                                                 Tao project
// ****************************************************************************
//
//   File Description:
//
//    Manage the documentation generation.
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
//  (C) 2010 Catherine Burvelle <cathy@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************
#include "action.h"
#include "parser.h"
#include "tao_tree.h"


struct ExtractDoc : public XL::Action
// ----------------------------------------------------------------------------
//   Extract documentation from a tree
// ----------------------------------------------------------------------------
{
    ExtractDoc() {}
    text extract(XL::CommentsList commentList);

    XL::Tree *DoInteger(XL::Integer *what);
    XL::Tree *DoReal(XL::Real *what);
    XL::Tree *DoText(XL::Text *what);
    XL::Tree *DoName(XL::Name *what);

    XL::Tree *DoBlock(XL::Block *what);
    XL::Tree *DoInfix(XL::Infix *what);
    XL::Tree *DoPrefix(XL::Prefix *what);
    XL::Tree *DoPostfix(XL::Postfix *what);
    XL::Tree *Do(XL::Tree *what)
    {
        return what->Do(this);
    }
};

#endif // DOCUMENTATION_H
