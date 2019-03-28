// *****************************************************************************
// init_cleanup.cpp                                                Tao3D project
// *****************************************************************************
//
// File description:
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
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2011,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2013, Jérôme Forissier <jerome@taodyne.com>
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
    record(tao_app, "XL symbols initialization");
    XL::EnterBasics();
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
    record(tao_app, "XL symbols initialization complete");
}


void Cleanup()
// ----------------------------------------------------------------------------
//   Cleaning up before exit
// ----------------------------------------------------------------------------
{
    record(tao_app, "XL symbols cleanup");

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

    record(tao_app, "XL symbols cleanup complete");
}

}
