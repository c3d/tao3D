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
#include "runtime.h"
#include <GL/glew.h>
#include <QtGui>

TAO_BEGIN

// ============================================================================
//
//   Identify:  Find objects under the cursor
//
// ============================================================================

Identify::Identify(text t, Widget *w)
// ----------------------------------------------------------------------------
//   Initialize the activity
// ----------------------------------------------------------------------------
    : Activity(t, w)
{}


uint Identify::ObjectAtPoint(coord x, coord y)
// ----------------------------------------------------------------------------
//   Find the object under the cursor at the given coordinates
// ----------------------------------------------------------------------------
{
    Box rectangle(x, y, 2, 2);
    return ObjectInRectangle(rectangle);
}


uint Identify::ObjectInRectangle(const Box &rectangle,
                                 uint      *handlePtr,
                                 uint      *characterPtr,
                                 uint      *childPtr,
                                 uint      *parentPtr)
// ----------------------------------------------------------------------------
//   Find the top-most object in the given box, return # of objects
// ----------------------------------------------------------------------------
{
    // Create the select buffer and switch to select mode
    GLuint capacity      = widget->selectionCapacity();
    GLuint selected      = 0;
    GLuint handleId      = 0;
    GLuint charSelected  = 0;
    GLuint childSelected = 0;
    GLuint parentId      = 0;
    int    hits          = 0;

    GLuint *buffer = new GLuint[capacity];
    glSelectBuffer(capacity, buffer);
    glRenderMode(GL_SELECT);

    // Adjust viewport for rendering
    widget->setup(widget->width(), widget->height(), &rectangle);

    // Initialize names
    glInitNames();

    // Draw the items in "Identity" mode (simplified drawing)
    widget->identifySelection();

    // Get number of hits and extract selection
    // Each record is as follows:
    // [0]: Depth of the name stack
    // [1]: Minimum depth
    // [2]: Maximum depth
    // [3..3+[0]-1]: List of names

    hits = glRenderMode(GL_RENDER);
    if (hits > 0)
    {
        GLuint depth = ~0U;
        GLuint *ptr = buffer;
        for (int i = 0; i < hits && !handleId; i++)
        {
            uint    size    = ptr[0];
            GLuint *selPtr  = ptr + 3;
            GLuint *selNext = selPtr + size;

            if (ptr[3] && ptr[1] <= depth)
            {
                depth = ptr[1];
                childSelected = false;

                // Walk down the hierarchy if item is in a group
                ptr += 3;
                parentId = selected = *ptr++;

                // Check if we have a handleId or character ID
                while (ptr < selNext)
                {
                    GLuint child = *ptr++;
                    if (child & Widget::HANDLE_SELECTED)
                        handleId = child & ~Widget::HANDLE_SELECTED;
                    else if (child & Widget::CHARACTER_SELECTED)
                        charSelected = child & ~Widget::CHARACTER_SELECTED;
                    else if (selected & Widget::CONTAINER_OPENED)
                        selected = child;
                    else if (!childSelected)
                        childSelected = child;
                }
            }

            ptr = selNext;
        }
    }

    delete[] buffer;

    // Update output arguments
    if (handlePtr)
        *handlePtr = handleId;
    if (characterPtr)
        *characterPtr = charSelected;
    if (childPtr)
        *childPtr = childSelected;
    if (parentPtr)
        *parentPtr = parentId;

    selected &= Widget::SELECTION_MASK;

    return selected;
}


int Identify::ObjectsInRectangle(const Box &rectangle, id_list &list)
// ----------------------------------------------------------------------------
//   Return the list of objects under the rectangle
// ----------------------------------------------------------------------------
{
    int    hits     = 0;
    GLuint selected = 0;
    GLuint capacity = widget->selectionCapacity();

    // Create the select buffer and switch to select mode
    GLuint *buffer = new GLuint[capacity];
    glSelectBuffer(capacity, buffer);
    glRenderMode(GL_SELECT);

    // Adjust viewport for rendering
    widget->setup(widget->width(), widget->height(), &rectangle);

    // Initialize names
    glInitNames();

    // Draw the items in "Identity" mode (simplified drawing)
    widget->identifySelection();

    // Get number of hits and extract selection
    // Each record is as follows:
    // [0]: Depth of the name stack
    // [1]: Minimum depth
    // [2]: Maximum depth
    // [3..3+[0]-1]: List of names
    hits = glRenderMode(GL_RENDER);
    if (hits > 0)
    {
        GLuint *ptr = buffer;
        for (int i = 0; i < hits; i++)
        {
            // With a drag rectangle, we only select the top item in a
            // hierarchy. For example, in a group, we only select the top group
            uint size = ptr[0];
            selected = ptr[3];
            list.push_back(selected);
            ptr += 3 + size;
        }
    }
    delete[] buffer;

    if (hits < 0)
        std::cout << "Capacity is incorrectly computed\n";

    return hits;
}


// ============================================================================
//
//    MouseFocusTracker : Find item under cursor, give it focus if it's a widget
//
// ============================================================================

MouseFocusTracker::MouseFocusTracker(text t, Widget *w)
// ----------------------------------------------------------------------------
//   Initialize the activity
// ----------------------------------------------------------------------------
    : Identify(t, w), previous(0)
{}


Activity *MouseFocusTracker::MouseMove(int x, int y, bool active)
// ----------------------------------------------------------------------------
//   Track focus as mouse moves
// ----------------------------------------------------------------------------
{
    if (active)
        return next;

    uint current = ObjectAtPoint(x, widget->height() - y);

    if (current != previous)
    {
        if (previous > 0)
        {
            // Forward 'focus-out' to previous item
            widget->focusId = 0;
            widget->focusWidget = NULL;
        }
        if (current > 0)
        {
            // Forward 'focus-in' to current item
            widget->focusId = current;
        }
        widget->refresh();
    }

    previous = current;
    return next;
}



// ============================================================================
//
//    Selection - Shape selection and selection rectangle
//
// ============================================================================

Selection::Selection(Widget *w)
// ----------------------------------------------------------------------------
//   Initialize the selection rectangle
// ----------------------------------------------------------------------------
    : Identify("Selection Rectangle", w)
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
    widget->drawSelection(NULL, b3, "selection_rectangle", 0);

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
        // count == 0: MB up
        // count == 1: single click
        // count == 2: double click
        if (count)
        {
            firstClick = true;
            rectangle.lower.Set(x-2, y-2);
            rectangle.upper.Set(x+2, y+2);
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


    // Get the object at the click point
    GLuint selected      = 0;
    GLuint handleId      = 0;
    GLuint charSelected  = 0;
    GLuint childSelected = 0;
    GLuint parentId      = 0;

    selected = ObjectInRectangle(rectangle,
                                 &handleId, &charSelected,
                                 &childSelected, &parentId);

    // If we selected an object, need to adjust dialogs to match new selection
    if (selected)
    {
        doneWithSelection = true;
        widget->updateDialogs();
    }

    // If this is the first click, then update selection
    Widget::selection_map oldSelection = widget->selection;
    if (firstClick)
    {
        // Check cases where we need to start with a fresh selection
        if (shiftModifier || handleId)
        {
            // User held shift or selected a control handle
            savedSelection = oldSelection;
        }
        else if (oldSelection.count(selected))
        {
            // Clicking on an already selected item
            savedSelection = oldSelection;
        }
        else
        {
            // Other cases: start with a fresh selection
            savedSelection.clear();

            // Clicking in some other child of a parent: select parent again
            parentId &= Widget::SELECTION_MASK;
            if (parentId && parentId != selected)
                selected = parentId;
        }

        widget->selection = savedSelection;

        if (selected)
        {
            if (shiftModifier && widget->selected(selected) && !handleId)
            {
                // De-select previously selected object using shift
                widget->select(selected, 0);
            }
            else if (childSelected && count == 2)
            {
                // Double-click on a container: mark it as opened
                widget->select(selected, Widget::CONTAINER_OPENED);
                widget->select(childSelected, 1);
            }
            else
            {
                // Select given object
                widget->select(selected, savedSelection[selected] + count);
                if (!shiftModifier && !handleId)
                    if (Tree *action = widget->shapeAction("click", selected))
                        xl_evaluate(action);
            }
        }
        widget->handleId = handleId;
    }
    if (!widget->selectionChanged &&
        !selectionsMatch(oldSelection, widget->selection))
        widget->selectionChanged = true;

    // In all cases, we want a screen refresh
    widget->refresh();

    // Delete any text selection we might have if we didn't click in it
    if (!charSelected)
        delete widget->textSelection();

    // If we double click in a text, create a text selection
    else if (count == 2)
    {
        TextSelect *tsel = widget->textSelection();
        if (!tsel)
            tsel = new TextSelect(widget);
        delete this;
        return tsel;
    }

    // If we are done with the selection, remove it and shift to a Drag
    if (doneWithSelection)
    {
        Widget *widget = this->widget; // Save before 'delete this'
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
        widget->refresh();
        return next;
    }

    y = widget->height() - y;
    rectangle.upper.Set(x,y);

    Widget::selection_map oldSelection = widget->selection;
    widget->selection = savedSelection;

    id_list list;
    if (ObjectsInRectangle(rectangle, list) > 0)
    {

        for (id_list::iterator i = list.begin(); i != list.end(); i++)
        {
            GLuint selected = *i;
            if (selected)
                widget->selection[selected] = !savedSelection[selected];
        }

        if (!widget->selectionChanged &&
            !selectionsMatch(oldSelection, widget->selection))
            widget->selectionChanged = true;
    }
    
    // Need a refresh
    widget->refresh();

    // We dealt with the mouse move, don't let other activities get it
    return NULL;
}


bool Selection::selectionsMatch(selection_map &s1, selection_map &s2)
// ----------------------------------------------------------------------------
//   Compare selections to check if they identify the same elements
// ----------------------------------------------------------------------------
//   We can't use operator== because we only compare keys, not values
{
    Widget::selection_map::iterator i;
    for (i = s1.begin(); i != s1.end(); i++)
        if ((*i).second)
            if (!s2.count((*i).first) || !s2[(*i).first])
                return false;
    for (i = s2.begin(); i != s2.end(); i++)
        if ((*i).second)
            if (!s1.count((*i).first) || !s1[(*i).first])
                return false;
    return true;
}


TAO_END
