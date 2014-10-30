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
// This software is licensed under the GNU General Public License v3.
// See file COPYING for details.
// ****************************************************************************
// * File       : $RCSFile$
// * Revision   : $Revision$
// * Date       : $Date$
// ****************************************************************************

#include "drawing.h"
#include "layout.h"
#include "tao_tree.h"

TAO_BEGIN

struct Table : Layout
// ----------------------------------------------------------------------------
//    A table-like layout for graphic elements
// ----------------------------------------------------------------------------
{
    Table(Widget *display, Context *ctx, Real_p x, Real_p y, uint r, uint c);
    ~Table();

    virtual void        Draw(Layout *);
    virtual void        DrawSelection(Layout *);
    virtual void        Evaluate(Layout *);
    virtual void        Identify(Layout *);
    virtual Box3        Bounds(Layout *);
    virtual Box3        Space(Layout *);
    virtual void        Compute(Layout *);
    virtual void        Add (Drawing *d);

    void                NextCell();

public:
    Context_p              context;
    Real_p                 x, y;
    uint                   rows, columns;
    uint                   row, column;
    Box                    margins;
    std::vector<scale>     columnWidth;
    std::vector<scale>     rowHeight;
    std::vector<coord>     columnOffset;
    std::vector<coord>     rowOffset;
    Box3                   bounds;
    Box                    cellBox;
    Tree_p                 fill, border;
    TreeList               cellFill, cellBorder;
    Layouts                fills, borders;
};



// ============================================================================
// 
//   Entering shapes in the symbols table
// 
// ============================================================================

extern void EnterTables();
extern void DeleteTables();

TAO_END

#endif // TABLE_H
