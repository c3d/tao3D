#ifndef SHAPENAME_H
#define SHAPENAME_H
// ****************************************************************************
//  shapename.h                                                     XLR project
// ****************************************************************************
// 
//   File Description:
// 
//    Helper class used to assign names to individual graphic shapes
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


TAO_BEGIN

struct ShapeName
// ----------------------------------------------------------------------------
//   Structure used simply to assign shape IDs during selection
// ----------------------------------------------------------------------------
{
    ShapeName(Widget *w): widget(w)        { widget->loadName(true); }
    ~ShapeName()                           { widget->loadName(false); }

public:
    Widget      *widget;
};

TAO_END

#endif // SHAPENAME_H
