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
    ApplyChanges (Tree *r): replace(r) {}
    Tree *DoInteger(Integer *what)
    {
        if (Integer *it = replace->AsInteger())
        {
            what->value = it->value;
            return what;
        }
        IFTRACE(filesync)
            std::cerr << "Integer " << (Tree *) what 
                      << " replaced with  " << (Tree *) replace << '\n';
        return NULL;
    }
    Tree *DoReal(Real *what)
    {
        if (Real *rt = replace->AsReal())
        {
            what->value = rt->value;
            return what;
        }
        IFTRACE(filesync)
            std::cerr << "Real " << (Tree *) what 
                      << " replaced with  " << (Tree *) replace << '\n';
        return NULL;
    }
    Tree *DoText(Text *what)
    {
        if (Text *tt = replace->AsText())
        {
            what->value = tt->value;
            return what;
        }
        IFTRACE(filesync)
            std::cerr << "Text " << (Tree *) what 
                      << " replaced with  " << (Tree *) replace << '\n';
        return NULL;
    }
    Tree *DoName(Name *what)
    {
        if (Name *nt = replace->AsName())
            if (nt->value == what->value)
                return what;
        return NULL;
    }

    Tree *DoBlock(Block *what)
    {
        if (Block *bt = replace->AsBlock())
        {
            if (bt->opening == what->opening &&
                bt->closing == what->closing)
            {
                replace = bt->child;
                Tree *br = what->child->Do(this);
                replace = bt;
                if (br)
                    return br;
                return NULL;
            }
        }
        IFTRACE(filesync)
            std::cerr << "Block " << (Tree *) what 
                      << " replaced with  " << (Tree *) replace << '\n';
        return NULL;
    }
    Tree *DoInfix(Infix *what)
    {
        if (Infix *it = replace->AsInfix())
        {
            if (it->name == what->name)
            {
                // For a declaration, we can't change test values
                if (it->name == "->")
                {
                    if (!EqualTrees(it->left, what->left))
                        return NULL;
                }
                else
                {
                    replace = it->left;
                    Tree *lr = what->left->Do(this);
                    replace = it;
                    if (!lr)
                        return NULL;
                }
                replace = it->right;
                Tree *rr = what->right->Do(this);
                replace = it;
                if (!rr)
                    return NULL;
                return what;
            }
        }
        IFTRACE(filesync)
            std::cerr << "Infix " << (Tree *) what 
                      << " replaced with  " << (Tree *) replace << '\n';
        return NULL;
    }
    Tree *DoPrefix(Prefix *what)
    {
        if (Prefix *pt = replace->AsPrefix())
        {
            replace = pt->left;
            Tree *lr = what->left->Do(this);
            replace = pt;
            if (!lr)
                return NULL;
            replace = pt->right;
            Tree *rr = what->right->Do(this);
            replace = pt;
            if (!rr)
                return NULL;
            return what;
        }
        IFTRACE(filesync)
            std::cerr << "Prefix " << (Tree *) what 
                      << " replaced with  " << (Tree *) replace << '\n';
        return NULL;
    }
    Tree *DoPostfix(Postfix *what)
    {
        if (Postfix *pt = replace->AsPostfix())
        {
            replace = pt->right;
            Tree *rr = what->right->Do(this);
            replace = pt;
            if (!rr)
                return NULL;
            replace = pt->left;
            Tree *lr = what->left->Do(this);
            replace = pt;
            if (!lr)
                return NULL;
            return what;
        }
        IFTRACE(filesync)
            std::cerr << "Postfix " << (Tree *) what 
                      << " replaced with  " << (Tree *) replace << '\n';
        return NULL;
    }
    Tree *Do(Tree * )
    {
        return NULL;
    }

    Tree_p  replace;
};


struct IsMarkedConstantInfo : XL::Info
// ----------------------------------------------------------------------------
//   Indicate whether a tree is constant or not
// ----------------------------------------------------------------------------
{};


inline bool IsMarkedConstant(Tree *tree)
// ----------------------------------------------------------------------------
//   Indicates if a tree is marked as constant
// ----------------------------------------------------------------------------
{
    return tree->Exists<IsMarkedConstantInfo>();
}


inline void MarkAsConstant(Tree* tree)
// ----------------------------------------------------------------------------
//   Mark the tree as constant
// ----------------------------------------------------------------------------
{
    if (!IsMarkedConstant(tree))
        tree->SetInfo<IsMarkedConstantInfo>(new IsMarkedConstantInfo());
}


typedef std::set<XL::SourceFile *> import_set;
bool ImportedFilesChanged(import_set &done,
                          bool markChanged);


TAO_END

#endif // APPLY_CHANGES_H
