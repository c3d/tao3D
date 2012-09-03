#ifndef ACTIVITY_H
#define ACTIVITY_H
// ****************************************************************************
//  activity.h                                                      Tao project
// ****************************************************************************
//
//   File Description:
//
//    A temporary interactive activity in the user interface, e.g. selection
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

#include "base.h"
#include "tao.h"

TAO_BEGIN

struct Widget;

struct Activity
// ----------------------------------------------------------------------------
//  Encapsulate a temporary UI activity, e.g. selection
// ----------------------------------------------------------------------------
{
    Activity(text name, Widget *s);
    virtual ~Activity();

    // Callbacks, return true if this activity 'handled' the event
    virtual Activity *  Display(void);
    virtual Activity *  Idle(void);
    virtual Activity *  Key(text key);
    virtual Activity *  Click(uint button, uint count, int x, int y);
    virtual Activity *  MouseMove(int x, int y, bool active);

public:
    Activity *          next;
    text                name;
    Widget *            widget;
};

TAO_END

#endif // ACTIVITY_H
