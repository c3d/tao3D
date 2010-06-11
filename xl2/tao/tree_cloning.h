#ifndef TREE_CLONING_H
#define TREE_CLONING_H
// ****************************************************************************
//  tree_cloning						   Tao project
// ****************************************************************************
//
//   File Description:
//
//     manage copy of tree with selection awarness.
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
//  (C) 2010 Catherine Burvelle <cathy@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "tao_tree.h"
#include "tree.h"
#include "widget.h"


TAO_BEGIN
struct Widget;

// ============================================================================
//
//    Tree cloning
//
// ============================================================================

typedef struct XL::DeepCopyCloneMode DeepCopyCloneMode;
typedef struct XL::ShallowCopyCloneMode ShallowCopyCloneMode;
typedef struct XL::NodeOnlyCloneMode NodeOnlyCloneMode;


template <typename mode> struct TaoCloneTemplate : Action
// ----------------------------------------------------------------------------
//   Clone a tree
// ----------------------------------------------------------------------------
{
    TaoCloneTemplate(Widget *widget) : widget(widget){}
    virtual ~TaoCloneTemplate(){}

    Tree *Reselect(Tree *from, Tree *to)
    {
        widget->reselect(from, to);
        return to;
    }

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
        return Reselect(what, new Text(what->value,
                                       what->opening,
                                       what->closing,
                                       what->Position()));
    }
    Tree *DoName(Name *what)
    {
        return Reselect(what, new Name(what->value, what->Position()));
    }

    Tree *DoBlock(Block *what)
    {
        return  Reselect(what, new Block(Clone(what->child),
                                         what->opening,
                                         what->closing,
                                         what->Position()));
    }
    Tree *DoInfix(Infix *what)
    {
        return Reselect(what,  new Infix (what->name,
                                          Clone(what->left),
                                          Clone(what->right),
                                          what->Position()));
    }
    Tree *DoPrefix(Prefix *what)
    {
        return  Reselect(what, new Prefix(Clone(what->left),
                                          Clone(what->right),
                                          what->Position()));
    }
    Tree *DoPostfix(Postfix *what)
    {
        return  Reselect(what, new Postfix(Clone(what->left),
                                           Clone(what->right),
                                           what->Position()));
    }
    Tree *Do(Tree *what)
    {
        return what;            // ??? Should not happen
    }
protected:
    // Default is to do a deep copy
    Tree *  Clone(Tree *t) { return t->Do(this); }

    Widget *widget;

};


template<> inline
Tree *TaoCloneTemplate<ShallowCopyCloneMode>::Clone(Tree *t)
// ----------------------------------------------------------------------------
//   Specialization for the shallow copy clone
// ----------------------------------------------------------------------------
{
    return t;
}


template<> inline
Tree *TaoCloneTemplate<NodeOnlyCloneMode>::Clone(Tree *)
// ----------------------------------------------------------------------------
//   Specialization for the node-only clone
// ----------------------------------------------------------------------------
{
    return NULL;
}


typedef struct TaoCloneTemplate<DeepCopyCloneMode>   TaoTreeClone;
typedef struct TaoCloneTemplate<ShallowCopyCloneMode>TaoShallowCopyTreeClone;
typedef struct TaoCloneTemplate<NodeOnlyCloneMode>    TaoNodeOnlyTreeClone;


TAO_END
#endif // TREE_CLONING_H
