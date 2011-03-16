#ifndef SPACE_LAYOUT_H
#define SPACE_LAYOUT_H
// ****************************************************************************
//  space_layout.h                                                  Tao project
// ****************************************************************************
//
//   File Description:
//
//     Layout objects in 3D space (z-ordering, ...)
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

#include "layout.h"
#include "justification.h"

TAO_BEGIN

struct SpaceLayout : Layout
// ----------------------------------------------------------------------------
//   Layout objects in 3D space
// ----------------------------------------------------------------------------
{
                        SpaceLayout(Widget *widget);
                        ~SpaceLayout();

    virtual Box3        Space(Layout *layout);
    virtual SpaceLayout*NewChild()      { return new SpaceLayout(*this); }

public:
    // Space requested for the layout
    Box3                space;
    Justification       alongX, alongY, alongZ;
};

TAO_END

#endif // SPACE_LAYOUT_H
