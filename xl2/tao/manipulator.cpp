// ****************************************************************************
//  manipulator.cpp                                                 Tao project
// ****************************************************************************
// 
//   File Description:
// 
//    Helper class used to assign GL names to individual graphic shapes
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
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "manipulator.h"
#include "drag.h"
#include "layout.h"
#include "gl_keepers.h"
#include "runtime.h"

TAO_BEGIN

// ============================================================================
// 
//    Simple manipulator
// 
// ============================================================================

Manipulator::Manipulator()
// ----------------------------------------------------------------------------
//   Record the GL name for a given tree
// ----------------------------------------------------------------------------
{
}


void Manipulator::Draw(Layout *layout)
// ----------------------------------------------------------------------------
//   During drawing path, we don't draw nothing
// ----------------------------------------------------------------------------
{
    (void) layout;
}


void Manipulator::DrawSelection(Layout *layout)
// ----------------------------------------------------------------------------
//   Draw the manipulator using simple GL points
// ----------------------------------------------------------------------------
{
    Widget *widget = layout->Display();
    if (widget->selected())
    {
        glPushName(0);
        DrawHandles(layout);
        glPopName();
    }
}


void Manipulator::Identify(Layout *layout)
// ----------------------------------------------------------------------------
//   Draw the manipulator selection handles
// ----------------------------------------------------------------------------
{
    Manipulator::DrawSelection(layout);
}


bool Manipulator::DrawHandle(Layout *layout, Point3 p, uint id)
// ----------------------------------------------------------------------------
//   Draw one of the handles for the current manipulator
// ----------------------------------------------------------------------------
{
    Widget *widget = layout->Display();
    glLoadName(id);
    widget->drawHandle(p, "handle");
    glLoadName(0);
    bool    selected = widget->manipulator == id;
    return selected;
}


double Manipulator::updateArg(Widget *widget, tree_p arg, coord delta)
// ----------------------------------------------------------------------------
//   Update the given argument by the given offset
// ----------------------------------------------------------------------------
{
    // Defensive coding against bad callers...
    if (!arg || delta == 0.0)
        return delta;

    Tree   *source   = xl_source(arg); // Find the source expression
    tree_p *ptr      = &source;
    bool    more     = true;
    bool    negative = false;
    tree_p *pptr     = NULL;
    tree_p *ppptr    = NULL;
    double  scale    = 1.0;

    // Check if we have an Infix +, if so walk down the left side
    arg = source;
    while (more)
    {
        more = false;
        ppptr = pptr;
        pptr = NULL;
        if (XL::Infix *infix = (*ptr)->AsInfix())
        {
            if (infix->name == "-")
            {
                ptr = &infix->left;
                more = true;
            }
            else if (infix->name == "+")
            {
                ptr = &infix->left;
                more = true;
            }
            else if (infix->name == "*")
            {
                if (XL::Real *lr = infix->left->AsReal())
                {
                    scale *= lr->value;
                    ptr = &infix->right;
                }
                else if (XL::Real *rr = infix->right->AsReal())
                {
                    scale *= rr->value;
                    ptr = &infix->left;
                }
                else if (XL::Integer *li = infix->left->AsInteger())
                {
                    scale *= li->value;
                    ptr = &infix->right;
                }
                else if (XL::Integer *ri = infix->right->AsInteger())
                {
                    scale *= ri->value;
                    ptr = &infix->left;
                }
            }
        }
        if (XL::Prefix *prefix = (*ptr)->AsPrefix())
        {
            if (XL::Name *name = prefix->left->AsName())
            {
                if (name->value == "-")
                {
                    pptr = ptr;
                    ptr = &prefix->right;
                    more = true;
                    negative = !negative;
                    delta = -delta;
                }
            }
        }
    }
    if (scale == 0.0)
        scale = 1.0;
    delta /= scale;

    // Test the simple cases where the argument is directly an Integer or Real
    if (XL::Integer *ival = (*ptr)->AsInteger())
    {
        delta = longlong(delta);
        ival->value += delta;
        if (ppptr && ival->value < 0)
            widget->reloadProgram = true;
    }
    else if (XL::Real *rval = (*ptr)->AsReal())
    {
        rval->value += delta;
        if (ppptr && rval->value < 0)
            widget->reloadProgram = true;
    }
    else
    {
        // Create an Infix + with the delta we add
        if (ptr != &arg)
        {
            *ptr = new XL::Infix("+", new XL::Real(delta), *ptr);
            widget->reloadProgram = true;
        }
    }

    return delta;
}



// ============================================================================
// 
//    A control point updates specific coordinates
// 
// ============================================================================

ControlPoint::ControlPoint(real_r x, real_r y, uint id)
// ----------------------------------------------------------------------------
//   Record where we want to draw
// ----------------------------------------------------------------------------
    : Manipulator(), x(x), y(y), id(id)
{}


void ControlPoint::DrawHandles(Layout *layout)
// ----------------------------------------------------------------------------
//   For a control point, there is a single handle
// ----------------------------------------------------------------------------
{
    if (DrawHandle(layout, Point3(x, y, 0), id))
    {
        Widget *widget = layout->Display();
        Vector3 v = widget->dragDelta();
        if (v.x != 0 || v.y != 0)
        {
            updateArg(widget, &x,  v.x);
            updateArg(widget, &y,  v.y);
            widget->markChanged("Control point moved");
        }
    }
}



// ============================================================================
// 
//   A DrawingManipulator represents an object
// 
// ============================================================================

DrawingManipulator::DrawingManipulator(Drawing *child)
// ----------------------------------------------------------------------------
//   Record the child we own
// ----------------------------------------------------------------------------
    : child(child)
{}


DrawingManipulator::~DrawingManipulator()
// ----------------------------------------------------------------------------
//   Delete the child with the control rectangle
// ----------------------------------------------------------------------------
{
    delete child;
}


void DrawingManipulator::Draw(Layout *layout)
// ----------------------------------------------------------------------------
//   Draw the child and then the manipulator
// ----------------------------------------------------------------------------
{
    Widget *widget = layout->Display();
    bool loadId = widget->currentId() != ~0U;
    if (loadId)
        glLoadName(widget->newId());
    child->Draw(layout);
    Manipulator::Draw(layout);
    if (loadId)
        glLoadName(0);
}


void DrawingManipulator::DrawSelection(Layout *layout)
// ----------------------------------------------------------------------------
//   Draw the selection for the child, and then for this
// ----------------------------------------------------------------------------
{
    Widget *widget = layout->Display();
    bool loadId = widget->currentId() != ~0U;
    if (loadId)
        glLoadName(widget->newId());
    child->DrawSelection(layout);
    Manipulator::DrawSelection(layout);
    if (loadId)
        glLoadName(0);
}


void DrawingManipulator::Identify(Layout *layout)
// ----------------------------------------------------------------------------
//   Identify the child
// ----------------------------------------------------------------------------
{
    Widget *widget = layout->Display();
    bool loadId = widget->currentId() != ~0U;
    if (loadId)
        glLoadName(widget->newId());
    child->Identify(layout);
    Manipulator::Identify(layout);
    if (loadId)
        glLoadName(0);
}


Box3 DrawingManipulator::Bounds()
// ----------------------------------------------------------------------------
//   Return the bounds of the child
// ----------------------------------------------------------------------------
{
    return child->Bounds();
}


Box3 DrawingManipulator::Space()
// ----------------------------------------------------------------------------
//   Return the space of the child
// ----------------------------------------------------------------------------
{
    return child->Space();
}


bool DrawingManipulator::IsWordBreak()
// ----------------------------------------------------------------------------
//   Return the property of the child
// ----------------------------------------------------------------------------
{
    return child->IsWordBreak();
}


bool DrawingManipulator::IsLineBreak()
// ----------------------------------------------------------------------------
//   Return the property of the child
// ----------------------------------------------------------------------------
{
    return child->IsLineBreak();
}


bool DrawingManipulator::IsAttribute()
// ----------------------------------------------------------------------------
//   Return the property of the child
// ----------------------------------------------------------------------------
{
    return child->IsAttribute();
}



// ============================================================================
//
//   A rectangle manipulator udpates x, y, w and h
//
// ============================================================================

ControlRectangle::ControlRectangle(real_r x, real_r y, real_r w, real_r h,
                                   Drawing *child)
// ----------------------------------------------------------------------------
//   A control rectangle owns a given child and manipulates it
// ----------------------------------------------------------------------------
    : DrawingManipulator(child), x(x), y(y), w(w), h(h)
{}


void ControlRectangle::DrawHandles(Layout *layout)
// ----------------------------------------------------------------------------
//   Draw the handles for a rectangular object
// ----------------------------------------------------------------------------
{
    Widget *widget = layout->Display();
    coord   xx = x, yy = y, ww = w, hh = h;
    Vector3 v = widget->dragDelta();
    bool    changed = v.x != 0 || v.y != 0;

    // Lower-left corner
    if (DrawHandle(layout, Point3(xx - ww/2, yy - hh/2, 0), 1))
    {
        if (changed)
        {
            updateArg(widget, &w, -2*updateArg(widget, &x,  v.x/2));
            updateArg(widget, &h, -2*updateArg(widget, &y,  v.y/2));
            widget->markChanged("Lower left corner moved");
            changed = false;
        }
    }

    // Lower-right corner
    if (DrawHandle(layout, Point3(xx + ww/2, yy - hh/2, 0), 2))
    {
        if (changed)
        {
            updateArg(widget, &w,  2*updateArg(widget, &x,  v.x/2));
            updateArg(widget, &h, -2*updateArg(widget, &y,  v.y/2));
            widget->markChanged("Lower right corner moved");
            changed = false;
        }
    }

    // Upper-left corner
    if (DrawHandle(layout, Point3(xx - ww/2, yy + hh/2, 0), 3))
    {
        if (changed)
        {
            updateArg(widget, &w, -2*updateArg(widget, &x,  v.x/2));
            updateArg(widget, &h,  2*updateArg(widget, &y,  v.y/2));
            widget->markChanged("Uppper left corner moved");
            changed = false;
        }
    }

    // Upper-right corner
    if (DrawHandle(layout, Point3(xx + ww/2, yy + hh/2, 0), 4))
    {
        if (changed)
        {
            updateArg(widget, &w,  2*updateArg(widget, &x,  v.x/2));
            updateArg(widget, &h,  2*updateArg(widget, &y,  v.y/2));
            widget->markChanged("Upper right corner moved");
            changed = false;
        }
    }

    // Anywhere else in the shape
    if (changed)
    {
        updateArg(widget, &x, v.x);
        updateArg(widget, &y, v.y);
        widget->markChanged("Shape moved");
    }
}

TAO_END
