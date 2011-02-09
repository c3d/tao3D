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


struct ExtractComment : public XL::Action
// ----------------------------------------------------------------------------
//   Extract Comment surrounded by /*| |*/ from the given tree
// ----------------------------------------------------------------------------
{
    ExtractComment() : opening("/*|"), closing("|*/") {}
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
public:
    text opening, closing;
};

struct ExtractDoc : public ExtractComment
// ----------------------------------------------------------------------------
//   Extract documentation surrounded by /** **/ from the given tree
// ----------------------------------------------------------------------------
{
    ExtractDoc(text defGrp = "") : defaultGroup(defGrp), params_tree(0)
    {
        opening = "/**";
        closing = "**/";
    }
    XL::Tree *DoInfix(XL::Infix *what);
    XL::Tree *DoPrefix(XL::Prefix *what);
    XL::Tree *DoPostfix(XL::Postfix *what);

    text formatSyntax(XL::Tree *t);
    text formatDoc(text *c);
    text addParam(XL::Name *name, XL::Name *ptype, text *coment);
    text defaultGroup;
    int params_tree;
    text symbol;
    text syntax;
    text params;
};

#endif // DOCUMENTATION_H
