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
#include "tao_utf8.h"


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


template <typename mode> struct TaoCloneTemplate
// ----------------------------------------------------------------------------
//   Clone a tree
// ----------------------------------------------------------------------------
{
    TaoCloneTemplate(Widget *widget) : widget(widget){}
    virtual ~TaoCloneTemplate(){}

    typedef Tree *value_type;

    Tree *Reselect(Tree *from, Tree *to)
    {
        widget->reselect(from, to);
        if (XL::CommentsInfo *cinfo = from->GetInfo<XL::CommentsInfo>())
            to->SetInfo<XL::CommentsInfo> (new XL::CommentsInfo(*cinfo));
        if (XL::Symbols *symbols = from->Symbols())
            to->SetSymbols(symbols);
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
typedef struct TaoCloneTemplate<NodeOnlyCloneMode>   TaoNodeOnlyTreeClone;


struct CopySelection
// ----------------------------------------------------------------------------
//   Create a copy of the selected trees.
// ----------------------------------------------------------------------------
{
    CopySelection(Widget *w) : widget(w), clone(){}
    virtual ~CopySelection(){}

    typedef Tree *value_type;

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



// ============================================================================
// 
//   Some useful specializations to manipulate parameter trees
// 
// ============================================================================

struct ColorTreeClone : XL::TreeClone
// ----------------------------------------------------------------------------
//  Override names 'red', 'green', 'blue' and 'alpha' in the input tree
// ----------------------------------------------------------------------------
{
    ColorTreeClone(const QColor &c): color(c){}
    XL::Tree *DoName(XL::Name *what)
    {
        if (what->value == "red")
            return new XL::Real(color.redF(), what->Position());
        if (what->value == "green")
            return new XL::Real(color.greenF(), what->Position());
        if (what->value == "blue")
            return new XL::Real(color.blueF(), what->Position());
        if (what->value == "alpha")
            return new XL::Real(color.alphaF(), what->Position());
        
        return new XL::Name(what->value, what->Position());
    }
    QColor color;
};


struct FontTreeClone : XL::TreeClone
// ----------------------------------------------------------------------------
//   Overrides font description names in the input tree
// ----------------------------------------------------------------------------
{
    FontTreeClone(const QFont &f) : font(f){}
    XL::Tree *DoName(XL::Name *what)
    {
        if (what->value == "font_family")
            return new XL::Text(+font.family(),
                                "\"" ,"\"",what->Position());
        if (what->value == "font_size")
            return new XL::Integer(font.pointSize(), what->Position());
        if (what->value == "font_weight")
            return new XL::Integer(font.weight(), what->Position());
        if (what->value == "font_slant")
            return new XL::Integer((int) font.style() * 100,
                                   what->Position());
        if (what->value == "font_stretch")
            return new XL::Integer(font.stretch(), what->Position());
        if (what->value == "font_is_italic")
            return font.italic() ? XL::xl_true : XL::xl_false;
        if (what->value == "font_is_bold")
            return font.bold() ? XL::xl_true : XL::xl_false;
        
        return new XL::Name(what->value, what->Position());
    }
    QFont font;
};


struct ToggleTreeClone : XL::TreeClone
// ----------------------------------------------------------------------------
//   Override the name "checked" in the input tree
// ----------------------------------------------------------------------------
{
    ToggleTreeClone(bool c) : checked(c){}
    XL::Tree *DoName(XL::Name *what)
    {
        if (what->value == "checked")
        {
            if (checked)
                return XL::xl_true;
            else
                return XL::xl_false;
        }
        return new XL::Name(what->value, what->Position());
    }
    bool checked;
};


struct ClickTreeClone : XL::TreeClone
// ----------------------------------------------------------------------------
//  Override name "button_name" in the input tree
// ----------------------------------------------------------------------------
{
    ClickTreeClone(text c) : name(c){}
    XL::Tree * DoName(XL::Name * what)
    {
        if (what->value == "button_name")
        {
            return new XL::Text(name);
        }
        return new XL::Name(what->value, what->Position());
    }
    text name;
};

TAO_END
#endif // TREE_CLONING_H
