#ifndef NORMALIZE_H
#define NORMALIZE_H
// ****************************************************************************
//  normalize.h                                                     Tao project
// ****************************************************************************
//
//   File Description:
//
//     Put the XL program in 'normal' form, i.e. one that has some
//     properties useful for editing purpose
//
//
//
//
//
//
//
// ****************************************************************************
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************
//
//  Normal form has the following properties:
//
//  1) Negative real and integer constants
//     In standard XL, -1 parses as the prefix - operator applied to integer 1.
//     In normal form, it is represented with an integer -1.
//
//  2) Infix ; and \n dangling on the right
//     The standard XL parses \n and ; so that they cascade on the right
//     of the infix node. Some transformations may add infix \n on the left
//     of an infix \n (e.g. some copy-paste operations).
//     Normalization puts reattaches all \n on the right of their parents.

#include "tree.h"
#include "tao_tree.h"
#include "tree_cloning.h"

TAO_BEGIN

struct Widget;

struct Renormalize : TreeClone
// ----------------------------------------------------------------------------
//   Put the input program back in normal form
// ----------------------------------------------------------------------------
{
    Renormalize(Widget *widget);
    virtual ~Renormalize();

    Tree *DoPrefix(Prefix *what);
    Tree *DoPostfix(Postfix *what);
    Tree *DoInfix(Infix *what);
    Tree *DoBlock(Block *what);
};

TAO_END

#endif // NORMALIZE_H
