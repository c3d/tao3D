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
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "coords3d.h"
#include "tao.h"
#include "tao_tree.h"


TAO_BEGIN

struct Layout;

struct Drawing
// ----------------------------------------------------------------------------
//   Something that can be drawn in a 2D or 3D layout
// ----------------------------------------------------------------------------
//   Draw() draws the shape in the given layout
//   Bounds() returns the untransformed bounding box for the shape
//   Space() returns the untransformed space desired around object
//   For instance, for text, Space() considers font line height, not Bounds()
{
                        Drawing()                { count++; }
                        Drawing(const Drawing &) { count++; }
    virtual             ~Drawing()               { count--; }

    virtual void        Draw(Layout *);
    virtual void        DrawSelection(Layout *);
    virtual void        Identify(Layout *);
    virtual Box3        Bounds(Layout *);
    virtual Box3        Space(Layout *);
    virtual Tree *      Source();

    enum BreakOrder
    {
        NoBreak,
        CharBreak, WordBreak, SentenceBreak, LineBreak, ParaBreak,
        ColumnBreak, PageBreak,
        AnyBreak
    };
    virtual Drawing *   Break(BreakOrder &order);
    virtual scale       TrailingSpaceSize(Layout *);
    virtual bool        IsAttribute();

    static uint count;
};

TAO_END

#endif // DRAWING_H
