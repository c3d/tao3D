// ****************************************************************************
//  init_cleanup.cpp                                                Tao project
// ****************************************************************************
//
//   File Description:
//
//     Separate initialization and cleanup code to minimize compilation times
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
//  (C) 2011 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2011 Taodyne SAS
// ****************************************************************************

#include "init_cleanup.h"

#include "graphics.h"
#include "basics.h"
#include "attributes.h"
#include "shapes.h"
#include "shapes3d.h"
#include "manipulator.h"
#include "transforms.h"
#include "lighting.h"
#include "table.h"
#include "text_drawing.h"
#include "widget_surface.h"
#include "frame.h"
#include "chooser.h"
#include "formulas.h"


namespace Tao
{

void Initialize()
// ----------------------------------------------------------------------------
//    Initialize application symbols
// ----------------------------------------------------------------------------
{
    Tao::EnterGraphics();
    Tao::EnterAttributes();
    Tao::EnterShapes();
    Tao::EnterShapes3D();
    Tao::EnterTables();
    Tao::EnterFrames();
    Tao::EnterTextDrawing();
    Tao::EnterWidgetSurfaces();
    Tao::EnterManipulators();
    Tao::EnterTransforms();
    Tao::EnterLighting();
    Tao::EnterChooser();
}


void Cleanup()
// ----------------------------------------------------------------------------
//   Cleaning up before exit
// ----------------------------------------------------------------------------
{
    // First, discard ALL global (smart) pointers to XL types/names
    XL::DeleteBasics();
    Tao::DeleteGraphics();     // REVISIT: move to Tao:: namespace?
    Tao::DeleteAttributes();
    Tao::DeleteShapes();
    Tao::DeleteShapes3D();
    Tao::DeleteTables();
    Tao::DeleteFrames();
    Tao::DeleteTextDrawing();
    Tao::DeleteWidgetSurfaces();
    Tao::DeleteManipulators();
    Tao::DeleteTransforms();
    Tao::DeleteLighting();
    Tao::DeleteChooser();

    TaoFormulas::DeleteFormulas();

    // No more global refs, deleting GC will purge everything
    XL::GarbageCollector::Delete();
}

}
