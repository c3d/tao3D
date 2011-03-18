// ****************************************************************************
//  space_layout.cpp                                                Tao project
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

#include "space_layout.h"
#include "attributes.h"

TAO_BEGIN

SpaceLayout::SpaceLayout(Widget *widget)
// ----------------------------------------------------------------------------
//   Constructor sets defaults
// ----------------------------------------------------------------------------
    : Layout(widget)
{}


SpaceLayout::~SpaceLayout()
// ----------------------------------------------------------------------------
//   Destructore
// ----------------------------------------------------------------------------
{}


Box3 SpaceLayout::Space(Layout *layout)
// ----------------------------------------------------------------------------
//   Return the space for the layout
// ----------------------------------------------------------------------------
{
    Box3 result = Bounds(layout);
    result |= space;
    return result;
}

TAO_END
