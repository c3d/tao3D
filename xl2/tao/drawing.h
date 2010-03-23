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
                        Drawing() {}
    virtual             ~Drawing() {}
    virtual void        Draw(Layout *)  {}
    virtual Box3        Bounds()        { return Box3(); }
    virtual Box3        Space()         { return Bounds(); }
    virtual bool        IsWordBreak()   { return false; }
    virtual bool        IsLineBreak()   { return false; }
    virtual bool        IsAttribute()   { return false; }
};

TAO_END

#endif // DRAWING_H
