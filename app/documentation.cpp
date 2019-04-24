// *****************************************************************************
// documentation.cpp                                               Tao3D project
// *****************************************************************************
//
// File description:
//
//     Manage the documentation generation.
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

#include "documentation.h"
#include "base.h"
#include "renderer.h"
#include <sstream>
#include <map>
#include <QFileInfo>


TAO_BEGIN

using namespace XL;

text ExtractComment::Extract(CommentsList commentList)
// ----------------------------------------------------------------------------
//   Extract the documentation (/*| |*/) from the comments list.
// ----------------------------------------------------------------------------
{
    bool inDoc = false;
    text theComment = "";
    for (text comment : commentList)
    {
        if (comment.find(opening) == 0)
        {
            inDoc = true;
            theComment += comment.substr(opening.length());
            theComment += "\n";
        }
        else if (inDoc)
        {
            theComment += comment;
            theComment += "\n";
        }

        size_t pos = theComment.find(closing);
        if (pos != std::string::npos)
        {
            theComment.erase(pos);
            inDoc = false;
        }
        else
            inDoc = true;
    }
    size_t pos = theComment.find_first_not_of(" \t\n");
    if (pos != std::string::npos && pos > 0 && pos < theComment.length())
        return theComment.substr(pos);

    return theComment;
}


text ExtractComment::Pre(Tree *what)
// ----------------------------------------------------------------------------
//   Extract comments preceding current tree
// ----------------------------------------------------------------------------
{
    CommentsInfo *info = what->GetInfo<CommentsInfo>();
    return info ? Extract(info->before) : "";
}


text ExtractComment::Post(Tree *what)
// ----------------------------------------------------------------------------
//   Extract comments following current tree
// ----------------------------------------------------------------------------
{
    CommentsInfo *info = what->GetInfo<CommentsInfo>();
    return info ? Extract(info->after) : "";
}


text ExtractComment::Do(Tree *what)
// ----------------------------------------------------------------------------
//   Extract the documentation (/*| |*/) from the current tree
// ----------------------------------------------------------------------------
{
    return Pre(what) + Post(what);
}


text ExtractComment::Do(Block *what)
// ----------------------------------------------------------------------------
//   Extract the documentation (/*| |*/) from a Block'
// ----------------------------------------------------------------------------
{
    return Pre(what) + Pre(what->child) + Post(what->child) + Post(what);
}


text ExtractComment::Do(Infix *what)
// ----------------------------------------------------------------------------
//   Extract the documentation (/*| |*/) from an Infix
// ----------------------------------------------------------------------------
{
    return Pre(what)
        + Pre(what->left) +  Post(what->left)
        + Pre(what->right) + Post(what->right)
        + Post(what);
}


text ExtractComment::Do(Prefix *what)
// ----------------------------------------------------------------------------
//   Extract the documentation (/*| |*/) from a prefix
// ----------------------------------------------------------------------------
{
    return Pre(what)
        + Pre(what->left) +  Post(what->left)
        + Pre(what->right) + Post(what->right)
        + Post(what);
}


text ExtractComment::Do(Postfix *what)
// ----------------------------------------------------------------------------
//   Extract the documentation (/*| |*/) from a postfix
// ----------------------------------------------------------------------------
{
    return Pre(what)
        + Pre(what->left) +  Post(what->left)
        + Pre(what->right) + Post(what->right)
        + Post(what);
}


text ExtractDoc::Do(Infix *what)
// ----------------------------------------------------------------------------
//   Extract the documentation (/** **/) from an Infix
// ----------------------------------------------------------------------------
// There are two specifics kinds of Infix:
// - the "->" defining a tree rewrite
// - The ":" specifying types (for parameters on the left of a "->")

{
    bool def = what->name == "->";
    if (def)
    {
        params_tree = 0;
        symbol.clear();
        syntax.clear();
        params.clear();
        if (Name *name = what->left->AsName())
            symbol = name->value;
    }
    bool param = params_tree > 0 && what->name == ":";

    text comment = Pre(what);
    if (def)
        params_tree++;
    text left = what->left->Do(this);
    if (def)
        params_tree--;

    if (!def)
        comment += left;

    text right = what->right->Do(this);
    comment += right;

    comment+= Post(what);

    if (param)
    {
        addParam(what->left->AsName(), what->right->AsName(), &comment);
    }
    if (def)
    {
        formatSyntax(what->left);
        return formatDoc(&comment);
    }

    return "";
}


text ExtractDoc::Do(Prefix *what)
// ----------------------------------------------------------------------------
//   Extract the documentation (/** **/) from a Prefix.
// ----------------------------------------------------------------------------
{
    if (params_tree && what && what->left && what->left->AsName())
        symbol = what->left->AsName()->value;
    return what->Do((ExtractComment *) this);
}


text ExtractDoc::Do(Postfix *what)
// ----------------------------------------------------------------------------
//   Extract the documentation (/** **/) from a Postfix.
// ----------------------------------------------------------------------------
{
    if ( params_tree && what && what->right && what->right->AsName())
        symbol = what->right->AsName()->value;

    return what->Do((ExtractComment *) this);
}


text ExtractDoc::formatSyntax(Tree *t)
// ----------------------------------------------------------------------------
//   Format the syntax documentation from the given tree
// ----------------------------------------------------------------------------
{
    std::ostringstream oss;
    Renderer render(oss);
    QFileInfo fi("system:nocomment.stylesheet");
    render.SelectStyleSheet( text(fi.absoluteFilePath().toUtf8().constData()) );
    render.RenderFile(t);

    syntax = oss.str();
    size_t begin;

    while ((begin = syntax.find('\n')) != std::string::npos )
    {
        syntax.erase(begin, 1);
    }

    return syntax;
}


text ExtractDoc::addParam(Name *name, Name *ptype, text *comment)
// ----------------------------------------------------------------------------
//   Add one parameter description to the list of parameters.
// ----------------------------------------------------------------------------
{
    if (name && ptype)
        params.append("  parameter \"").append(ptype->value).append("\", \"")
            .append(name->value).append("\", <<").append(*comment).append(">>\n");

    return params;
}


text ExtractDoc::formatDoc(text *c)
// ----------------------------------------------------------------------------
//   Finalize the documentation formating.
// ----------------------------------------------------------------------------
{
    // First line is synopsis
    size_t id1 = c->find('\n');
    text syno = c->substr(0, id1);

    // jump over the \n
    id1 += 1;
    // Second line is group
    size_t id2 = c->find('\n', id1);
    text group = c->substr(id1, id2 - id1);
    if (group.empty())
        group = defaultGroup;

    // jump over the \n
    id2 += 1;
    // Following text is description
    // @return describes the returned value
    // @see gives a reference
    size_t id3 = c->find("@return ", id2);
    size_t id4 = c->find("@see ", id2);
    size_t len_desc;
    text desc;
    text ret_doc;
    text see;
    // No @return nor @see : description goes till the end
    if (id3 == std::string::npos && id4 == std::string::npos )
        len_desc = std::string::npos;
    else if (id3 == std::string::npos)
    {
        // No @return :
        //    description goes till @see
        len_desc = id4 - id2;
        //    @see goes till the end
        see = c->substr(id4 + 5);
    }
    else if (id4 == std::string::npos)
    {
        // No @see :
        //    description goes till @return
        len_desc = id3 - id2;
        //    @return goes till the end
        ret_doc = c->substr(id3 + 8);
    }
    else if (id3 < id4)
    {
        // @return is before @see :
        //    description goes till @return
        len_desc = id3 - id2;
        //    @return
        ret_doc = c->substr(id3 + 8, id4 - id3 - 8);
        //    @see
        see = c->substr(id4 + 5);
    }
    else
    {
        // @see is before @return : description goes till @see
        len_desc = id4 - id2;
        //    @see
        see = c->substr(id4 + 5, id3 - id4 - 5);
        //    @return
        ret_doc = c->substr(id3 + 8);
    }

    // Extract the description
    desc = c->substr(id2, len_desc-1); // -1 cuts the last \n

    text doc = text("docname \"").append(symbol).append("\", \"").append(group)
               .append("\", do\n");
    doc.append("  dsyntax <<").append(syntax).append(">>\n");
    doc.append("  synopsis <<").append(syno).append(">>\n");
    doc.append("  description <<").append(desc).append(">>\n");

    if ( ! params.empty())
        doc.append("  parameters\n").append(params);

    if (id3 != std::string::npos)
    {
        size_t comma = ret_doc.find(',');
        doc.append("  return_value \"").append(ret_doc.substr(0, comma))
                .append("\", <<")
                .append(ret_doc.substr(comma+1, ret_doc.length() - comma -2 ))
                .append(">>\n");
    }
    if (id4 != std::string::npos)
    {
        doc.append("  see \"").append(see.substr(0,see.length()-1)).append("\"\n");
    }

    return doc;
}

TAO_END
