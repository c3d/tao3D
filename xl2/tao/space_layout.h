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
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "page_layout.h"

TAO_BEGIN

struct SpaceLayout : PageLayout
// ----------------------------------------------------------------------------
//   Layout objects in 3D space
// ----------------------------------------------------------------------------
{
                        SpaceLayout();
                        ~SpaceLayout();

protected:
    Justification       alongZ;
};

TAO_END

#endif // SPACE_LAYOUT_H
