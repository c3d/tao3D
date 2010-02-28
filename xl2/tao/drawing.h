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

#include "coord3d.h"

TAO_BEGIN

struct Drawing
// ----------------------------------------------------------------------------
//   Something that can be drawn in a layout
// ----------------------------------------------------------------------------
//   A single Drawing class can represent a number of items in a layout
//   For example, a text Drawing can represent consecutive glyphs.
//   In that case, the caller is supposed to iterate until More() == false
{
    Drawing() {}
    virtual ~Drawing() {}
    virtual void        Draw(const Point &where)  {}
    virtual bool        IsWordBreak()   { return false; }
    virtual bool        IsLineBreak()   { return false; }
    virtual bool        IsAttribute()   { return false; }
    virtual Box         Bounds()        { return Box(); }
    virtual bool        More()          { return false; }
};

TAO_END

#endif // DRAWING_H
