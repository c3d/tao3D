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
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Lionel Schaffhauser <lionel@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "widget.h"
#include "tree.h"
#include "tao_tree.h"
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
    typedef const Point3 &      kPoint3;

    Manipulator(Tree *self);

    virtual void        Draw(Layout *layout);
    virtual void        DrawSelection(Layout *layout);
    virtual void        Identify(Layout *layout);
    virtual bool        DrawHandle(Layout *layout, Point3 p, uint id,
                                   text name = "handle");
    virtual bool        DrawHandles(Layout *layout) = 0;

    virtual Tree *      Source();

protected:
    void                updateArg(Widget *widget, Tree *arg,
                                  double first, double previous, double current,
                                  double min = -1e10, double max = 1e10);
    void                rotate(Widget *widget, Tree *shape, kPoint3 center,
                               kPoint3 p0, kPoint3 p1, kPoint3 p2, bool stepped);

    Tree_p              self;
};


struct ControlPoint : Manipulator
// ----------------------------------------------------------------------------
//    A control point in an object like a path
// ----------------------------------------------------------------------------
{
    ControlPoint(Tree * self, Real * x, Real * y, Real * z, uint id);
    virtual bool        DrawHandles(Layout *layout);
    virtual void        Draw(Layout *layout);
    virtual void        DrawSelection(Layout *layout);
    virtual void        Identify(Layout *layout);

    Real_p              x, y, z;
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

    FrameManipulator(Tree *self,
                     Real *x, Real *y, Real *w, Real *h);
    virtual void        DrawSelection(Layout *layout);
    virtual bool        DrawHandles(Layout *layout);
    virtual Box3        Bounds(Layout *);
    virtual TransformMode CurrentTransformMode();
    virtual double      ScaleX(void)       { return 1; }
    virtual double      ScaleY(void)       { return 1; }

protected:
    Real_p              x, y, w, h;
};


struct ControlRectangle : FrameManipulator
// ----------------------------------------------------------------------------
//   Manipulators for a rectangle-bounded object
// ----------------------------------------------------------------------------
{
    ControlRectangle(Tree *self, Real *x, Real *y, Real *w, Real *h);
    virtual bool        DrawHandles(Layout *layout);
};


struct ControlRoundedRectangle : ControlRectangle
// ----------------------------------------------------------------------------
//   Manipulators for a rectangle-bounded object
// ----------------------------------------------------------------------------
{
    ControlRoundedRectangle(Tree *self,
                            Real *x, Real *y, Real *w, Real *h,
                            Real *r);
    virtual bool        DrawHandles(Layout *layout);

    protected:
    Real_p              r;
};


struct ControlArrow : ControlRectangle
// ----------------------------------------------------------------------------
//   Manipulators for a arrow object
// ----------------------------------------------------------------------------
{
    ControlArrow(Tree *self,
                 Real *x, Real *y, Real *w, Real *h,
                 Real *ax, Real *ary);
    ControlArrow(Tree *self,
                 Real *x, Real *y, Real *w, Real *h,
                 Real *ax, Real *ary, bool is_double);
    virtual bool        DrawHandles(Layout *layout);

protected:
    Real_p              ax, ary;
    bool                d;
};


struct ControlPolygon : ControlRectangle
// ----------------------------------------------------------------------------
//   Manipulators for a polygon object
// ----------------------------------------------------------------------------
{
    ControlPolygon(Tree *self,
                   Real *x, Real *y, Real *w, Real *h, Integer *p);
    virtual bool        DrawHandles(Layout *layout);

protected:
    Integer_p           p;
};


struct ControlStar : ControlPolygon
// ----------------------------------------------------------------------------
//   Manipulators for a star object
// ----------------------------------------------------------------------------
{
    ControlStar(Tree *self,
                Real *x, Real *y, Real *w, Real *h, Integer *p, Real *r);
    virtual bool        DrawHandles(Layout *layout);

protected:
    Real_p              r;
};


struct ControlBalloon : ControlRoundedRectangle
// ----------------------------------------------------------------------------
//   Manipulators for a Balloon object
// ----------------------------------------------------------------------------
{
    ControlBalloon(Tree *self,
                   Real *x, Real *y, Real *w, Real *h, Real *r,
                   Real *ax, Real *ay);
    virtual bool        DrawHandles(Layout *layout);

protected:
    Real_p              ax, ay;
};


struct ControlCallout : ControlBalloon
// ----------------------------------------------------------------------------
//   Manipulators for a Callout object
// ----------------------------------------------------------------------------
{
    ControlCallout(Tree *self,
                   Real *x, Real *y, Real *w, Real *h, Real *r,
                   Real *ax, Real *ay, Real *d);
    virtual bool        DrawHandles(Layout *layout);

protected:
    Real_p              d;
};


struct ImageManipulator : ControlRectangle
// ----------------------------------------------------------------------------
//   Manipulators for a rectangle-bounded object
// ----------------------------------------------------------------------------
{
    ImageManipulator(Tree *self,
                     Real *x, Real *y,
                     Real *sx, Real *sy,
                     double w, double h);
    virtual double   ScaleX(void)       { return 1/w0; }
    virtual double   ScaleY(void)       { return 1/h0; }
    double           w0, h0;
};


struct WidgetSurface;
struct WidgetManipulator : FrameManipulator
// ----------------------------------------------------------------------------
//   Manipulators for widgets
// ----------------------------------------------------------------------------
{
    WidgetManipulator(Tree *self,
                      Real *x, Real *y, Real *w, Real *h,
                      WidgetSurface *s);
    virtual void        DrawSelection(Layout *layout);

protected:
    WidgetSurface *     surface;
};


struct Table;
struct TableManipulator : FrameManipulator
// ----------------------------------------------------------------------------
//   Manipulator for tables
// ----------------------------------------------------------------------------
{
    TableManipulator(Tree *self, Real *x, Real *y, Table *table);
    virtual bool        DrawHandles(Layout *layout);

protected:
    Table *             table;
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
};


struct BoxManipulator : Manipulator
// ----------------------------------------------------------------------------
//   Dispays 8 handles in the corner, but clicks in the volume pass through
// ----------------------------------------------------------------------------
{
    BoxManipulator(Tree *self,
                   Real *x, Real *y, Real *z, Real *w, Real *h, Real *d);
    virtual bool        DrawHandles(Layout *layout);

protected:
    Real_p              x, y, z, w, h, d;
};


struct ControlBox : BoxManipulator
// ----------------------------------------------------------------------------
//   Manipulators for a box-bounded object
// ----------------------------------------------------------------------------
{
    ControlBox(Tree *self,
               Real *x, Real *y, Real *z, Real *w, Real *h, Real *d);
    virtual bool        DrawHandles(Layout *layout);
};


struct TransformManipulator : Manipulator
// ----------------------------------------------------------------------------
//   Manipulators for transform objects
// ----------------------------------------------------------------------------
{
    TransformManipulator(Tree *self);
};


struct RotationManipulator : TransformManipulator
// ----------------------------------------------------------------------------
//   Manipulation of a rotation axis and amount
// ----------------------------------------------------------------------------
{
    RotationManipulator(Tree *self, Real *a, Real *x, Real *y, Real *z);
    virtual void        Identify(Layout *layout);
    virtual bool        DrawHandles(Layout *layout);

protected:
    Real_p              a, x, y, z;
};


struct TranslationManipulator : TransformManipulator
// ----------------------------------------------------------------------------
//   Manipulation of translation along 3 axes
// ----------------------------------------------------------------------------
{
    TranslationManipulator(Tree *self, Real *x, Real *y, Real *z);
    virtual void        Identify(Layout *layout);
    virtual bool        DrawHandles(Layout *layout);

protected:
    Real_p              x, y, z;
};


struct ScaleManipulator : TransformManipulator
// ----------------------------------------------------------------------------
//   Manipulation of scale along 3 axes
// ----------------------------------------------------------------------------
{
    ScaleManipulator(Tree *self, Real *x, Real *y, Real *z);
    virtual void        Identify(Layout *layout);
    virtual bool        DrawHandles(Layout *layout);

protected:
    Real_p              x, y, z;
};

TAO_END

#endif // MANIPULATOR_H
