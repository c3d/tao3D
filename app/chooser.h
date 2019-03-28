#ifndef CHOOSER_H
#define CHOOSER_H
// *****************************************************************************
// chooser.h                                                       Tao3D project
// *****************************************************************************
//
// File description:
//
//    The chooser is a way to select a command or item in a large set.
//    It works by showing a list, and then allowing you to restrict that
//    list by typing keystrokes.
//
//
//
//
//
//
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2010, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010-2011,2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2012, Jérôme Forissier <jerome@taodyne.com>
// (C) 2010, Lionel Schaffhauser <lionel@taodyne.com>
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

#include "activity.h"
#include "tao_tree.h"


TAO_BEGIN

struct ChooserItem
// ----------------------------------------------------------------------------
//   An entry in a chooser
// ----------------------------------------------------------------------------
{
    ChooserItem(text c, XL::Tree *f): caption(c), function(f) {}
    text        caption;
    XL::Tree_p  function;
};
typedef std::vector<ChooserItem> ChooserItems;


struct Chooser : Activity
// ----------------------------------------------------------------------------
//  A chooser shows a selection among the possible commands
// ----------------------------------------------------------------------------
{
    Chooser(SourceFile *program, text name, Widget *w);
    virtual ~Chooser();

    // Callbacks, return true if this activity 'handled' the event
    virtual Activity *  Display(void);
    virtual Activity *  Idle(void);
    virtual Activity *  Key(text key);
    virtual Activity *  Click(uint button, uint count, int x, int y);
    virtual Activity *  MouseMove(int x, int y, bool active);

    // Add an element to a chooser
    void                AddItem(text caption, XL::Tree *function);
    void                AddCommands(text prefix, text label);
    void                SetCurrentItem(text caption);

public:
    SourceFile * xlProgram;
    text         keystrokes;
    int          item;
    int          firstItem;
    ChooserItems items;
    XL::Tree_p   selected;
};



// ============================================================================
//
//   Entering chooser items in the symbols table
//
// ============================================================================

extern void EnterChooser();
extern void DeleteChooser();

TAO_END

#endif // CHOOSER_H
