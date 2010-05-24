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
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
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

TAO_BEGIN

struct Widget;

struct Renormalize : XL::Action
// ----------------------------------------------------------------------------
//   Put the input program back in normal form
// ----------------------------------------------------------------------------
{
    Renormalize(Widget *widget);
    virtual ~Renormalize();

    Tree *        Reselect(Tree *old, Tree *cloned);
    virtual Tree *DoPrefix(Prefix *what);
    virtual Tree *DoPostfix(Postfix *what);
    virtual Tree *DoInfix(Infix *what);

    Tree *DoInteger(Integer *what)
    {
        return Reselect(what, new Integer(what->value, what->Position()));
    }
    Tree *DoReal(Real *what)
    {
        return Reselect(what, new Real(what->value, what->Position()));

    }
    Tree *DoText(Text *what)
    {
        return Reselect(what,
                        new Text(what->value, what->opening, what->closing,
                                 what->Position()));
    }
    Tree *DoName(Name *what)
    {
        return Reselect(what, new Name(what->value, what->Position()));
    }

    Tree *DoBlock(Block *what)
    {
        return Reselect(what,
                        new Block(what->child->Do(this),
                                  what->opening, what->closing,
                                  what->Position()));
    }
    Tree *Do(Tree *what)
    {
        return what;            // Should not happen
    }

protected:
    Widget *widget;
};

TAO_END

#endif // NORMALIZE_H
