#ifndef TABLE_H
#define TABLE_H
// ****************************************************************************
//  table.h                         (C) 1992-2009 Christophe de Dinechin (ddd) 
//                                                                 XL2 project 
// ****************************************************************************
// 
//   File Description:
// 
//    Table-style layout of elements
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
// ****************************************************************************
// * File       : $RCSFile$
// * Revision   : $Revision$
// * Date       : $Date$
// ****************************************************************************

#include "drawing.h"

TAO_BEGIN

struct Table : Drawing
// ----------------------------------------------------------------------------
//    A table-like layout for graphic elements
// ----------------------------------------------------------------------------
{
    Table(uint r, uint c);
    ~Table();

    virtual void        Draw(Layout *);
    virtual Box3        Bounds(Layout *);
    virtual void        AddElement(Drawing *d);
    virtual void        Compute(Layout *);

public:
    uint                   rows, columns;
    std::vector<Drawing *> elements;
    Box                    margins;
    std::vector<scale>     column_width;
    std::vector<scale>     row_height;
    Box3                   bounds;
    XL::Tree_p             fill;
    coord                  left, right, top, bottom;
    uint                   row, column;
};

TAO_END

#endif // TABLE_H
