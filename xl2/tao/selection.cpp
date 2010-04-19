// ****************************************************************************
//  selection.h                                                     Tao project
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
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "drag.h"
#include "selection.h"
#include "widget.h"
#include "text_drawing.h"
#include "gl_keepers.h"
#include <GL/glew.h>
#include <QtGui>

TAO_BEGIN


Selection::Selection(Widget *w)
// ----------------------------------------------------------------------------
//   Initialize the selection rectangle
// ----------------------------------------------------------------------------
    : Activity("Selection Rectangle", w), rectangle()
{}


Activity *Selection::Display(void)
// ----------------------------------------------------------------------------
//   Display the selection rectangle
// ----------------------------------------------------------------------------
{
    GLStateKeeper save;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, widget->width(), 0, widget->height());
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    Box b = rectangle;
    b.Normalize();
    Box3 b3 (b.lower.x, b.lower.y, 0, b.Width(), b.Height(), 0);
    widget->setupGL();
    widget->drawSelection(b3, "selection_rectangle");

    return next;
}


Activity *Selection::Idle(void)
// ----------------------------------------------------------------------------
//   Make the refresh rate shorter so that we animate the rectangle
// ----------------------------------------------------------------------------
{
    widget->refresh();
    return next;               // Keep doing other idle activities
}


Activity *Selection::Click(uint button, uint count, int x, int y)
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
        if (count)
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
    else
    {
        Idle();
        Activity *next = this->next;
        delete this;
        return next;
    }

    // Create the select buffer and switch to select mode
    GLuint capacity = widget->selectionCapacity();
    GLuint *buffer = new GLuint[4 * capacity];
    glSelectBuffer(4 * capacity, buffer);
    glRenderMode(GL_SELECT);

    // Adjust viewport for rendering
    widget->setup(widget->width(), widget->height(), &rectangle);

    // Initialize names
    glInitNames();
    glPushName(0);

    // Run the programs, which will give us the list of selectable things
    widget->identifySelection();

    // Get number of hits and extract selection
    // Each record is as follows:
    // [0]: Depth of the name stack
    // [1]: Minimum depth
    // [2]: Maximum depth
    // [3..3+[0]-1]: List of names
    int hits = glRenderMode(GL_RENDER);
    GLuint selected = 0;
    GLuint manipulator = 0;
    bool charSelected = false;
    if (hits > 0)
    {
        GLuint depth = ~0U;
        GLuint *ptr = buffer;
        for (int i = 0; i < hits; i++)
        {
            uint size = ptr[0];
            if (ptr[3] && ptr[1] < depth)
            {
                selected = ptr[3];
                if (size > 1)
                    manipulator = ptr[4];
                depth = ptr[1];
                if (selected & Widget::CHAR_ID_BIT)
                    charSelected = true;
            }
            ptr += 3 + size;
        }
        if (selected)
            doneWithSelection = true;
    }

    delete[] buffer;

    // If this is the first click, then update selection
    if (firstClick)
    {
        if (shiftModifier || widget->selection.count(selected) || manipulator)
            widget->savedSelection = widget->selection;
        else
            widget->savedSelection.clear();
        widget->selection = widget->savedSelection;
        if (selected)
        {
            if (shiftModifier && widget->selection[selected] && !manipulator)
                widget->selection[selected] = 0;
            else
                widget->selection[selected]++;
        }
        widget->manipulator = manipulator;
    }

    // In all cases, we want a screen refresh
    Idle();

    // Delete a text selection if we didn't click in it
    if (count == 1 && !charSelected)
        delete widget->textSelection();

    // If we are done with the selection, remove it and shift to a Drag
    if (doneWithSelection)
    {
        Widget *widget = this->widget; // Save before 'delete this'
        Idle();
        delete this;
        if (selected)
            return new Drag(widget);
    }

    return NULL;                // We dealt with the event
}


Activity *Selection::MouseMove(int x, int y, bool active)
// ----------------------------------------------------------------------------
//   Track selection rectangle as mouse moves
// ----------------------------------------------------------------------------
{
    if (!active)
    {
        Idle();
        return next;
    }

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
    widget->identifySelection();

    // Get number of hits and extract selection
    // Each record is as follows:
    // [0]: Depth of the name stack
    // [1]: Minimum depth
    // [2]: Maximum depth
    // [3..3+[0]-1]: List of names
    widget->selection = widget->savedSelection;
    int hits = glRenderMode(GL_RENDER);
    GLuint selected = 0;
    if (hits > 0)
    {
        GLuint *ptr = buffer;
        for (int i = 0; i < hits; i++)
        {
            uint size = ptr[0];
            selected = ptr[3];
            if (selected)
                widget->selection[selected] = !widget->savedSelection[selected];
            ptr += 3 + size;
        }
    }
    delete[] buffer;

    // Need a refresh
    Idle();

    // We dealt with the mouse move, don't let other activities get it
    return NULL;
}

TAO_END
