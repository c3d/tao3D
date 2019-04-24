#ifndef DOCUMENTATION_H
#define DOCUMENTATION_H
// *****************************************************************************
// documentation.h                                                 Tao3D project
// *****************************************************************************
//
// File description:
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
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2010, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010,2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2011, Jérôme Forissier <jerome@taodyne.com>
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

#include "action.h"
#include "parser.h"
#include "tao_tree.h"

TAO_BEGIN

struct ExtractComment
// ----------------------------------------------------------------------------
//   Extract Comment surrounded by /*| |*/ from the given tree
// ----------------------------------------------------------------------------
{
    typedef text value_type;

    ExtractComment() : opening("/*|"), closing("|*/") {}
    text Extract(XL::CommentsList commentList);

    text Do(Tree *what);
    text Do(Block *what);
    text Do(Infix *what);
    text Do(Prefix *what);
    text Do(Postfix *what);

protected:
    text Pre (Tree *what);
    text Post(Tree *what);

public:
    text opening, closing;
};


struct ExtractDoc : public ExtractComment
// ----------------------------------------------------------------------------
//   Extract documentation surrounded by /** **/ from the given tree
// ----------------------------------------------------------------------------
{
    typedef text value_type;

    ExtractDoc(text defGrp = "") : defaultGroup(defGrp), params_tree(0)
    {
        opening = "/**";
        closing = "**/";
    }
    text Do(Tree *what);
    text Do(Infix *what);
    text Do(Prefix *what);
    text Do(Postfix *what);

    text formatSyntax(Tree *t);
    text formatDoc(text *c);
    text addParam(Name *name, Name *ptype, text *coment);
    text defaultGroup;
    int  params_tree;
    text symbol;
    text syntax;
    text params;
};

TAO_END

#endif // DOCUMENTATION_H
