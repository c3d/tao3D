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
//  (C) 2010 Lionel Schaffhauser <lionel@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "widget.h"
#include "tree.h"
#include "coords3d.h"
#include "drawing.h"

#include <vector>
#include <map>


TAO_BEGIN

struct RealRef : XL::TreeRoot
// ----------------------------------------------------------------------------
//   Looks and behaves like a real_r, but keeps the value around
// ----------------------------------------------------------------------------
{
    RealRef(XL::Real &r): XL::TreeRoot(&r) {}
    XL::Real *operator&()       { return (XL::Real *) tree; }
    operator double()           { return ((XL::Real *) tree)->value; }
};


struct IntegerRef : XL::TreeRoot
// ----------------------------------------------------------------------------
//   Looks and behaves like a real_r, but keeps the value around
// ----------------------------------------------------------------------------
{
    IntegerRef(XL::Integer &r): XL::TreeRoot(&r) {}
    XL::Integer *operator&()    { return (XL::Integer *) tree; }
    operator longlong()         { return ((XL::Integer *) tree)->value; }
};


struct Manipulator : Drawing
// ----------------------------------------------------------------------------
//   Structure used to manipulate XL coordinates using the mouse and keyboard
// ----------------------------------------------------------------------------
{
    typedef XL::Tree     Tree, *tree_p;
    typedef RealRef      real_r;
    typedef IntegerRef   integer_r;

    Manipulator();

    virtual void        Draw(Layout *layout);
    virtual void        DrawSelection(Layout *layout);
    virtual void        Identify(Layout *layout);
    virtual bool        DrawHandle(Layout *layout, Point3 p, uint id,
                                   text name = "handle");
    virtual bool        DrawHandles(Layout *layout) = 0;

protected:
    void                updateArg(Widget *widget, tree_p arg,
                                  double first, double previous, double current,
                                  bool has_min = false, double min = 0.0, 
                                  bool has_max = false, double max = 0.0);
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
    virtual Drawing *   Break(BreakOrder &order);
    virtual bool        IsAttribute();

protected:
    Drawing *           child;
};


struct FrameManipulator : DrawingManipulator
// ----------------------------------------------------------------------------
//   Dispays 4 handles in the corner, but clicks in the surface pass through
// ----------------------------------------------------------------------------
{
    enum TransformMode
    {
        TM_FreeResize,                     // Free resizing
        TM_ResizeLockCenter,               // Resize object wrt. its center
        TM_ResizeLockAspectRatio,          // Keep width/height aspect ratio
        TM_ResizeLockCenterAndAspectRatio,
    };

    FrameManipulator(real_r x, real_r y, real_r w, real_r h, Drawing *child);
    virtual void        DrawSelection(Layout *layout);
    virtual bool        DrawHandles(Layout *layout);
    virtual TransformMode CurrentTransformMode();

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


struct ControlRoundedRectangle : ControlRectangle
// ----------------------------------------------------------------------------
//   Manipulators for a rectangle-bounded object
// ----------------------------------------------------------------------------
{
    ControlRoundedRectangle(real_r x, real_r y, real_r w, real_r h, real_r r,
                            Drawing *child);
    virtual bool        DrawHandles(Layout *layout);

    protected:
    real_r              r;
};


struct ControlArrow : ControlRectangle
// ----------------------------------------------------------------------------
//   Manipulators for a arrow object
// ----------------------------------------------------------------------------
{
    ControlArrow(real_r x, real_r y, real_r w, real_r h, real_r ax, real_r ary,
                 Drawing *child);
    ControlArrow(real_r x, real_r y, real_r w, real_r h, real_r ax, real_r ary,
                 bool is_double, Drawing *child);
    virtual bool        DrawHandles(Layout *layout);

protected:
    real_r              ax, ary;
    bool                d;
};


struct ControlPolygon : ControlRectangle
// ----------------------------------------------------------------------------
//   Manipulators for a polygon object
// ----------------------------------------------------------------------------
{
    ControlPolygon(real_r x, real_r y, real_r w, real_r h, integer_r p,
                   Drawing *child);
    virtual bool        DrawHandles(Layout *layout);

protected:
    integer_r           p;
};


struct ControlStar : ControlPolygon
// ----------------------------------------------------------------------------
//   Manipulators for a star object
// ----------------------------------------------------------------------------
{
    ControlStar(real_r x, real_r y, real_r w, real_r h, integer_r p, real_r r,
                Drawing *child);
    virtual bool        DrawHandles(Layout *layout);

protected:
    real_r              r;
};


struct ControlBalloon : ControlRoundedRectangle
// ----------------------------------------------------------------------------
//   Manipulators for a Balloon object
// ----------------------------------------------------------------------------
{
    ControlBalloon(real_r x, real_r y, real_r w, real_r h, real_r r,
                   real_r ax, real_r ay, Drawing *child);
    virtual bool        DrawHandles(Layout *layout);

    protected:
    real_r              ax, ay;
};


struct ControlCallout : ControlBalloon
// ----------------------------------------------------------------------------
//   Manipulators for a Callout object
// ----------------------------------------------------------------------------
{
    ControlCallout(real_r x, real_r y, real_r w, real_r h, real_r r,
                   real_r ax, real_r ay, real_r d, Drawing *child);
    virtual bool        DrawHandles(Layout *layout);

    protected:
    real_r              d;
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


struct TransformManipulator : DrawingManipulator
// ----------------------------------------------------------------------------
//   Manipulators for transform objects
// ----------------------------------------------------------------------------
{
    TransformManipulator(Drawing *child);
};


struct RotationManipulator : TransformManipulator
// ----------------------------------------------------------------------------
//   Manipulation of a rotation axis and amount
// ----------------------------------------------------------------------------
{
    RotationManipulator(real_r a, real_r x, real_r y, real_r z);
    virtual void        Identify(Layout *layout);
    virtual bool        DrawHandles(Layout *layout);

protected:
    real_r              a, x, y, z;
};


struct TranslationManipulator : TransformManipulator
// ----------------------------------------------------------------------------
//   Manipulation of translation along 3 axes
// ----------------------------------------------------------------------------
{
    TranslationManipulator(real_r x, real_r y, real_r z);
    virtual void        Identify(Layout *layout);
    virtual bool        DrawHandles(Layout *layout);

protected:
    real_r              x, y, z;
};


struct ScaleManipulator : TransformManipulator
// ----------------------------------------------------------------------------
//   Manipulation of scale along 3 axes
// ----------------------------------------------------------------------------
{
    ScaleManipulator(real_r x, real_r y, real_r z);
    virtual void        Identify(Layout *layout);
    virtual bool        DrawHandles(Layout *layout);

protected:
    real_r              x, y, z;
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
