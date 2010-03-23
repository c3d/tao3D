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
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "space_layout.h"
#include "attributes.h"
#include <cairo.h>
#include <cairo-gl.h>


TAO_BEGIN

SpaceLayout::SpaceLayout()
// ----------------------------------------------------------------------------
//   Constructor sets defaults
// ----------------------------------------------------------------------------
    : PageLayout(), alongZ()
{}


SpaceLayout::~SpaceLayout()
// ----------------------------------------------------------------------------
//   Destructore
// ----------------------------------------------------------------------------
{}

TAO_END
