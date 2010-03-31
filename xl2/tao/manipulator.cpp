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
#include "shapes.h"
#include "widget_surface.h"
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
//   When drawing a manipulator, we don't draw anything
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
    Vector3 offset = layout->Offset();
    glLoadName(id);
    widget->drawHandle(p + offset, "handle");
    glLoadName(0);
    bool selected = widget->manipulator == id;
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
                    more = true;
                }
                else if (XL::Real *rr = infix->right->AsReal())
                {
                    scale *= rr->value;
                    ptr = &infix->left;
                    more = true;
                }
                else if (XL::Integer *li = infix->left->AsInteger())
                {
                    scale *= li->value;
                    ptr = &infix->right;
                    more = true;
                }
                else if (XL::Integer *ri = infix->right->AsInteger())
                {
                    scale *= ri->value;
                    ptr = &infix->left;
                    more = true;
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
                    scale = -scale;
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

    return delta * scale;
}



// ============================================================================
// 
//    A control point updates specific coordinates
// 
// ============================================================================

ControlPoint::ControlPoint(real_r x, real_r y, real_r z, uint id)
// ----------------------------------------------------------------------------
//   Record where we want to draw
// ----------------------------------------------------------------------------
    : Manipulator(), x(x), y(y), z(z), id(id)
{}


void ControlPoint::Draw(Layout *where)
// ----------------------------------------------------------------------------
//    Draw a control point
// ----------------------------------------------------------------------------
{
    Manipulator::Draw(where);
}


void ControlPoint::DrawSelection(Layout *layout)
// ----------------------------------------------------------------------------
//    Draw the selection for a control point
// ----------------------------------------------------------------------------
{
    // We don't need to glPushName, as the parent should have done it for us
    Widget *widget = layout->Display();
    if (widget->selected())
        DrawHandles(layout);
}


void ControlPoint::Identify(Layout *where)
// ----------------------------------------------------------------------------
//    Select the identity for a control point
// ----------------------------------------------------------------------------
{
    ControlPoint::DrawSelection(where);
}


bool ControlPoint::DrawHandles(Layout *layout)
// ----------------------------------------------------------------------------
//   For a control point, there is a single handle
// ----------------------------------------------------------------------------
{
    if (DrawHandle(layout, Point3(x, y, z), id))
    {
        Widget *widget = layout->Display();
        Vector3 v = widget->dragDelta();
        if (v.x != 0 || v.y != 0)
        {
            updateArg(widget, &x,  v.x);
            updateArg(widget, &y,  v.y);
            updateArg(widget, &z,  v.z);
            widget->markChanged("Control point moved");
            return true;
        }
    }
    return false;
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


bool DrawingManipulator::DrawHandles(Layout *layout)
// ----------------------------------------------------------------------------
//   Default is to draw no handle...
// ----------------------------------------------------------------------------
{
    (void) layout;
    return false;
}




// ============================================================================
//
//   A frame manipulator allows click-through for rectangular shapes
//
// ============================================================================

FrameManipulator::FrameManipulator(real_r x, real_r y, real_r w, real_r h,
                             Drawing *child)
// ----------------------------------------------------------------------------
//   A control rectangle owns a given child and manipulates it
// ----------------------------------------------------------------------------
    : DrawingManipulator(child), x(x), y(y), w(w), h(h)
{}


void FrameManipulator::DrawSelection(Layout *layout)
// ----------------------------------------------------------------------------
//   Avoid drawing the selection for the child
// ----------------------------------------------------------------------------
{
    Widget *widget = layout->Display();
    bool loadId = widget->currentId() != ~0U;
    if (loadId)
        glLoadName(widget->newId());
    child->Identify(layout);    // Don't draw it
    Manipulator::DrawSelection(layout);
    if (loadId)
        glLoadName(0);
}


bool FrameManipulator::DrawHandles(Layout *layout)
// ----------------------------------------------------------------------------
//   Draw the handles around a widget
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

    return changed;
}



// ============================================================================
//
//   A rectangle manipulator udpates x, y, w and h and allows translation
//
// ============================================================================

ControlRectangle::ControlRectangle(real_r x, real_r y, real_r w, real_r h,
                                   Drawing *child)
// ----------------------------------------------------------------------------
//   A control rectangle owns a given child and manipulates it
// ----------------------------------------------------------------------------
    : FrameManipulator(x, y, w, h, child)
{}


void ControlRectangle::DrawSelection(Layout *layout)
// ----------------------------------------------------------------------------
//   Avoid drawing the selection for the child
// ----------------------------------------------------------------------------
{
    Widget *widget = layout->Display();
    bool loadId = widget->currentId() != ~0U;
    if (loadId)
        glLoadName(widget->newId());
    child->DrawSelection(layout);    // Don't draw it
    Manipulator::DrawSelection(layout);
    if (loadId)
        glLoadName(0);
}


bool ControlRectangle::DrawHandles(Layout *layout)
// ----------------------------------------------------------------------------
//   Draw the handles for a rectangular object
// ----------------------------------------------------------------------------
{
    // Check if we clicked anywhere else in the shape
    bool changed = FrameManipulator::DrawHandles(layout);
    if (changed)
    {
        Widget *widget = layout->Display();
        Vector3 v = widget->dragDelta();
        updateArg(widget, &x, v.x);
        updateArg(widget, &y, v.y);
        widget->markChanged("Shape moved");
        changed = false;
    }
    return changed;
}



// ============================================================================
// 
//   Manipulate a widget
// 
// ============================================================================

WidgetManipulator::WidgetManipulator(real_r x, real_r y, real_r w, real_r h,
                                     WidgetSurface *s)
// ----------------------------------------------------------------------------
//    Create a widget manipulator within the given rectangle
// ----------------------------------------------------------------------------
    : FrameManipulator(x, y, w, h, new Rectangle(Box(x-w/2, y-h/2, w, h))),
      surface(s)
{}


void WidgetManipulator::DrawSelection(Layout *layout)
// ----------------------------------------------------------------------------
//   Draw the selection as usual, and if selected, request focus
// ----------------------------------------------------------------------------
{
    Widget *widget = layout->Display();
    bool loadId = widget->currentId() != ~0U;
    if (loadId)
        glLoadName(widget->newId());
    bool selected = widget->selected();
    child->Identify(layout);
    Manipulator::DrawSelection(layout);
    if (selected)
        surface->requestFocus(x, y);
    if (loadId)
        glLoadName(0);
    if (selected)
        widget->drawSelection(Bounds() + layout->Offset(), "widget_selection");
}



// ============================================================================
//
//   A box manipulator displays the 8 external control points
//
// ============================================================================

BoxManipulator::BoxManipulator(real_r x, real_r y, real_r z,
                               real_r w, real_r h, real_r d,
                               Drawing *child)
// ----------------------------------------------------------------------------
//   A control rectangle owns a given child and manipulates it
// ----------------------------------------------------------------------------
    : DrawingManipulator(child), x(x), y(y), z(z), w(w), h(h), d(d)
{}


void BoxManipulator::DrawSelection(Layout *layout)
// ----------------------------------------------------------------------------
//   Avoid drawing the selection for the child
// ----------------------------------------------------------------------------
{
    Widget *widget = layout->Display();
    bool loadId = widget->currentId() != ~0U;
    if (loadId)
        glLoadName(widget->newId());
    child->DrawSelection(layout);    // Don't draw the child, only identify it
    Manipulator::DrawSelection(layout);
    if (loadId)
        glLoadName(0);
}


bool BoxManipulator::DrawHandles(Layout *layout)
// ----------------------------------------------------------------------------
//   Draw the handles around a widget
// ----------------------------------------------------------------------------
{
    Widget *widget = layout->Display();
    coord   xx = x, yy = y, zz = z, ww = w, hh = h, dd = d;
    Vector3 v = widget->dragDelta();
    bool    changed = v.x != 0 || v.y != 0 || v.z != 0;

    // Lower-left front corner
    if (DrawHandle(layout, Point3(xx - ww/2, yy - hh/2, zz - dd/2), 1))
    {
        if (changed)
        {
            updateArg(widget, &w, -2*updateArg(widget, &x,  v.x/2));
            updateArg(widget, &h, -2*updateArg(widget, &y,  v.y/2));
            updateArg(widget, &d, -2*updateArg(widget, &z,  v.z/2));
            widget->markChanged("Lower left front corner moved");
            changed = false;
        }
    }

    // Lower-right front corner
    if (DrawHandle(layout, Point3(xx + ww/2, yy - hh/2, zz - dd/2), 2))
    {
        if (changed)
        {
            updateArg(widget, &w,  2*updateArg(widget, &x,  v.x/2));
            updateArg(widget, &h, -2*updateArg(widget, &y,  v.y/2));
            updateArg(widget, &d, -2*updateArg(widget, &z,  v.z/2));
            widget->markChanged("Lower right front corner moved");
            changed = false;
        }
    }

    // Upper-left front corner
    if (DrawHandle(layout, Point3(xx - ww/2, yy + hh/2, zz - dd/2), 3))
    {
        if (changed)
        {
            updateArg(widget, &w, -2*updateArg(widget, &x,  v.x/2));
            updateArg(widget, &h,  2*updateArg(widget, &y,  v.y/2));
            updateArg(widget, &d, -2*updateArg(widget, &z,  v.z/2));
            widget->markChanged("Uppper left front corner moved");
            changed = false;
        }
    }

    // Upper-right front corner
    if (DrawHandle(layout, Point3(xx + ww/2, yy + hh/2, zz - dd/2), 4))
    {
        if (changed)
        {
            updateArg(widget, &w,  2*updateArg(widget, &x,  v.x/2));
            updateArg(widget, &h,  2*updateArg(widget, &y,  v.y/2));
            updateArg(widget, &d, -2*updateArg(widget, &z,  v.z/2));
            widget->markChanged("Upper right front corner moved");
            changed = false;
        }
    }

    // Lower-left back corner
    if (DrawHandle(layout, Point3(xx - ww/2, yy - hh/2, zz + dd/2), 5))
    {
        if (changed)
        {
            updateArg(widget, &w, -2*updateArg(widget, &x,  v.x/2));
            updateArg(widget, &h, -2*updateArg(widget, &y,  v.y/2));
            updateArg(widget, &d,  2*updateArg(widget, &z,  v.z/2));
            widget->markChanged("Lower left back corner moved");
            changed = false;
        }
    }

    // Lower-right back corner
    if (DrawHandle(layout, Point3(xx + ww/2, yy - hh/2, zz + dd/2), 6))
    {
        if (changed)
        {
            updateArg(widget, &w,  2*updateArg(widget, &x,  v.x/2));
            updateArg(widget, &h, -2*updateArg(widget, &y,  v.y/2));
            updateArg(widget, &d,  2*updateArg(widget, &z,  v.z/2));
            widget->markChanged("Lower right back corner moved");
            changed = false;
        }
    }

    // Upper-left back corner
    if (DrawHandle(layout, Point3(xx - ww/2, yy + hh/2, zz + dd/2), 7))
    {
        if (changed)
        {
            updateArg(widget, &w, -2*updateArg(widget, &x,  v.x/2));
            updateArg(widget, &h,  2*updateArg(widget, &y,  v.y/2));
            updateArg(widget, &d,  2*updateArg(widget, &z,  v.z/2));
            widget->markChanged("Uppper left back corner moved");
            changed = false;
        }
    }

    // Upper-right back corner
    if (DrawHandle(layout, Point3(xx + ww/2, yy + hh/2, zz + dd/2), 8))
    {
        if (changed)
        {
            updateArg(widget, &w,  2*updateArg(widget, &x,  v.x/2));
            updateArg(widget, &h,  2*updateArg(widget, &y,  v.y/2));
            updateArg(widget, &d,  2*updateArg(widget, &z,  v.z/2));
            widget->markChanged("Upper right back corner moved");
            changed = false;
        }
    }

    return changed;
}



// ============================================================================
//
//   A control box manipulator udpates 3D position and size
//
// ============================================================================

ControlBox::ControlBox(real_r x, real_r y, real_r z,
                       real_r w, real_r h, real_r d,
                       Drawing *child)
// ----------------------------------------------------------------------------
//   A control rectangle owns a given child and manipulates it
// ----------------------------------------------------------------------------
    : BoxManipulator(x, y, z, w, h, d, child)
{}


bool ControlBox::DrawHandles(Layout *layout)
// ----------------------------------------------------------------------------
//   Draw the handles for a 3D object
// ----------------------------------------------------------------------------
{
    // Check if we clicked anywhere else in the shape
    bool changed = BoxManipulator::DrawHandles(layout);
    if (changed)
    {
        Widget *widget = layout->Display();
        Vector3 v = widget->dragDelta();
        updateArg(widget, &x, v.x);
        updateArg(widget, &y, v.y);
        updateArg(widget, &z, v.z);
        widget->markChanged("3D shape moved");
        changed = false;
    }
    return changed;
}

TAO_END
