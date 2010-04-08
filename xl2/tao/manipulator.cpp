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
//  (C) 2010 Lionel Schaffhauser <lionel@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "manipulator.h"
#include "drag.h"
#include "layout.h"
#include "shapes.h"
#include "widget_surface.h"
#include "gl_keepers.h"
#include "runtime.h"
#include "transforms.h"

#include <QtDebug>

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
    if (uint sel = widget->selected())
    {
        if (sel < 0x1000)
        {
            uint count = 1;
            uint idR = 99, idT = 99, idS = 99;
            if (layout->lastRotation)       idR = count++;
            if (layout->lastTranslation)    idT = count++;
            if (layout->lastScale)          idS = count++;

            sel = (sel-1) % count;

            widget->select(layout->lastRotation,    sel == idR ? 0x1000 : 0);
            widget->select(layout->lastTranslation, sel == idT ? 0x2000 : 0);
            widget->select(layout->lastScale,       sel == idS ? 0x4000 : 0);
            widget->select(widget->currentId(),     sel+1);
        }

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


bool Manipulator::DrawHandle(Layout *layout, Point3 p, uint id, text name)
// ----------------------------------------------------------------------------
//   Draw one of the handles for the current manipulator
// ----------------------------------------------------------------------------
{
    Widget *widget = layout->Display();
    Vector3 offset = layout->Offset();
    glLoadName(id);
    widget->drawHandle(p + offset, name);
    glLoadName(0);
    bool selected = widget->manipulator == id;
    return selected;
}


void Manipulator::updateArg(Widget *widget, tree_p arg,
                            double first, double previous, double current,
                            bool has_min, double min, bool has_max, double max)
// ----------------------------------------------------------------------------
//   Update the given argument by the given offset
// ----------------------------------------------------------------------------
{
    // Defensive coding against bad callers...
    if (!arg || previous == current)
        return;

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
        if (XL::Postfix *postfix = (*ptr)->AsPostfix())
        {
            if (XL::Name *name = postfix->right->AsName())
            {
                if (name->value == "%")
                {
                    pptr = ptr;
                    ptr = &postfix->left;
                    more = true;
                    scale /= 100;
                }
            }
        }
    }
    // REVISIT: Really?
    if (scale == 0.0)
        scale = 1.0;

    // Test the simple cases where the argument is directly an Integer or Real
    if (XL::Integer *ival = (*ptr)->AsInteger())
    {
        ival->value -= longlong((previous - first) / scale);
        ival->value += longlong((current - first) / scale);
        if (has_min && ival->value * scale < min)
            ival->value = min / scale;
        if (has_max && ival->value * scale > max)
            ival->value = max / scale;
        if (ppptr && ival->value < 0)
            widget->reloadProgram = true;
    }
    else if (XL::Real *rval = (*ptr)->AsReal())
    {
        rval->value += (current - previous) / scale;
        if (has_min && ival->value * scale < min)
            ival->value = min / scale;
        if (has_max && ival->value * scale > max)
            ival->value = max / scale;
        if (ppptr && rval->value < 0)
            widget->reloadProgram = true;
    }
    else
    {
        // LIONEL: When does that happen?
        // Create an Infix + with the delta we add
        if (ptr != &arg)
        {
            double value = current;
            if (has_min && current < min)
                value = min;
            if (has_max && current > max)
                value = max;
            double delta = (value - previous) / scale;
            *ptr = new XL::Infix("+", new XL::Real(delta), *ptr);
            widget->reloadProgram = true;
        }
    }
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
        Drag *drag = widget->drag();
        if (drag)
        {
            Point3 p1 = drag->Previous();
            Point3 p2 = drag->Current();
            if (p1 != p2)
            {
                Point3 p0 = drag->Origin();
                updateArg(widget, &x,  p0.x, p1.x, p2.x);
                updateArg(widget, &y,  p0.y, p1.y, p2.y);
                updateArg(widget, &z,  p0.z, p1.z, p2.z);
                widget->markChanged("Control point moved");
                return true;
            }
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
    // Manipulator::Draw(layout); is a NOOP
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
    Drag   *drag = widget->drag();
    uint    handle = 0;

    for (uint hn = 0; hn < 4; hn++)
    {
        short  sw = (hn & 1) ? 1 : -1;
        short  sh = (hn & 2) ? 1 : -1;

        // Lower-left corner
        if (DrawHandle(layout, Point3(xx + sw*ww/2, yy + sh*hh/2, 0), hn+1))
        {
            if (!handle)
            {
                handle = hn+1;

                // Update arguments if necessary
                if (drag)
                {
                    Point3 p1 = drag->Previous();
                    Point3 p2 = drag->Current();
                    if (p1 != p2)
                    {
                        Point3 p0 = drag->Origin();
                        text   t1 = sh < 0 ? "Lower " : "Upper ";
                        text   t2 = sw < 0 ? "left " : "right ";

                        updateArg(widget, &x, p0.x/2, p1.x/2, p2.x/2);
                        updateArg(widget, &y, p0.y/2, p1.y/2, p2.y/2);
                        updateArg(widget, &w, sw*p0.x, sw*p1.x, sw*p2.x);
                        updateArg(widget, &h, sh*p0.y, sh*p1.y, sh*p2.y);

                        widget->markChanged(t1 + t2 + " corner moved");
                    }
                }
            }
        }
    }

    return handle != 0;
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
    if (!changed)
    {
        Widget *widget = layout->Display();
        Drag *drag = widget->drag();
        if (drag && !widget->manipulatorId())
        {
            Point3 p1 = drag->Previous();
            Point3 p2 = drag->Current();
            if (p1 != p2)
            {
                Point3 p0 = drag->Origin();
                updateArg(widget, &x, p0.x, p1.x, p2.x);
                updateArg(widget, &y, p0.y, p1.y, p2.y);
                widget->markChanged("Shape moved");
                changed = true;
            }
        }
    }
    return changed;
}



// ============================================================================
//
//   An rounded rectangle manipulator udpates x, y, w, h, the radius of the
//   corners and allows translation
//
// ============================================================================

ControlRoundedRectangle::ControlRoundedRectangle(real_r x, real_r y, 
                                                 real_r w, real_r h, 
                                                 real_r r,
                                                 Drawing *child)
// ----------------------------------------------------------------------------
//   A control arrow adds the radius of the corners to the control rectangle 
// ----------------------------------------------------------------------------
    : ControlRectangle(x, y, w, h, child), r(r)
{}


bool ControlRoundedRectangle::DrawHandles(Layout *layout)
// ----------------------------------------------------------------------------
//   Draw the handles for the rounded rectangle (assuming rx = ry)
// ----------------------------------------------------------------------------
{
    bool changed = false;
    Widget *widget = layout->Display();
    Drag   *drag = widget->drag();

    coord rr = (r < 0? 0: 1)*r;
    int sw = w > 0? 1: -1;
    int sh = h > 0? 1: -1;

    if (sh*h < sw*w)
    {
        if (r > sw*w/2)
            rr = sw*w/2;

        if (DrawHandle(layout, Point3(x - sw*w/2 + rr, y + sh*h/2, 0), 9))
        {
            if (drag)
            {
                Point3 p1 = drag->Previous();
                Point3 p2 = drag->Current();
                if (p1 != p2)
                {
                    Point3 p0 = drag->Origin();
                    updateArg(widget, &r, 
                              p0.x-x+sw*w/2, p1.x-x+sw*w/2, p2.x-x+sw*w/2,
                              true, 0.0, true, sw*w/2);
                    widget->markChanged("Rounded rectangle corner modified");
                    changed = true;
                }
            }
        }
    } 
    else
    {
        if (r > sh*h/2)
            rr = sh*h/2;

        if (DrawHandle(layout, Point3(x - sw*w/2,y + sh*h/2 - rr, 0), 9))
        {
            if (drag)
            {
                Point3 p1 = drag->Previous();
                Point3 p2 = drag->Current();
                if (p1 != p2)
                {
                    Point3 p0 = drag->Origin();
                    updateArg(widget, &r, 
                              y+sh*h/2-p0.y, y+sh*h/2-p1.y, y+sh*h/2-p2.y,
                              true, 0.0, true, sh*h/2);
                    widget->markChanged("Rounded rectangle corner modified");
                    changed = true;
                }
            }
        }
     }
    if (!changed)
    {
        changed = ControlRectangle::DrawHandles(layout);
    }
    return changed;
}



// ============================================================================
//
//   An arrow manipulator udpates x, y, w, h, the arrow handle a and allows 
//   translation
//
// ============================================================================

ControlArrow::ControlArrow(real_r x, real_r y, real_r w, real_r h, 
                           real_r ax, real_r ary, bool is_double,
                           Drawing *child)
// ----------------------------------------------------------------------------
//   A control arrow adds the arrow hanfle to the control rectangle 
// ----------------------------------------------------------------------------
    : ControlRectangle(x, y, w, h, child), ax(ax), ary(ary), d(is_double)
{}

ControlArrow::ControlArrow(real_r x, real_r y, real_r w, real_r h, 
                           real_r ax, real_r ary,
                           Drawing *child)
    : ControlRectangle(x, y, w, h, child), ax(ax), ary(ary), d(false)
{}


bool ControlArrow::DrawHandles(Layout *layout)
// ----------------------------------------------------------------------------
//   Draw the handles for an arrow
// ----------------------------------------------------------------------------
{
    bool changed = false;
    coord aax, aay;
    int sw = w > 0? 1: -1;
    int sdw = d? sw: 1;
    int swd = d? 1: sw;
    int sh = h > 0? 1: -1;
    int df = d? 2: 1;

    if (ax > sw*w/df) 
        aax = w/df;
    else
        aax = sw*ax;
    
    if (ax < 0.0) 
        aax = 0.0;

    if (ary > 1.0) 
        aay = h;
    else
        aay = ary*h;
    
    if (ary < 0.0) 
        aay = 0.0;
 
    if (DrawHandle(layout, 
                   Point3(x+sdw*(w/2-aax), y+sh*aay/2, 0), 9))
    {
        Widget *widget = layout->Display();
        Drag *drag = widget->drag();
        if (drag)
        {
            Point3 p1 = drag->Previous();
            Point3 p2 = drag->Current();
            if (p1 != p2)
            {
                Point3 p0 = drag->Origin();
                updateArg(widget, &ax, 
                          swd*(x-p0.x)+sw*w/2, swd*(x-p1.x)+sw*w/2, swd*(x-p2.x)+sw*w/2,
                          true, 0.0, true, sw*w/df);
                if (h != 0)
                {
                    updateArg(widget, &ary, 
                              2*sh*(p0.y-y)/h, 2*sh*(p1.y-y)/h, 2*sh*(p2.y-y)/h,
                              true, 0.0, true, 1.0);
                }
                widget->markChanged("Arrow modified");
                changed = true;
            }
        }
    }
    if (!changed)
    {
        changed = ControlRectangle::DrawHandles(layout);
    }
    return changed;
}



// ============================================================================
//
//   A polygon manipulator udpates x, y, w, h, he number of points for the 
//   polygon and allows translation
//
// ============================================================================

ControlPolygon::ControlPolygon(real_r x, real_r y, real_r w, real_r h, 
                               integer_r p,
                               Drawing *child)
// ----------------------------------------------------------------------------
//   A control star adds the number of points to the control rectangle 
// ----------------------------------------------------------------------------
    : ControlRectangle(x, y, w, h, child), p(p)
{}


bool ControlPolygon::DrawHandles(Layout *layout)
// ----------------------------------------------------------------------------
//   Draw the handles for a polygon
// ----------------------------------------------------------------------------
{
    bool changed = false;
    if (!changed && DrawHandle(layout, Point3(x -w/2 + (p-2)*w/19, y - h/2, 0), 9))
    {
        Widget *widget = layout->Display();
        Drag *drag = widget->drag();
        if (drag)
        {
            Point3 p1 = drag->Previous();
            Point3 p2 = drag->Current();
            if (p1 != p2)
            {
                Point3 p0 = drag->Origin();
                coord p0x = 19*(p0.x - x)/w + 11.5;
                coord p1x = 19*(p1.x - x)/w + 11.5;
                coord p2x = 19*(p2.x - x)/w + 11.5;
                updateArg(widget, &p, p0x, p1x, p2x, true, 3, true, 20);
                widget->markChanged("Number of points changed");
                changed = true;
            }
        }
    }
    if (!changed)
    {
        changed = ControlRectangle::DrawHandles(layout);
    }
    return changed;
}



// ============================================================================
//
//   A star manipulator udpates x, y, w, h, the number of points, the inner
//   circle ration and allows translation
//
// ============================================================================

ControlStar::ControlStar(real_r x, real_r y, real_r w, real_r h, 
                         integer_r p, real_r r,
                         Drawing *child)
// ----------------------------------------------------------------------------
//   A control star adds inner circle ratio to the control polygon 
// ----------------------------------------------------------------------------
    : ControlPolygon(x, y, w, h, p, child), r(r)
{}


bool ControlStar::DrawHandles(Layout *layout)
// ----------------------------------------------------------------------------
//   Draw the handles for a star
// ----------------------------------------------------------------------------
{
    double cp = cos(M_PI/p);
    double sp = sin(M_PI/p);
    bool changed = false;
    if (DrawHandle(layout, Point3(x + r*w/2*sp, y + r*h/2*cp, 0), 11))
    {
        Widget *widget = layout->Display();
        Drag *drag = widget->drag();
        if (drag)
        {
            Point3 p1 = drag->Previous();
            Point3 p2 = drag->Current();
            if (p1 != p2)
            {
                Point3 p0 = drag->Origin();
                scale hp = sqrt(w*sp*w*sp + h*cp*h*cp) * cp/2;
                updateArg(widget, &r, 
                          (p0.y - y)/hp, (p1.y - y)/hp, (p2.y - y)/hp,
                          true, 0.0, true, 1.0);
                widget->markChanged("Star inner circle changed");
                changed = true;
            }
        }
    }
    if (!changed)
    {
        changed = ControlPolygon::DrawHandles(layout);
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
    Drag   *drag = widget->drag();
    uint    handle = 0;

    for (uint hn = 0; hn < 8; hn++)
    {
        short  sw = (hn & 1) ? 1 : -1;
        short  sh = (hn & 2) ? 1 : -1;
        short  sd = (hn & 4) ? 1 : -1;

        // Lower-left corner
        if (DrawHandle(layout, Point3(xx+sw*ww/2,yy+sh*hh/2,zz+sd*dd/2), hn+1))
        {
            if (!handle)
            {
                handle = hn+1;

                // Update arguments if necessary
                if (drag)
                {
                    Point3 p1 = drag->Previous();
                    Point3 p2 = drag->Current();
                    if (p1 != p2)
                    {
                        Point3 p0 = drag->Origin();
                        text   t1 = sh < 0 ? "Lower " : "Upper ";
                        text   t2 = sw < 0 ? "left " : "right ";
                        text   t3 = sd < 0 ? "front " : "back ";

                        if (hn < 4)
                        {
                            updateArg(widget, &x, p0.x/2, p1.x/2, p2.x/2);
                            updateArg(widget, &w, sw*p0.x, sw*p1.x, sw*p2.x);
                        }
                        else
                        {
                            updateArg(widget, &z, p0.x/2, p1.x/2, p2.x/2);
                            updateArg(widget, &d, sd*p0.x, sd*p1.x, sd*p2.x);
                        }
                        updateArg(widget, &y, p0.y/2, p1.y/2, p2.y/2);
                        updateArg(widget, &h, sh*p0.y, sh*p1.y, sh*p2.y);

                        widget->markChanged(t1 + t2 + t3 + " corner moved");
                    }
                }
            }
        }
    }

    return handle != 0;
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
    if (!changed)
    {
        Widget *widget = layout->Display();
        Drag *drag = widget->drag();
        if (drag && !widget->manipulatorId())
        {
            Point3 p1 = drag->Previous();
            Point3 p2 = drag->Current();
            if (p1 != p2)
            {
                Point3 p0 = drag->Origin();
                updateArg(widget, &x, p0.x, p1.x, p2.x);
                updateArg(widget, &y, p0.y, p1.y, p2.y);
                updateArg(widget, &z, p0.z, p1.z, p2.z);
                widget->markChanged("3D shape moved");
                changed = true;
            }
        }
    }
    return changed;
}



// ============================================================================
// 
//   A TransformManipulator is used for rotation, translation, scale
// 
// ============================================================================

TransformManipulator::TransformManipulator(Drawing *child)
// ----------------------------------------------------------------------------
//   Record the child we own
// ----------------------------------------------------------------------------
    : DrawingManipulator(child)
{}



// ============================================================================
// 
//   Rotation manipulator
// 
// ============================================================================

RotationManipulator::RotationManipulator(real_r a, real_r x,real_r y,real_r z)
// ----------------------------------------------------------------------------
//   Manipulation of a rotation
// ----------------------------------------------------------------------------
    : TransformManipulator(new Rotation(a, x, y, z)), a(a), x(x), y(y), z(z)
{}


void RotationManipulator::Identify(Layout *layout)
// ----------------------------------------------------------------------------
//   Remember the last rotation for subsequent shapes
// ----------------------------------------------------------------------------
{
    TransformManipulator::Identify(layout);
    Widget *widget = layout->Display();
    uint id = widget->currentId();
    layout->lastRotation = id;
}


bool RotationManipulator::DrawHandles(Layout *layout)
// ----------------------------------------------------------------------------
//   Draw the selection for a rotation
// ----------------------------------------------------------------------------
{
    Widget  *widget  = layout->Display();
    Vector3  offset  = layout->Offset();
    Drag    *drag    = widget->drag();
    bool     changed = false;

    // Vector around which we rotate (normalized)
    Vector3 tip = Vector3(x, y, z);
    tip.Normalize();
    if (DrawHandle(layout, 50*tip + offset, 0x1001, "rotation_tip"))
    {
        if (drag)
        {
            // Compute the desired tip position
            Point3 p1 = drag->Previous();
            Point3 p2 = drag->Current();
            if (p1 != p2)
            {
                Point3 p0 = drag->Origin();

                updateArg(widget, &x, p0.x, p1.x, p2.x);
                updateArg(widget, &y, p0.y, p1.y, p2.y);
                updateArg(widget, &z, p0.z, p1.z, p2.z);                
                widget->markChanged("Changed rotation direction");
                changed = true;
            }
        }
    }

    // Draw the sphere
    if (DrawHandle(layout, offset, 0x1002, "rotation_base"))
    {
        if (drag && !changed)
        {
            // Compute the desired angle
            Point3 p1 = drag->Previous();
            Point3 p2 = drag->Current();
            if (p1 != p2)
            {
                Point3 p0 = drag->Origin();

                Vector3 v0 = Vector3(p0) - offset;
                double  a0 = atan2(v0.y, v0.x) * (180 / M_PI);
                Vector3 v1 = Vector3(p1) - offset;
                double  a1 = atan2(v1.y, v1.x) * (180 / M_PI);
                Vector3 v2 = Vector3(p2) - offset;
                double  a2 = atan2(v2.y, v2.x) * (180 / M_PI);

                updateArg(widget, &a,  a0, a1, a2);
                updateArg(widget, &a,  a0, a1, a2);
                
                widget->markChanged("Changed rotation amount");
                changed = true;
            }
        }
    }

    return changed;
}



// ============================================================================
// 
//   Translation manipulator
// 
// ============================================================================

TranslationManipulator::TranslationManipulator(real_r x, real_r y, real_r z)
// ----------------------------------------------------------------------------
//   Manipulation of a translation
// ----------------------------------------------------------------------------
    : TransformManipulator(new Translation(x, y, z)), x(x), y(y), z(z)
{}


void TranslationManipulator::Identify(Layout *layout)
// ----------------------------------------------------------------------------
//   Remember the last translation for subsequent shapes
// ----------------------------------------------------------------------------
{
    TransformManipulator::Identify(layout);
    Widget *widget = layout->Display();
    uint id = widget->currentId();
    layout->lastTranslation = id;
}


bool TranslationManipulator::DrawHandles(Layout *layout)
// ----------------------------------------------------------------------------
//   Manipulate the translation
// ----------------------------------------------------------------------------
{
    Widget  *widget = layout->Display();
    Vector3  offset = layout->Offset();
    Drag    *drag   = widget->drag();
    uint     handle = 0;

    // Draw the translation base
    if (DrawHandle(layout, offset, 0x2001, "translation_base"))
        handle = 1;

    // Size of the X, Y, Z direction
    if (DrawHandle(layout, offset, 0x2002, "translation_x"))
        handle = 2;

    if (DrawHandle(layout, offset, 0x2003, "translation_y"))
        handle = 3;

    if (DrawHandle(layout, offset, 0x2004, "translation_z"))
        handle = 4;

    if (handle && drag)
    {
        Point3 p1 = drag->Previous();
        Point3 p2 = drag->Current();
        if (p1 != p2)
        {
            Point3 p0 = drag->Origin();

            switch(handle)
            {
            case 1:
                updateArg(widget, &x,  p0.x, p1.x, p2.x);
                updateArg(widget, &y,  p0.y, p1.y, p2.y);
                widget->markChanged("Updated translation");
                break;
            case 2:
                updateArg(widget, &x,  p0.x, p1.x, p2.x);
                widget->markChanged("Updated translation X axis");
                break;
            case 3:
                updateArg(widget, &y,  p0.y, p1.y, p2.y);
                widget->markChanged("Updated translation Y axis");
                break;
            case 4:
                updateArg(widget, &z,  p0.x - p0.y, p1.x - p1.y, p2.x - p2.y);
                widget->markChanged("Updated translation Z axis");
                break;
            }

            return true;
        }
    }

    return false;
};




// ============================================================================
// 
//   Scale manipulator
// 
// ============================================================================

ScaleManipulator::ScaleManipulator(real_r x, real_r y, real_r z)
// ----------------------------------------------------------------------------
//   Manipulation of a scale
// ----------------------------------------------------------------------------
    : TransformManipulator(new Scale(x, y, z)), x(x), y(y), z(z)
{}


void ScaleManipulator::Identify(Layout *layout)
// ----------------------------------------------------------------------------
//   Remember the last scaling for subsequent shapes
// ----------------------------------------------------------------------------
{
    TransformManipulator::Identify(layout);
    Widget *widget = layout->Display();
    uint id = widget->currentId();
    layout->lastScale = id;
}


bool ScaleManipulator::DrawHandles(Layout *layout)
// ----------------------------------------------------------------------------
//   Manipulate the scale
// ----------------------------------------------------------------------------
{
    Widget  *widget = layout->Display();
    Vector3  offset = layout->Offset();
    Drag    *drag   = widget->drag();
    uint     handle = 0;
    coord    s = 0.01;

    // Draw the scale base
    if (DrawHandle(layout, offset, 0x4001, "scale_base"))
        handle = 1;

    // Size of the X, Y, Z direction
    if (DrawHandle(layout, offset, 0x4002, "scale_x"))
        handle = 2;

    if (DrawHandle(layout, offset, 0x4003, "scale_y"))
        handle = 3;

    if (DrawHandle(layout, offset, 0x4004, "scale_z"))
        handle = 4;

    if (handle && drag)
    {
        Point3 p1 = drag->Previous();
        Point3 p2 = drag->Current();
        if (p1 != p2)
        {
            Point3 p0 = drag->Origin();
            coord  v0 = s * (p0.x + p0.y + p0.z);
            coord  v1 = s * (p1.x + p1.y + p1.z);
            coord  v2 = s * (p2.x + p2.y + p2.z);

            switch(handle)
            {
            case 1:
                updateArg(widget, &x, v0, v1, v2);
                updateArg(widget, &y, v0, v1, v2);
                updateArg(widget, &z, v0, v1, v2);
                widget->markChanged("Updated scale");
                break;
            case 2:
                updateArg(widget, &x,  s * p0.x, s * p1.x, s * p2.x);
                widget->markChanged("Updated scale X axis");
                break;
            case 3:
                updateArg(widget, &y,  s * p0.y, s * p1.y, s * p2.y);
                widget->markChanged("Updated scale Y axis");
                break;
            case 4:
                updateArg(widget, &z,  v0, v1, v2);
                widget->markChanged("Updated scale Z axis");
                break;
            }

            return true;
        }
    }

    return false;
};

TAO_END
