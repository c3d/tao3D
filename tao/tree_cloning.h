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
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
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

template <typename CloneMode>
struct WidgetCloneMode : CloneMode
// ----------------------------------------------------------------------------
//   A special way to clone where we reselect items in a given widget
// ----------------------------------------------------------------------------
{
    WidgetCloneMode() : widget(NULL) {}
    Tree *Reselect(Tree *from, Tree *to)
    {
        widget->reselect(from, to);
        return to;
    }
    template<typename CloneClass>
    Tree *Clone(Tree *from, CloneClass *clone)
    {
        Tree *to = CloneMode::Clone(from, clone);
        return Reselect(from, to);
    }
    Widget *widget;
};


typedef WidgetCloneMode <XL::DeepCloneMode>             DeepCloneMode;
typedef WidgetCloneMode <XL::ShallowCloneMode>          ShallowCloneMode;
typedef WidgetCloneMode <XL::NullCloneMode>             NullCloneMode;

typedef XL::TreeCloneTemplate<DeepCloneMode>            TreeClone;
typedef XL::TreeCloneTemplate<ShallowCloneMode>         ShallowClone;
typedef XL::TreeCloneTemplate<NullCloneMode>            NullClone;


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

struct NameChangeCloneMode
// ----------------------------------------------------------------------------
//   Clone mode where DoName is virtual so that we can override it
// ----------------------------------------------------------------------------
{
    virtual XL::Tree *DoName(XL::Name *what) = 0;
    template<typename CloneClass>
    XL::Tree *Clone(Tree *t, CloneClass *clone) { return t->Do(clone); }
};
typedef XL::TreeCloneTemplate<NameChangeCloneMode>      NameChangeClone;


struct ColorTreeClone : NameChangeClone
// ----------------------------------------------------------------------------
//  Override names 'red', 'green', 'blue' and 'alpha' in the input tree
// ----------------------------------------------------------------------------
{
    ColorTreeClone(const QColor &c): color(c) {}
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


struct FontTreeClone : NameChangeClone
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


struct ToggleTreeClone : NameChangeClone
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


struct ClickTreeClone : NameChangeClone
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

XL_END
#endif // TREE_CLONING_H
