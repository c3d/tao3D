// ****************************************************************************
//  table.cpp                       (C) 1992-2009 Christophe de Dinechin (ddd) 
//                                                                 XL2 project 
// ****************************************************************************
// 
//   File Description:
// 
//    Table-style layout for drawable elements
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

#include "table.h"
#include "layout.h"
#include "graphics.h"
#include "runtime.h"

TAO_BEGIN

Table::Table(uint r, uint c)
// ----------------------------------------------------------------------------
//    Constructor
// ----------------------------------------------------------------------------
    : rows(r), columns(c),
      elements(), margins(), column_width(), row_height(),
      fill(NULL)
{}


Table::~Table()
// ----------------------------------------------------------------------------
//   Destructor
// ----------------------------------------------------------------------------
{
    std::vector<Drawing *>::iterator it;
    for (it = elements.begin(); it != elements.end(); it++)
        delete *it;
    elements.clear();
}


void Table::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Draw all the table elements
// ----------------------------------------------------------------------------
{
    if (column_width.size() != columns || row_height.size() != rows)
        Compute(where);

    Box     selectionBox;
    Vector3 offset(where->offset);
    coord   px = bounds.lower.x;
    coord   py = bounds.upper.y;
    uint    r, c;
    std::vector<Drawing *>::iterator i = elements.begin();

    for (r = 0; r < rows; r++)
    {
        if (i == elements.end())
            break;

        px = bounds.lower.x;
        if (r < row_height.size())
            py -= row_height[r];
        py -= margins.lower.y + margins.upper.y;
        row = r;
        for (c = 0; c < columns; c++)
        {
            if (i == elements.end())
                break;

            Drawing *d = *i++;
            Vector3 pos(px + margins.lower.x, py + margins.lower.y, 0);
            column = c;
            left = px + offset.x;
            bottom = py + offset.y;
            right = left + column_width[c] + margins.lower.x + margins.upper.x;
            top = bottom + row_height[r] + margins.lower.y + margins.upper.y;

            XL::LocalSave<Point3> saveOffset(where->offset, pos + offset);
            d->Draw(where);

            if (fill)
                xl_evaluate(fill);
            if (c < column_width.size())
                px += column_width[c];
            px += margins.lower.x + margins.upper.x;
        }
    }
}


Box3 Table::Bounds(Layout *where)
// ----------------------------------------------------------------------------
//   Compute the bounds of the table
// ----------------------------------------------------------------------------
{
    if (column_width.size() != columns || row_height.size() != rows)
        Compute(where);
    return bounds;
}


void Table::AddElement(Drawing *d)
// ----------------------------------------------------------------------------
//    Add an element to a table
// ----------------------------------------------------------------------------
{
    elements.push_back(d);
    column_width.clear();
    row_height.clear();
}


void Table::Compute(Layout *where)
// ----------------------------------------------------------------------------
//   Compute column widths and row heights
// ----------------------------------------------------------------------------
{
    uint r, c;
    std::vector<Drawing *>::iterator i = elements.begin();
    std::vector<Box3> rowBB, colBB;

    // Compute the actual column width and heights
    column_width.clear();
    column_width.resize(columns);
    colBB.resize(columns);
    row_height.clear();
    row_height.resize(rows);
    rowBB.resize(rows);
    for (r = 0; r < rows; r++)
    {
        if (i == elements.end())
            break;
        for (c = 0; c < columns; c++)
        {
            if (i == elements.end())
                break;
            Drawing *d = *i++;
            Box3 bb = d->Bounds(where);
            rowBB[r] |= bb;
            colBB[c] |= bb;
        }
    }

    // Compute the bounding box
    Box3 bb;
    for (c = 0; c < columns; c++)
    {
        column_width[c] = colBB[c].Width();
        bb.upper.x += column_width[c];
        bb.upper.x += margins.lower.x + margins.upper.x;
    }
    for (r = 0; r < rows; r++)
    {
        row_height[r] = rowBB[r].Height();
        bb.upper.y += row_height[r];
        bb.upper.y += margins.lower.y + margins.upper.y;
    }
    bounds = bb;
}

TAO_END
