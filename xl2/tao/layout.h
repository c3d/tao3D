#ifndef LAYOUT_H
#define LAYOUT_H
// ****************************************************************************
//  layout.h                                                        Tao project
// ****************************************************************************
//
//   File Description:
//
//     Drawing object that is used to lay out objects in 2D or 3D space
//     Layouts are used to represent 2D pages, 3D spaces (Z ordering)
//     and text made of a sequence of glyphs
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

#include "drawing.h"
#include "color.h"
#include "justification.h"
#include <vector>
#include <QFont>


TAO_BEGIN

struct Layout : Drawing
// ----------------------------------------------------------------------------
//   A layout is responsible for laying out Drawing objects in 2D or 3D space
// ----------------------------------------------------------------------------
{
                        Layout();
                        Layout(const Layout &o);
                        ~Layout();

    // Drawing interface
    virtual void        Draw(Layout *where);
    virtual Box3        Bounds();
    virtual Box3        Space();

    // Layout interface
    virtual Layout &    Add (Drawing *d);
    virtual Vector3     Offset();
    virtual Layout *    NewChild()      { return new Layout(*this); }
    virtual void        Clear();

public:
    // Attributes that get propagated to children
    Vector3         offset;
    QFont           font;
    Justification   alongX, alongY, alongZ;
    Color           lineColor;
    Color           fillColor;
    uint            fillTexture;

protected:
    // List of drawing elements
    typedef std::vector<Drawing *>      layout_items;
    layout_items        items;
};

TAO_END

#endif // LAYOUT_H
