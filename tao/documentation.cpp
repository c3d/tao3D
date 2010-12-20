// ****************************************************************************
//  documentation.cpp						   Tao project
// ****************************************************************************
//
//   File Description:
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
// ****************************************************************************
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Catherine Burvelle <cathy@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************
#include "documentation.h"
#include "base.h"
#include "renderer.h"
#include <sstream>

text ExtractDoc::extract(XL::CommentsList commentList)
// ----------------------------------------------------------------------------
//   Extract the documentation (/*| |*/) from the comments list.
// ----------------------------------------------------------------------------
{
    bool inDoc = false;
    text theComment = "";
    for (uint i = 0; i < commentList.size(); i++)
    {
        if (commentList[i].find("/*|") == 0)
        {
            inDoc = true;
            theComment += commentList[i].substr(3);
            theComment += "\n";
        }
        else if (inDoc)
        {
            theComment += commentList[i];
            theComment += "\n";
        }

        size_t pos = theComment.find("|*/");
        if (pos != std::string::npos)
        {
            theComment.erase(pos);
           // theComment += "\n";
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


XL::Tree *ExtractDoc::DoInteger(XL::Integer *what)
// ----------------------------------------------------------------------------
//   Extract the documentation (/*| |*/) from an Integer.
// ----------------------------------------------------------------------------
{
    text comment = "";
    XL::CommentsInfo *info = what->GetInfo<XL::CommentsInfo>();
    if (info)
        comment = extract(info->before) + extract(info->after);
    return new XL::Text(comment,"","");

}


XL::Tree *ExtractDoc::DoReal(XL::Real *what)
// ----------------------------------------------------------------------------
//   Extract the documentation (/*| |*/) from a Real.
// ----------------------------------------------------------------------------
{
    text comment = "";
    XL::CommentsInfo *info = what->GetInfo<XL::CommentsInfo>();
    if (info)
        comment = extract(info->before) + extract(info->after);
    return new XL::Text(comment,"","");
}


XL::Tree *ExtractDoc::DoText(XL::Text *what)
// ----------------------------------------------------------------------------
//   Extract the documentation (/*| |*/) from a Text.
// ----------------------------------------------------------------------------
{
    text comment = "";
    XL::CommentsInfo *info = what->GetInfo<XL::CommentsInfo>();
    if (info)
        comment = extract(info->before) + extract(info->after);
    return new XL::Text(comment,"","");
}


XL::Tree *ExtractDoc::DoName(XL::Name *what)
// ----------------------------------------------------------------------------
//   Extract the documentation (/*| |*/) from a Name.
// ----------------------------------------------------------------------------
{
    text comment = "";
    XL::CommentsInfo *info = what->GetInfo<XL::CommentsInfo>();
    if (info)
        comment = extract(info->before) + extract(info->after);
    return new XL::Text(comment,"","");
}


XL::Tree *ExtractDoc::DoBlock(XL::Block *what)
// ----------------------------------------------------------------------------
//   Extract the documentation (/*| |*/) from a Block'
// ----------------------------------------------------------------------------
{
    text comment = "";
    XL::CommentsInfo *info = what->GetInfo<XL::CommentsInfo>();
    if (info)
        comment = extract(info->before);

    XL::Tree *child = what->child->Do(this);
    XL::Text *c = NULL;
    if (child && (c = child->AsText()))
        comment += c->value;

    if (info)
        comment+= extract(info->after);

    return new XL::Text(comment,"","");
}

XL::Tree *ExtractDoc::DoInfix(XL::Infix *what)
// ----------------------------------------------------------------------------
//   Extract the documentation (/*| |*/) from an Infix
// ----------------------------------------------------------------------------
{
    // There are two specifics kind of Infix : the "->" that define a transformation
    // and ":" on the left side of "->" that define a parameter

    bool def =  (what->name.find("->") != std::string::npos);
    if (def)
    {
        params_tree = 0;
        symbol.clear();
        syntax.clear();
        params.clear();
        if (XL::Name *name = what->left->AsName())
            symbol = name->value;
    }
    bool param = params_tree > 0 && (what->name.find(":") != std::string::npos);

    XL::Text *c = NULL;
    text comment = "";
    XL::CommentsInfo *info = what->GetInfo<XL::CommentsInfo>();
    if (info)
        comment = extract(info->before);

    if (def ) params_tree++;
    XL::Tree *left = what->left->Do(this);
    if (def) params_tree--;
    if (!def && left && (c = left->AsText()))
        comment += c->value;

    XL::Tree *right = what->right->Do(this);
    if (right && (c = right->AsText()))
        comment += c->value;

    if (info)
        comment+= extract(info->after);

    if (param)
    {
        addParam(what->left->AsName(), what->right->AsName(), &comment);
    }
    if (def)
    {
        formatSyntax(what->left);
        return new XL::Text(formatDoc(&comment));
    }

    return new XL::Text(comment,"","");
}


XL::Tree *ExtractDoc::DoPrefix(XL::Prefix *what)
// ----------------------------------------------------------------------------
//   Extract the documentation (/*| |*/) from a Prefix.
// ----------------------------------------------------------------------------
{
    if ( params_tree && what && what->left && what->left->AsName())
        symbol = what->left->AsName()->value;

    text comment = "";
    XL::Text *c = NULL;
    XL::CommentsInfo *info = what->GetInfo<XL::CommentsInfo>();
    if (info)
        comment = extract(info->before);

    XL::Tree *left = what->left->Do(this);
    if (left && (c = left->AsText()))
        comment += c->value;

    XL::Tree *right = what->right->Do(this);
    if (right && (c = right->AsText()))
        comment += c->value;

    if (info)
        comment+= extract(info->after);

    return new XL::Text(comment,"","");
}


XL::Tree *ExtractDoc::DoPostfix(XL::Postfix *what)
// ----------------------------------------------------------------------------
//   Extract the documentation (/*| |*/) from a Postfix.
// ----------------------------------------------------------------------------
{
    if ( params_tree && what && what->right && what->right->AsName())
        symbol = what->right->AsName()->value;

    text comment = "";
    XL::Text *c = NULL;
    XL::CommentsInfo *info = what->GetInfo<XL::CommentsInfo>();
    if (info)
        comment = extract(info->before);

    XL::Tree *left = what->left->Do(this);
    if (left && (c = left->AsText()))
        comment += c->value;

    XL::Tree *right = what->right->Do(this);
    if (right && (c = right->AsText()))
        comment += c->value;

    if (info)
        comment+= extract(info->after);

    return new XL::Text(comment,"","");
}


text ExtractDoc::formatSyntax(XL::Tree *t)
// ----------------------------------------------------------------------------
//   Format the syntax documentation from the given tree
// ----------------------------------------------------------------------------
{
    std::ostringstream oss;
    oss << t;
    syntax = oss.str();
    size_t begin, end;
    while ((begin = syntax.find("/*|")) != std::string::npos )
    {
        end = syntax.find("|*/");
        syntax.erase(begin, (end+3)-begin);
    }
    while ((begin = syntax.find('\n')) != std::string::npos )
    {
        syntax.erase(begin, 1);
    }

    return syntax;
}

text ExtractDoc::addParam(XL::Name *name, XL::Name *ptype, text *comment)
// ----------------------------------------------------------------------------
//   Add one parameter description to the list of parameters.
// ----------------------------------------------------------------------------
{
    params.append("    parameter \"").append(ptype->value).append("\", \"")
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
