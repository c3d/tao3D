#ifndef APPLY_CHANGES_H
#define APPLY_CHANGES_H
// ****************************************************************************
//  apply_changes.h                                                 Tao project
// ****************************************************************************
//
//   File Description:
//
//    Check if we can leverage the same generated code between two trees
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
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "tao.h"
#include "tao_tree.h"
#include "main.h"
#include <set>

TAO_BEGIN

struct ApplyChanges : XL::Action
// ----------------------------------------------------------------------------
//   Check if we can apply changes from another tree on the current one
// ----------------------------------------------------------------------------
{
    ApplyChanges (Tree_p r): replace(r) {}
    Tree_p DoInteger(Integer_p what)
    {
        if (Integer_p it = replace->AsInteger())
        {
            what->value = it->value;
            return what;
        }
        return NULL;
    }
    Tree_p DoReal(Real_p what)
    {
        if (Real_p rt = replace->AsReal())
        {
            what->value = rt->value;
            return what;
        }
        return NULL;
    }
    Tree_p DoText(Text_p what)
    {
        if (Text_p tt = replace->AsText())
        {
            what->value = tt->value;
            return what;
        }
        return NULL;
    }
    Tree_p DoName(Name_p what)
    {
        if (Name_p nt = replace->AsName())
            if (nt->value == what->value)
                return what;
        return NULL;
    }

    Tree_p DoBlock(Block_p what)
    {
        if (Block_p bt = replace->AsBlock())
        {
            if (bt->opening == what->opening &&
                bt->closing == what->closing)
            {
                replace = bt->child;
                Tree_p br = what->child->Do(this);
                replace = bt;
                if (br)
                    return br;
            }
        }
        return NULL;
    }
    Tree_p DoInfix(Infix_p what)
    {
        if (Infix_p it = replace->AsInfix())
        {
            if (it->name == what->name)
            {
                // For a declaration, we can't change test values
                if (it->name == "->")
                {
                    XL::TreeMatch identical(it->left);
                    Tree_p lr = what->left->Do(identical);
                    if (!lr)
                        return NULL;
                }
                else
                {
                    replace = it->left;
                    Tree_p lr = what->left->Do(this);
                    replace = it;
                    if (!lr)
                        return NULL;
                }
                replace = it->right;
                Tree_p rr = what->right->Do(this);
                replace = it;
                if (!rr)
                    return NULL;
                return what;
            }
        }
        return NULL;
    }
    Tree_p DoPrefix(Prefix_p what)
    {
        if (Prefix_p pt = replace->AsPrefix())
        {
            replace = pt->left;
            Tree_p lr = what->left->Do(this);
            replace = pt;
            if (!lr)
                return NULL;
            replace = pt->right;
            Tree_p rr = what->right->Do(this);
            replace = pt;
            if (!rr)
                return NULL;
            return what;
        }
        return NULL;
    }
    Tree_p DoPostfix(Postfix_p what)
    {
        if (Postfix_p pt = replace->AsPostfix())
        {
            replace = pt->right;
            Tree_p rr = what->right->Do(this);
            replace = pt;
            if (!rr)
                return NULL;
            replace = pt->left;
            Tree_p lr = what->left->Do(this);
            replace = pt;
            if (!lr)
                return NULL;
            return what;
        }
        return NULL;
    }
    Tree_p Do(Tree_p )
    {
        return NULL;
    }

    Tree_p  replace;
};


struct PruneInfo : XL::Action
// ----------------------------------------------------------------------------
//    Remove info that we don't want to persist if we change programs
// ----------------------------------------------------------------------------
{
    PruneInfo() : XL::Action() {}
    XL::Tree_p Do(XL::Tree_p what);
};


typedef std::set<XL::SourceFile *> import_set;
bool ImportedFilesChanged(XL::Tree_p prog,
                          import_set &done,
                          bool markChanged);


TAO_END

#endif // APPLY_CHANGES_H
