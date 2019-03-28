#ifndef APPLY_CHANGES_H
#define APPLY_CHANGES_H
// *****************************************************************************
// apply_changes.h                                                 Tao3D project
// *****************************************************************************
//
// File description:
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
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2010, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010,2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2012, Jérôme Forissier <jerome@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can r redistribute it and/or modify
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

#include "tao.h"
#include "tao_tree.h"
#include "main.h"
#include "action.h"
#include <set>

TAO_BEGIN

struct ApplyChanges
// ----------------------------------------------------------------------------
//   Check if we can apply changes from another tree on the current one
// ----------------------------------------------------------------------------
{
    ApplyChanges (Tree *r): replace(r) {}
    Tree *Do(Integer *what)
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
    Tree *Do(Real *what)
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
    Tree *Do(Text *what)
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
    Tree *Do(Name *what)
    {
        if (Name *nt = replace->AsName())
            if (nt->value == what->value)
                return what;
        return NULL;
    }

    Tree *Do(Block *what)
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
    Tree *Do(Infix *what)
    {
        if (Infix *it = replace->AsInfix())
        {
            if (it->name == what->name)
            {
                // For a declaration, we can't change test values
                if (it->name == "->")
                {
                    if (!Tree::Equal(it->left, what->left))
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
    Tree *Do(Prefix *what)
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
    Tree *Do(Postfix *what)
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
void ScanImportedFiles(import_set &done,  bool markChanged);


TAO_END

#endif // APPLY_CHANGES_H
