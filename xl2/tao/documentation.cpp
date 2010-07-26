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

text ExtractDoc::extract(XL::CommentsList commentList)
// ----------------------------------------------------------------------------
//   Extract the documentation relative (/*| |*/) comments from the list
// ----------------------------------------------------------------------------
{
    bool inDoc = false;
    text theComment = "";
    for ( uint i = 0; i < commentList.size(); i++)
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
            theComment += "\n";
            inDoc = false;
        }
        else
            inDoc = true;
    }
    return theComment;
}


XL::Tree *ExtractDoc::DoInteger(XL::Integer *what)
{
    text comment = "";
    XL::CommentsInfo *info = what->GetInfo<XL::CommentsInfo>();
    if (info)
        comment = extract(info->before) + extract(info->after);
    return new XL::Text(comment,"","");

}


XL::Tree *ExtractDoc::DoReal(XL::Real *what)
{
    text comment = "";
    XL::CommentsInfo *info = what->GetInfo<XL::CommentsInfo>();
    if (info)
        comment = extract(info->before) + extract(info->after);
    return new XL::Text(comment,"","");
}


XL::Tree *ExtractDoc::DoText(XL::Text *what)
{
    text comment = "";
    XL::CommentsInfo *info = what->GetInfo<XL::CommentsInfo>();
    if (info)
        comment = extract(info->before) + extract(info->after);
    return new XL::Text(comment,"","");
}


XL::Tree *ExtractDoc::DoName(XL::Name *what)
{
    text comment = "";
    XL::CommentsInfo *info = what->GetInfo<XL::CommentsInfo>();
    if (info)
        comment = extract(info->before) + extract(info->after);
    return new XL::Text(comment,"","");
}


XL::Tree *ExtractDoc::DoBlock(XL::Block *what)
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
{
    XL::Text *c = NULL;
    text comment = "";
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


XL::Tree *ExtractDoc::DoPrefix(XL::Prefix *what)
{
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
{
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
