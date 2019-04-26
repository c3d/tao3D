// *****************************************************************************
// table.cpp                                                       Tao3D project
// *****************************************************************************
//
// File description:
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
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2010-2011, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010-2011,2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2011, Jérôme Forissier <jerome@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Tao3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Tao3D, in a file named COPYING.
// If not, see <https://www.gnu.org/licenses/>.
// *****************************************************************************

#include "table.h"
#include "layout.h"
#include "graphics.h"
#include "runtime.h"
#include "widget.h"

TAO_BEGIN

Table::Table(Widget *w, Scope *scope, Real_p x, Real_p y, uint r, uint c)
// ----------------------------------------------------------------------------
//    Constructor
// ----------------------------------------------------------------------------
    : Layout(w), scope(scope),
      x(x), y(y), rows(r), columns(c), row(0), column(0),
      margins(0,0,5,5), columnWidth(), rowHeight(),
      fill(NULL), border(NULL), fills(), borders()
{}


Table::~Table()
// ----------------------------------------------------------------------------
//   Destructor
// ----------------------------------------------------------------------------
{
    for (Drawings::iterator it = items.begin(); it != items.end(); it++)
        delete *it;
    items.clear();

    for (Layouts::iterator f = fills.begin(); f != fills.end(); f++)
        delete *f;
    for (Layouts::iterator b = borders.begin(); b != borders.end(); b++)
        delete *b;
}


void Table::Evaluate(Layout *where)
// ----------------------------------------------------------------------------
//   Compute the layout of the table, then evaluate fill forms
// ----------------------------------------------------------------------------
{
    // Compute the layout of the whole table
    Compute(where);

    // Draw cell fill and border for each individual cell
    Widget *widget = Display();
    XL::Save<Table *> saveTable(widget->table, this);
    TreeList::iterator fi = cellFill.begin();
    TreeList::iterator bi = cellBorder.begin();

    // Check that Table::Add invariant was respected
    assert (cellFill.size() == rows * columns);
    assert (cellBorder.size() == rows * columns);
    assert (fills.size() == 0);
    assert (borders.size() == 0);

    coord   cellX, cellY, cellW, cellH;
    coord   x0 = double(x) - bounds.Width()/2;
    coord   y0 = double(y) - bounds.Height()/2;
    coord   px = bounds.lower.x + x0;
    coord   py = bounds.upper.y + y0;


    // Loop on all cell items, creating the layouts for the fill and border
    for (row = 0; row < rows; row++)
    {
        px = bounds.lower.x + x0;
        if (row < rowHeight.size())
            py -= rowHeight[row];
        py -= margins.Height();

        for (column = 0; column < columns; column++)
        {
            Vector3 pos(px - margins.lower.x + columnOffset[column],
                        py - margins.lower.y + rowOffset[row], 0);

            cellW = columnWidth[column] + margins.Width();
            cellH = rowHeight[row] + margins.Height();
            cellX = px + cellW/2;
            cellY = py + cellH/2;
            cellBox = Box(cellX-cellW/2, cellY-cellH/2, cellW, cellH);

            Layout *fillLayout = NULL;
            if (Tree_p fill = *fi++)
                fillLayout = widget->drawTree(this, scope, fill);
            fills.push_back(fillLayout);

            Layout *borderLayout = NULL;
            if (Tree_p border = *bi++)
                borderLayout = widget->drawTree(this, scope, border);
            borders.push_back(borderLayout);

            if (column < columnWidth.size())
                px += columnWidth[column];
            px += margins.Width();
        }
    }
}


void Table::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Draw all the table items
// ----------------------------------------------------------------------------
{
    Compute(where);

    coord   cellX, cellY, cellW, cellH;
    coord   x0 = double(x) - bounds.Width()/2;
    coord   y0 = double(y) - bounds.Height()/2;
    coord   px = bounds.lower.x + x0;
    coord   py = bounds.upper.y + y0;
    uint    r, c;
    Widget *widget = where->Display();
    Drawings::iterator i = items.begin();
    Layouts::iterator fi = fills.begin();
    Layouts::iterator bi = borders.begin();
    XL::Save<Table *> saveTable(widget->table, this);

    // Check that we correctly computed layouts in Evaluate and Compute
    assert(fills.size() == rows * columns);
    assert(borders.size() == rows * columns);

    // Loop on all elements to draw
    for (r = 0; r < rows; r++)
    {
        px = bounds.lower.x + x0;
        if (r < rowHeight.size())
            py -= rowHeight[r];
        py -= margins.Height();
        row = r;
        for (c = 0; c < columns; c++)
        {
            bool atEnd = i == items.end();

            Drawing *d = atEnd ? NULL : *i++;
            Vector3 pos(px - margins.lower.x + columnOffset[c],
                        py - margins.lower.y + rowOffset[r], 0);
            column = c;

            cellW = columnWidth[c] + margins.Width();
            cellH = rowHeight[r] + margins.Height();
            cellX = px + cellW/2;
            cellY = py + cellH/2;
            cellBox = Box(cellX-cellW/2, cellY-cellH/2, cellW, cellH);

            if (Layout *cell = dynamic_cast<Layout *>(d))
            {
                XL::Save<Point3> zeroOffset(offset, Point3());
                Box3 bb = d->Space(this);
                pos.x += (columnWidth[c]-bb.Width() ) * cell->alongX.centering;
                pos.y += (rowHeight[r]-bb.Height()) * cell->alongY.centering;
            }

            if (fi != fills.end())
                if (Layout *fillLayout = *fi++)
                    fillLayout->Draw(where);

            if (d)
            {
                XL::Save<Point3> saveOffset(offset, pos + where->offset);
                d->Draw(this);
            }

            if (bi != borders.end())
                if (Layout *borderLayout = *bi++)
                    borderLayout->Draw(where);

            if (c < columnWidth.size())
                px += columnWidth[c];
            px += margins.Width();
        }
    }
}


void Table::DrawSelection(Layout *where)
// ----------------------------------------------------------------------------
//   Draw the selection for all the table items
// ----------------------------------------------------------------------------
{
    Compute(where);

    coord   cellX, cellY, cellW, cellH;
    coord   x0 = double(x) - bounds.Width()/2;
    coord   y0 = double(y) - bounds.Height()/2;
    coord   px = bounds.lower.x + x0;
    coord   py = bounds.upper.y + y0;
    uint    r, c;
    Widget *widget = where->Display();
    Drawings::iterator i = items.begin();
    XL::Save<Table *> saveTable(widget->table, this);

    for (r = 0; r < rows; r++)
    {
        px = bounds.lower.x + x0;
        if (r < rowHeight.size())
            py -= rowHeight[r];
        py -= margins.Height();
        row = r;
        for (c = 0; c < columns; c++)
        {
            bool atEnd = i == items.end();

            Drawing *d = atEnd ? NULL : *i++;
            Vector3 pos(px - margins.lower.x + columnOffset[c],
                        py - margins.lower.y + rowOffset[r], 0);
            column = c;

            cellW = columnWidth[c] + margins.Width();
            cellH = rowHeight[r] + margins.Height();
            cellX = px + cellW/2;
            cellY = py + cellH/2;
            cellBox = Box(cellX-cellW/2, cellY-cellH/2, cellW, cellH);

            if (Layout *cell = dynamic_cast<Layout *>(d))
            {
                XL::Save<Point3> zeroOffset(offset, Point3());
                Box3 bb = d->Space(this);
                pos.x += (columnWidth[c]-bb.Width() ) * cell->alongX.centering;
                pos.y += (rowHeight[r]-bb.Height()) * cell->alongY.centering;
            }

            if (d)
            {
                XL::Save<Point3> saveOffset(offset, where->offset + pos);
                d->DrawSelection(this);
            }

            if (c < columnWidth.size())
                px += columnWidth[c];
            px += margins.Width();
        }
    }
}


void Table::Identify(Layout *where)
// ----------------------------------------------------------------------------
//   Identify all the table items
// ----------------------------------------------------------------------------
{
    Compute(where);

    coord   cellX, cellY, cellW, cellH;
    coord   x0 = double(x) - bounds.Width()/2;
    coord   y0 = double(y) - bounds.Height()/2;
    coord   px = bounds.lower.x + x0;
    coord   py = bounds.upper.y + y0;
    uint    r, c;
    Widget *widget = where->Display();
    Drawings::iterator i = items.begin();
    XL::Save<Table *> saveTable(widget->table, this);

    for (r = 0; r < rows; r++)
    {
        px = bounds.lower.x + x0;
        if (r < rowHeight.size())
            py -= rowHeight[r];
        py -= margins.Height();
        row = r;
        for (c = 0; c < columns; c++)
        {
            bool atEnd = i == items.end();

            Drawing *d = atEnd ? NULL : *i++;
            Vector3 pos(px - margins.lower.x + columnOffset[c],
                        py - margins.lower.y + rowOffset[r], 0);
            column = c;

            cellW = columnWidth[c] + margins.Width();
            cellH = rowHeight[r] + margins.Height();
            cellX = px + cellW/2;
            cellY = py + cellH/2;
            cellBox = Box(cellX-cellW/2, cellY-cellH/2, cellW, cellH);

            if (Layout *cell = dynamic_cast<Layout *>(d))
            {
                XL::Save<Point3> zeroOffset(offset, Point3());
                Box3 bb = d->Space(this);
                pos.x += (columnWidth[c]-bb.Width() ) * cell->alongX.centering;
                pos.y += (rowHeight[r]-bb.Height()) * cell->alongY.centering;
            }

            if (d)
            {
                XL::Save<Point3> saveOffset(offset, where->offset + pos);
                d->Identify(this);
            }

            if (c < columnWidth.size())
                px += columnWidth[c];
            px += margins.Width();
        }
    }
}


Box3 Table::Bounds(Layout *where)
// ----------------------------------------------------------------------------
//   Compute the bounds of the table
// ----------------------------------------------------------------------------
{
    Compute(where);
    Vector3 boff(-bounds.Width()/2, -bounds.Height()/2, 0);
    return bounds + where->offset + boff;
}


Box3 Table::Space(Layout *where)
// ----------------------------------------------------------------------------
//   Compute the bounds of the table
// ----------------------------------------------------------------------------
{
    return Bounds(where);
}


void Table::Add(Drawing *d)
// ----------------------------------------------------------------------------
//    Add an element to a table
// ----------------------------------------------------------------------------
{
    // Normal layout add
    Layout::Add(d);

    // Must recompute all column and row positions
    columnWidth.clear();
    rowHeight.clear();

    // Add current fill to cell fill and cell border lists
    cellFill.push_back(fill);
    cellBorder.push_back(border);
}


void Table::Compute(Layout *where)
// ----------------------------------------------------------------------------
//   Compute column widths and row heights
// ----------------------------------------------------------------------------
{
    Inherit(where);
    if (columnWidth.size() == columns && rowHeight.size() == rows)
        return;

    // Check that we don't get here after fills and borders have been computed
    assert (fills.size() == 0);
    assert (borders.size() == 0);

    uint r, c;
    Drawings::iterator i = items.begin();
    std::vector<Box3> rowBB, colBB;

    // Compute the actual column width and heights
    columnWidth.clear();
    columnWidth.resize(columns);
    colBB.resize(columns);
    columnOffset.clear();
    columnOffset.resize(columns);

    rowHeight.clear();
    rowHeight.resize(rows);
    rowBB.resize(rows);
    rowOffset.clear();
    rowOffset.resize(rows);

    // Set initial (empty) bounding box for rows and columns
    for (r = 0; r < rows; r++)
        rowBB[r] |= Point3(0,0,0);
    for (c = 0; c < columns; c++)
        colBB[c] |= Point3(0,0,0);

    // Compute actual row and column bounding box
    for (r = 0; r < rows; r++)
    {
        if (i == items.end())
            break;
        for (c = 0; c < columns; c++)
        {
            if (i == items.end())
                break;
            Drawing *d = *i++;
            Inherit(where);
            XL::Save<Point3> zeroOffset(offset, Point3());
            Box3 bb = d->Space(this);
            double lowX = colBB[c].lower.x;
            double lowY = rowBB[r].lower.y;
            if (lowX < 0)
                columnOffset[c] = -lowX;
            if (lowY < 0)
                rowOffset[r] = -lowY;
            rowBB[r] |= bb;
            colBB[c] |= bb;
        }
    }

    // Compute the bounding box
    Box3 bb;
    bb |= Point3(0,0,0);
    for (c = 0; c < columns; c++)
    {
        columnWidth[c] = colBB[c].Width();
        bb.upper.x += columnWidth[c] + margins.Width();
    }
    for (r = 0; r < rows; r++)
    {
        rowHeight[r] = rowBB[r].Height();
        bb.upper.y += rowHeight[r] + margins.Height();
    }
    bounds = bb;

    // Restore original state for the caller
    Inherit(where);
}


void Table::NextCell()
// ----------------------------------------------------------------------------
//   Increase row and column for the next cell
// ----------------------------------------------------------------------------
{
    column++;
    if (column >= columns)
    {
        column = 0;
        row++;
        if (row >= rows)
            row = 0;
    }
}

TAO_END
