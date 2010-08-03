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
#include "parser.h"


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
        if (XL::CommentsInfo *cinfo = from->GetInfo<XL::CommentsInfo>())
            to->SetInfo<XL::CommentsInfo> (new XL::CommentsInfo(*cinfo));
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


struct CopySelection : Action
// ----------------------------------------------------------------------------
//   Create a copy of the selected trees.
// ----------------------------------------------------------------------------
{
    CopySelection(Widget *w) : Action(), widget(w), clone(){}
    virtual ~CopySelection(){}

    Tree *DoInteger(Integer *what)
    {
        if (widget->selected(what))
            return what->Do(clone);

        return NULL;
    }
    Tree *DoReal(Real *what)
    {
        if (widget->selected(what))
            return  what->Do(clone);

        return NULL;
    }
    Tree *DoText(Text *what)
    {
        if (widget->selected(what))
            return  what->Do(clone);

        return NULL;
    }
    Tree *DoName(Name *what)
    {
        if (widget->selected(what))
            return  what->Do(clone);

        return NULL;
    }

    Tree *DoBlock(Block *what)
    {
        if (widget->selected(what))
            return  what->Do(clone);

        return  Do(what->child);
    }
    Tree *DoInfix(Infix *what)
    {
        if (widget->selected(what))
            return  what->Do(clone);

        Tree * l = Do(what->left);
        Tree * r = Do(what->right);

        if (!l) return r;
        if (!r) return l;

        return new Infix(what->name, l, r);

    }
    Tree *DoPrefix(Prefix *what)
    {
        if (widget->selected(what))
            return  what->Do(clone);

        return  Do(what->right);
    }
    Tree *DoPostfix(Postfix *what)
    {
        if (widget->selected(what))
            return what->Do(clone);

        return  Do(what->left);
    }
    Tree *Do(Tree *what)
    {
        return what->Do(*this);
    }

    Widget *widget;
    XL::TreeClone clone;
};

TAO_END
#endif // TREE_CLONING_H