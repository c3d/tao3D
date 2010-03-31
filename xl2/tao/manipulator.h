#ifndef MANIPULATOR_H
#define MANIPULATOR_H
// ****************************************************************************
//  manipulator.h                                                   Tao project
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

#include "widget.h"
#include "tree.h"
#include "coords3d.h"
#include "drawing.h"

#include <vector>
#include <map>


TAO_BEGIN

struct Manipulator : Drawing
// ----------------------------------------------------------------------------
//   Structure used to manipulate XL coordinates using the mouse
// ----------------------------------------------------------------------------
{
    typedef XL::Tree     Tree, *tree_p;
    typedef XL::Real&    real_r;
    typedef XL::Integer& integer_r;

    Manipulator();

    virtual void        Draw(Layout *layout);
    virtual void        DrawSelection(Layout *layout);
    virtual void        Identify(Layout *layout);
    virtual bool        DrawHandle(Layout *layout, Point3 p, uint id);
    virtual bool        DrawHandles(Layout *layout) = 0;

protected:
    double              updateArg(Widget *widget, tree_p param, coord delta);
};


struct ControlPoint : Manipulator
// ----------------------------------------------------------------------------
//    A control point in an object like a path
// ----------------------------------------------------------------------------
{
    ControlPoint(real_r x, real_r y, real_r z, uint id);
    virtual bool        DrawHandles(Layout *layout);
    virtual void        Draw(Layout *layout);
    virtual void        DrawSelection(Layout *layout);
    virtual void        Identify(Layout *layout);

protected:
    real_r              x, y, z;
    uint                id;
};


struct DrawingManipulator : Manipulator
// ----------------------------------------------------------------------------
//   Manipulators for objects that have a child
// ----------------------------------------------------------------------------
{
    DrawingManipulator(Drawing *child);
    ~DrawingManipulator();

    virtual void        Draw(Layout *layout);
    virtual void        DrawSelection(Layout *layout);
    virtual void        Identify(Layout *layout);
    virtual bool        DrawHandles(Layout *layout);
    virtual Box3        Bounds();
    virtual Box3        Space();
    virtual bool        IsWordBreak();
    virtual bool        IsLineBreak();
    virtual bool        IsAttribute();

protected:
    Drawing *           child;
};


struct FrameManipulator : DrawingManipulator
// ----------------------------------------------------------------------------
//   Dispays 4 handles in the corner, but clicks in the surface pass through
// ----------------------------------------------------------------------------
{
    FrameManipulator(real_r x, real_r y, real_r w, real_r h, Drawing *child);
    virtual void        DrawSelection(Layout *layout);
    virtual bool        DrawHandles(Layout *layout);

protected:
    real_r              x, y, w, h;
};


struct ControlRectangle : FrameManipulator
// ----------------------------------------------------------------------------
//   Manipulators for a rectangle-bounded object
// ----------------------------------------------------------------------------
{
    ControlRectangle(real_r x, real_r y, real_r w, real_r h,
                     Drawing *child);
    virtual void        DrawSelection(Layout *layout);
    virtual bool        DrawHandles(Layout *layout);
};


struct WidgetSurface;
struct WidgetManipulator : FrameManipulator
// ----------------------------------------------------------------------------
//   Manipulators for widgets
// ----------------------------------------------------------------------------
{
    WidgetManipulator(real_r x, real_r y, real_r w, real_r h, WidgetSurface *s);
    virtual void        DrawSelection(Layout *layout);

protected:
    WidgetSurface *     surface;
};


struct BoxManipulator : DrawingManipulator
// ----------------------------------------------------------------------------
//   Dispays 8 handles in the corner, but clicks in the volume pass through
// ----------------------------------------------------------------------------
{
    BoxManipulator(real_r x, real_r y, real_r z, real_r w, real_r h, real_r d,
                   Drawing *child);
    virtual void        DrawSelection(Layout *layout);
    virtual bool        DrawHandles(Layout *layout);

protected:
    real_r              x, y, z, w, h, d;
};


struct ControlBox : BoxManipulator
// ----------------------------------------------------------------------------
//   Manipulators for a box-bounded object
// ----------------------------------------------------------------------------
{
    ControlBox(real_r x, real_r y, real_r z, real_r w, real_r h, real_r d,
               Drawing *child);
    virtual bool        DrawHandles(Layout *layout);
};

TAO_END

XL_BEGIN

// ============================================================================
// 
//    Specialized variant of 'TreeClone' that normalizes --x into x
// 
// ============================================================================

struct NormalizedCloneMode;
template<>
inline Tree *TreeCloneTemplate<NormalizedCloneMode>::DoPrefix(Prefix *what)
// ----------------------------------------------------------------------------
//   Specialization of TreeClone that changes --x into x
// ----------------------------------------------------------------------------
{
    if (Name *n = what->left->AsName())
    {
        if (n->value == "-")
        {
            if (Real *rv = what->right->AsReal())
                if (rv->value < 0)
                    return new Real(-rv->value, rv->Position());
            if (Integer *iv = what->right->AsInteger())
                if (iv->value < 0)
                    return new Integer(-iv->value, iv->Position());
        }
    }
    return new Prefix(Clone(what->left), Clone(what->right),
                      what->Position());
}


typedef XL::TreeCloneTemplate<NormalizedCloneMode> NormalizedClone;

XL_END

#endif // MANIPULATOR_H
