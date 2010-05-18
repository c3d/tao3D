// ****************************************************************************
//  normalize.cpp                                                   Tao project
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
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Jérôme Forissier <jerome@taodyne.com>
//  (C) 2010 Catherine Burvelle <cathy@taodyne.com>
//  (C) 2010 Lionel Schaffhauser <lionel@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "normalize.h"
#include "widget.h"

TAO_BEGIN

Renormalize::Renormalize(Widget *widget)
// ----------------------------------------------------------------------------
//   Constructor, nothing special to do
// ----------------------------------------------------------------------------
    : XL::TreeClone(), widget(widget)
{}


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
                return new Real(-rr->value, what->Position());
            if (Integer *ir = what->right->AsInteger())
                return new Integer(-ir->value, what->Position());
        }
    }
    return XL::TreeClone::DoPrefix(what);
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
            if (il->name == "\n" || il->name == ";")
            {
                // Loop on the right to find where we want to attach
                Infix *last = il->LastStatement();

                // Build the top tree of the result
                result = new Infix(what->name, il->left, il->right,
                                   what->Position());

                // Disconnect what we had on the left, now useless
                il->left = NULL;
                il->right = NULL;

                // Connect the bottom of what we had on the left
                last->right = new Infix(what->name,
                                        last->right, right,
                                        what->Position());
            }
        }
    }

    // Otherwise, return clone
    if (!result)
        result = new Infix(what->name, left, right, what->Position());

    // Check if we are possibly changing the selection
    std::set<Tree_p> &sel = widget->selectionTrees;
    if (sel.count(what->left))
        sel.insert(left);
    if (sel.count(what->right))
        sel.insert(right);
    if (sel.count(what))
        sel.insert(result);

    // Check if we are possibly changing the next selection
    std::set<Tree_p> &nxSel = widget->selectNextTime;
    if (nxSel.count(what->left))
        nxSel.insert(left);
    if (nxSel.count(what->right))
        nxSel.insert(right);
    if (nxSel.count(what))
        nxSel.insert(result);

    return result;
}


TAO_END
