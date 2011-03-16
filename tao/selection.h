#ifndef SELECTION_H
#define SELECTION_H
// ****************************************************************************
//  selection.h                                                     Tao project
// ****************************************************************************
//
//   File Description:
//
//     An activity for selection rectangles and clicks
//
//
//
//
//
//
//
//
// ****************************************************************************
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "activity.h"
#include "coords3d.h"
#include <vector>

TAO_BEGIN

struct Identify : Activity
// ----------------------------------------------------------------------------
//   Identify the object under the mouse
// ----------------------------------------------------------------------------
{
    Identify(text t, Widget *w);

public:
    typedef std::vector<uint> id_list;

public:
    uint        ObjectAtPoint(coord x, coord y);
    uint        ObjectInRectangle(const Box &rectangle,
                                  uint *handleId = 0, // Selection handle or 0
                                  uint *charId   = 0, // Selected char or 0
                                  uint *childId  = 0, // Selected child or 0
                                  uint *parentId = 0);// Parent or 0
    int         ObjectsInRectangle(const Box &rectangle, id_list &list);
};


struct MouseFocusTracker : Identify
// ----------------------------------------------------------------------------
//   Track which object gets the focus based on mouse movements
// ----------------------------------------------------------------------------
{
    MouseFocusTracker(text t, Widget *w);
    virtual Activity *  MouseMove(int x, int y, bool active);
    uint previous;
};


struct Selection : Identify
// ----------------------------------------------------------------------------
//   A selection (represented by a selection rectangle)
// ----------------------------------------------------------------------------
{
    Selection(Widget *w);

    virtual Activity *  Display(void);
    virtual Activity *  Idle(void);
    virtual Activity *  Click(uint button, uint count, int x, int y);
    virtual Activity *  MouseMove(int x, int y, bool active);

public:
    typedef std::map<uint, uint> selection_map; // Widget::selection_map
    static bool selectionsMatch(selection_map &s1, selection_map &s2);
    Box                 rectangle;
    selection_map       savedSelection;
};

TAO_END

#endif // SELECTION_H
