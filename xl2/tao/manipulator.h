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

struct Manipulator : Drawing
// ----------------------------------------------------------------------------
//   Structure used to manipulate XL coordinates using the mouse and keyboard
// ----------------------------------------------------------------------------
{
    typedef XL::Tree            Tree;
    typedef XL::Tree_p          Tree_p;
    typedef XL::RealRoot        real_r;
    typedef XL::IntegerRoot     integer_r;
    typedef const Point3 &      kPoint3;

    Manipulator(Tree_p self);

    virtual void        Draw(Layout *layout);
    virtual void        DrawSelection(Layout *layout);
    virtual void        Identify(Layout *layout);
    virtual bool        DrawHandle(Layout *layout, Point3 p, uint id,
                                   text name = "handle");
    virtual bool        DrawHandles(Layout *layout) = 0;

    virtual XL::Tree *  Source();

protected:
    void                updateArg(Widget *widget, Tree_p arg,
                                  double first, double previous, double current,
                                  double min = -1e10, double max = 1e10);
    void                rotate(Widget *widget, Tree_p shape, kPoint3 center,
                               kPoint3 p0, kPoint3 p1, kPoint3 p2, bool stepped);

    XL::TreeRoot        self;
};


struct ControlPoint : Manipulator
// ----------------------------------------------------------------------------
//    A control point in an object like a path
// ----------------------------------------------------------------------------
{
    ControlPoint(Tree_p self, real_r x, real_r y, real_r z, uint id);
    virtual bool        DrawHandles(Layout *layout);
    virtual void        Draw(Layout *layout);
    virtual void        DrawSelection(Layout *layout);
    virtual void        Identify(Layout *layout);

    real_r              x, y, z;
    uint                id;
};


struct FrameManipulator : Manipulator
// ----------------------------------------------------------------------------
//   Dispays 4 handles in the corner, but clicks in the surface pass through
// ----------------------------------------------------------------------------
{
    enum { TM_ROTATE_BIT = 0x10 };
    enum TransformMode
    {
        TM_FreeResize,                  // Free resizing
        TM_ResizeLockCenter,            // Resize object wrt. its center
        TM_ResizeLockAspectRatio,       // Keep width/height aspect ratio
        TM_ResizeLockCenterAndAspectRatio,
        TM_FreeCenteredRotate = TM_ROTATE_BIT,  // Free rotation around center
        TM_FreeOppositeRotate,          // Free rotation aound opposite handle
        TM_SteppedCenteredRotate,       // 45˚-step rotation
        TM_SteppedOppositeRotate,
    };

    FrameManipulator(Tree_p self,
                     real_r x, real_r y, real_r w, real_r h);
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
    ControlRectangle(Tree_p self, real_r x, real_r y, real_r w, real_r h);
    virtual bool        DrawHandles(Layout *layout);
};


struct ControlRoundedRectangle : ControlRectangle
// ----------------------------------------------------------------------------
//   Manipulators for a rectangle-bounded object
// ----------------------------------------------------------------------------
{
    ControlRoundedRectangle(Tree_p self,
                            real_r x, real_r y, real_r w, real_r h,
                            real_r r);
    virtual bool        DrawHandles(Layout *layout);

    protected:
    real_r              r;
};


struct ControlArrow : ControlRectangle
// ----------------------------------------------------------------------------
//   Manipulators for a arrow object
// ----------------------------------------------------------------------------
{
    ControlArrow(Tree_p self,
                 real_r x, real_r y, real_r w, real_r h,
                 real_r ax, real_r ary);
    ControlArrow(Tree_p self,
                 real_r x, real_r y, real_r w, real_r h,
                 real_r ax, real_r ary, bool is_double);
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
    ControlPolygon(Tree_p self,
                   real_r x, real_r y, real_r w, real_r h, integer_r p);
    virtual bool        DrawHandles(Layout *layout);

protected:
    integer_r           p;
};


struct ControlStar : ControlPolygon
// ----------------------------------------------------------------------------
//   Manipulators for a star object
// ----------------------------------------------------------------------------
{
    ControlStar(Tree_p self,
                real_r x, real_r y, real_r w, real_r h, integer_r p, real_r r);
    virtual bool        DrawHandles(Layout *layout);

protected:
    real_r              r;
};


struct ControlBalloon : ControlRoundedRectangle
// ----------------------------------------------------------------------------
//   Manipulators for a Balloon object
// ----------------------------------------------------------------------------
{
    ControlBalloon(Tree_p self,
                   real_r x, real_r y, real_r w, real_r h, real_r r,
                   real_r ax, real_r ay);
    virtual bool        DrawHandles(Layout *layout);

protected:
    real_r              ax, ay;
};


struct ControlCallout : ControlBalloon
// ----------------------------------------------------------------------------
//   Manipulators for a Callout object
// ----------------------------------------------------------------------------
{
    ControlCallout(Tree_p self,
                   real_r x, real_r y, real_r w, real_r h, real_r r,
                   real_r ax, real_r ay, real_r d);
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
    WidgetManipulator(Tree_p self,
                      real_r x, real_r y, real_r w, real_r h,
                      WidgetSurface *s);
    virtual void        DrawSelection(Layout *layout);

protected:
    WidgetSurface *     surface;
};


struct GraphicPath;
struct GraphicPathManipulator : FrameManipulator
// ----------------------------------------------------------------------------
//   Manipulator for GraphicPath
// ----------------------------------------------------------------------------
{
    GraphicPathManipulator(Tree_p self, GraphicPath *path, Tree_p path_tree);
    virtual bool        DrawHandles(Layout *layout);

protected:
    GraphicPath       * path;
    Tree_p              path_tree;
    XL::Real            x, y, w, h;
};


struct BoxManipulator : Manipulator
// ----------------------------------------------------------------------------
//   Dispays 8 handles in the corner, but clicks in the volume pass through
// ----------------------------------------------------------------------------
{
    BoxManipulator(Tree_p self,
                   real_r x, real_r y, real_r z, real_r w, real_r h, real_r d);
    virtual bool        DrawHandles(Layout *layout);

protected:
    real_r              x, y, z, w, h, d;
};


struct ControlBox : BoxManipulator
// ----------------------------------------------------------------------------
//   Manipulators for a box-bounded object
// ----------------------------------------------------------------------------
{
    ControlBox(Tree_p self,
               real_r x, real_r y, real_r z, real_r w, real_r h, real_r d);
    virtual bool        DrawHandles(Layout *layout);
};


struct TransformManipulator : Manipulator
// ----------------------------------------------------------------------------
//   Manipulators for transform objects
// ----------------------------------------------------------------------------
{
    TransformManipulator(Tree_p self);
};


struct RotationManipulator : TransformManipulator
// ----------------------------------------------------------------------------
//   Manipulation of a rotation axis and amount
// ----------------------------------------------------------------------------
{
    RotationManipulator(Tree_p self, real_r a, real_r x, real_r y, real_r z);
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
    TranslationManipulator(Tree_p self, real_r x, real_r y, real_r z);
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
    ScaleManipulator(Tree_p self, real_r x, real_r y, real_r z);
    virtual void        Identify(Layout *layout);
    virtual bool        DrawHandles(Layout *layout);

protected:
    real_r              x, y, z;
};

TAO_END

XL_BEGIN

// ============================================================================
// 
//    Specialized variant of 'TreeClone' that normalizes -(-x) into x
// 
// ============================================================================
//    Note that -(-x) shows as --x when rendered. That's probably a bug

struct NormalizedCloneMode;
template<>
inline Tree_p TreeCloneTemplate<NormalizedCloneMode>::DoPrefix(Prefix_p what)
// ----------------------------------------------------------------------------
//   Specialization of TreeClone that changes -(-x) into x
// ----------------------------------------------------------------------------
{
    if (Name_p n = what->left->AsName())
    {
        if (n->value == "-")
        {
            if (Real_p rv = what->right->AsReal())
                if (rv->value < 0)
                    return new Real(-rv->value, rv->Position());
            if (Integer_p iv = what->right->AsInteger())
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
