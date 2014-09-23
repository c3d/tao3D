#ifndef DRAWING_H
#define DRAWING_H
// ****************************************************************************
//  drawing.h                                                       Tao project
// ****************************************************************************
//
//   File Description:
//
//      Elements that can be drawn on a 2D layout
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
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "coords3d.h"
#include "tao.h"
#include "tao_tree.h"
#include <vector>


TAO_BEGIN
struct TextureState;
struct Layout;
struct PageLayout;


struct Drawing
// ----------------------------------------------------------------------------
//   Something that can be drawn in a 2D or 3D layout
// ----------------------------------------------------------------------------
//   Draw() draws the shape in the given layout
//   Bounds() returns the untransformed bounding box for the shape
//   Space() returns the untransformed space desired around object
//   For instance, for text, Space() considers font line height, not Bounds()
{
    typedef std::vector<Drawing *>      Drawings;

                        Drawing();
                        Drawing(const Drawing &);
    virtual             ~Drawing();

    virtual void        Draw(Layout *);
    virtual void        DrawSelection(Layout *);
    virtual void        Evaluate(Layout *);
    virtual void        Identify(Layout *);
    virtual Box3        Bounds(Layout *);
    virtual Box3        Space(Layout *);
    virtual Tree *      Source();

    virtual bool        Paginate(PageLayout *page);
    virtual void        Clear();
    virtual void        ClearCaches();
    virtual scale       TrailingSpaceSize(Layout *);
    virtual bool        IsAttribute();
    virtual bool        IsRTL() { return false; }

    static uint count;
};


enum BreakOrder
// ----------------------------------------------------------------------------
//   An enumeration indicating where we can break lines
// ----------------------------------------------------------------------------
{
    NoBreak,
    CharBreak, WordBreak, SentenceBreak, LineBreak, ParaBreak,
    ColumnBreak, PageBreak,
    AnyBreak
};

TAO_END

#endif // DRAWING_H
