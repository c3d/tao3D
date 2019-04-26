// *****************************************************************************
// normalize.cpp                                                   Tao3D project
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
// (C) 2010-2011,2013-2014,2019, Christophe de Dinechin <christophe@dinechin.org>
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

#include "normalize.h"
#include "widget.h"

TAO_BEGIN

Renormalize::Renormalize(Widget *widget)
// ----------------------------------------------------------------------------
//   Constructor, nothing special to do
// ----------------------------------------------------------------------------
    : TreeClone()
{
    this->widget = widget;
}


Renormalize::~Renormalize()
// ----------------------------------------------------------------------------
//   Destructor, nothing special to do
// ----------------------------------------------------------------------------
{}



Tree *Renormalize::DoPrefix(Prefix *what)
// ----------------------------------------------------------------------------
//   Detect prefix - with a real or integer child
// ----------------------------------------------------------------------------
{
    if (Name *leftName = what->left->AsName())
    {
        if (leftName->value == "-")
        {
            if (Real *rr = what->right->AsReal())
                return Reselect(what,
                                new Real(-rr->value, what->Position()));
            if (Integer *ir = what->right->AsInteger())
                return Reselect(what,
                                new Integer(-ir->value, what->Position()));
        }
    }
    return Reselect(what, new Prefix(what->left->Do(this),
                                     what->right->Do(this),
                                     what->Position()));
}


Tree *Renormalize::DoPostfix(Postfix *what)
// ----------------------------------------------------------------------------
//   No special treatment of postfix values so far...
// ----------------------------------------------------------------------------
{
    return Reselect(what, new Postfix(what->left->Do(this),
                                      what->right->Do(this),
                                      what->Position()));
}


Tree *Renormalize::DoInfix(Infix *what)
// ----------------------------------------------------------------------------
//   Detect infix \n or \; with a similar node on its left
// ----------------------------------------------------------------------------
{
    // Put left and right in normal form
    Tree  *left   = what->left->Do(this);
    Tree  *right  = what->right->Do(this);
    Infix *result = NULL;

    // Look for \n and ; with abnormal form
    if (what->name == "\n" || what->name == ";")
    {
        if (Infix *il = left->AsInfix())
        {
            if (il->name == what->name)
            {
                // Loop on the right to find where we want to attach
                Infix *last = il->LastStatement();

                // Connect the bottom of what we had on the left
                last->right = new Infix(what->name, last->right, right,
                                        what->Position());

                // Build the top tree of the result
                result = new Infix(what->name, il->left, il->right,
                                   what->Position());
            }
        }
    }

    // Otherwise, return clone
    if (!result)
        result = new Infix(what->name, left, right, what->Position());

    return Reselect(what, result);
}


Tree *Renormalize::DoBlock(Block *what)
// ----------------------------------------------------------------------------
//   Detect selection changes on a block
// ----------------------------------------------------------------------------
{
    return Reselect(what,
                    new Block(what->child->Do(this),
                              what->opening, what->closing,
                              what->Position()));
}

TAO_END
