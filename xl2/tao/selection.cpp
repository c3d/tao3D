// ****************************************************************************
//  selection.cpp                   (C) 1992-2009 Christophe de Dinechin (ddd) 
//                                                                 XL2 project 
// ****************************************************************************
// 
//   File Description:
// 
//     A selection is a kind of activity for selection rectangles and clicks
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

#include "selection.h"
#include "widget.h"
#include <glew.h>
#include <QtGui>

TAO_BEGIN


Selection::Selection(Widget *w)
// ----------------------------------------------------------------------------
//   Initialize the selection rectangle
// ----------------------------------------------------------------------------
    : Activity("Selection Rectangle", w), rectangle()
{}


void Selection::Display(void)
// ----------------------------------------------------------------------------
//   Display the selection rectangle
// ----------------------------------------------------------------------------
{
    glPushMatrix();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, widget->width(), 0, widget->height());
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    ulong tick = widget->now();
    Box b = rectangle;
    b.Normalize();

    glColor4f(0.4 * sin(tick / 1.9e6) + 0.6,
              0.4 * sin(tick / 1.4e6) + 0.6,
              0.4 * sin(tick / 0.5e6) + 0.6,
              0.3);

    glBegin(GL_QUADS);
    glVertex2f (b.lower.x, b.lower.y);
    glVertex2f (b.upper.x, b.lower.y);
    glVertex2f (b.upper.x, b.upper.y);
    glVertex2f (b.lower.x, b.upper.y);
    glEnd();

    glColor4f(1.0, 0.4, 0.4, 0.9);
    glBegin(GL_LINE_LOOP);
    glVertex2f (b.lower.x, b.lower.y);
    glVertex2f (b.lower.x, b.upper.y);
    glVertex2f (b.upper.x, b.upper.y);
    glVertex2f (b.upper.x, b.lower.y);
    glEnd();

    glPopMatrix();
}


bool Selection::Idle(void)
// ----------------------------------------------------------------------------
//   Make the refresh rate shorter so that we animate the rectangle
// ----------------------------------------------------------------------------
{
    widget->refresh(NULL, 0.1);
    return false;               // Keep doing other idle activities
}


bool Selection::Click(uint button, bool down, int x, int y)
// ----------------------------------------------------------------------------
//   Initial and final click in a selection rectangle
// ----------------------------------------------------------------------------
{
    bool firstClick = false;
    bool doneWithSelection = false;
    bool shiftModifier = qApp->keyboardModifiers() & Qt::ShiftModifier;
    y = widget->height() - y;

    if (button & Qt::LeftButton)
    {
        if (down)
        {
            firstClick = true;
            rectangle.lower.Set(x, y);
            rectangle.upper = rectangle.lower;
        }
        else
        {
            rectangle.upper.Set(x, y);
            doneWithSelection = true;
        }
    }

    // Create the select buffer and switch to select mode
    GLuint *buffer = new GLuint[4 * widget->capacity];
    glSelectBuffer(4 * widget->capacity, buffer);
    glRenderMode(GL_SELECT);

    // Adjust viewport for rendering
    widget->setup(widget->width(), widget->height(), &rectangle);

    // Initialize names
    glInitNames();
    glPushName(0);

    // Run the programs, which will give us the list of selectable things
    widget->runProgram();

    // Get number of hits and extract selection
    // Each record is as follows:
    // [0]: Depth of the name stack
    // [1]: Minimum depth
    // [2]: Maximum depth
    // [3..3+[0]-1]: List of names
    int hits = glRenderMode(GL_RENDER);
    uint selected = 0;
    if (hits > 0)
    {
        GLuint depth = ~0U;
        GLuint *ptr = buffer;
        for (int i = 0; i < hits; i++)
        {
            uint size = ptr[0];
            if (ptr[1] < depth)
                selected = ptr[3];
            ptr += 3 + size;            
        }
        if (selected)
            doneWithSelection = true;
    }

    delete[] buffer;

    // If this is the first click, then update selection
    if (firstClick)
    {
        if (shiftModifier)
            widget->savedSelection = widget->selection;
        else
            widget->savedSelection.clear();
        widget->selection = widget->savedSelection;
        widget->selection.insert(selected);
    }

    // If we are done with the selection, remove it
    if (doneWithSelection)
        delete this;

    // Need a refresh
    widget->updateGL();

    return true;                // We dealt with it
}


bool Selection::MouseMove(int x, int y, bool active)
// ----------------------------------------------------------------------------
//   Track selection rectangle as mouse moves
// ----------------------------------------------------------------------------
{
    if (!active)
        return false;

    y = widget->height() - y;
    rectangle.upper.Set(x,y);

    // Create the select buffer and switch to select mode
    GLuint *buffer = new GLuint[4 * widget->capacity];
    glSelectBuffer(4 * widget->capacity, buffer);
    glRenderMode(GL_SELECT);

    // Adjust viewport for rendering
    widget->setup(widget->width(), widget->height(), &rectangle);

    // Initialize names
    glInitNames();
    glPushName(0);

    // Run the programs, which detects selected items
    widget->runProgram();

    // Get number of hits and extract selection
    // Each record is as follows:
    // [0]: Depth of the name stack
    // [1]: Minimum depth
    // [2]: Maximum depth
    // [3..3+[0]-1]: List of names
    widget->selection = widget->savedSelection;
    int hits = glRenderMode(GL_RENDER);
    uint selected = 0;
    if (hits > 0)
    {
        GLuint *ptr = buffer;
        for (int i = 0; i < hits; i++)
        {
            uint size = ptr[0];
            selected = ptr[3];
            widget->selection.insert(selected);
            ptr += 3 + size;            
        }
        
    }
    delete[] buffer;

    // Need a refresh
    widget->updateGL();

    return true;                // We dealt with the mouse move
}

TAO_END
