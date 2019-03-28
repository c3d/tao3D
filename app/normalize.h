#ifndef NORMALIZE_H
#define NORMALIZE_H
// *****************************************************************************
// normalize.h                                                     Tao3D project
// *****************************************************************************
//
// File description:
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
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2010, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010-2011,2014,2019, Christophe de Dinechin <christophe@dinechin.org>
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
