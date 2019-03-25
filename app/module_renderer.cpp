// *****************************************************************************
// module_renderer.cpp                                             Tao3D project
// *****************************************************************************
//
// File description:
//
//    Enables a module to draw GL stuff inside a Tao document
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
// (C) 2011-2013, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
// (C) 2010-2011, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010-2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2011, Jérôme Forissier <jerome@taodyne.com>
// (C) 2010, Lionel Schaffhauser <lionel@taodyne.com>
// (C) 2011-2013, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can r redistribute it and/or modify
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


#include "tao/module_api.h"
#include "module_renderer.h"
#include "widget.h"
#include "shapes.h"
#include "application.h"
#include "attributes.h"
#include <recorder/recorder.h>

RECORDER(module_renderer, 16, "Tao module renderer - Drawing GL in a document");
RECORDER(module_renderer_error, 8, "Tao module renderer errors");

namespace Tao {

Layout* ModuleRenderer::currentLayout = NULL;

ModuleRenderer::~ModuleRenderer()
// ----------------------------------------------------------------------------
//   Destructor
// ----------------------------------------------------------------------------
{
    record(module_renderer, "Deleting [%p]", this);
    if (del)
        del(arg);
}


bool ModuleRenderer::ScheduleRender(ModuleApi::render_fn callback, void *arg)
// ----------------------------------------------------------------------------
//   Create a ModuleRendererPrivate object attached to current layout
// ----------------------------------------------------------------------------
{
    record(module_renderer, "Schedule %p (%p)", callback, arg);
    Widget::Tao()->layout->Add(new ModuleRenderer(callback, arg));
    return true;
}


bool ModuleRenderer::AddToLayout(ModuleApi::render_fn callback, void *arg,
                                 ModuleApi::delete_fn del)
// ----------------------------------------------------------------------------
//   Create a ModuleRendererPrivate object attached to current layout
// ----------------------------------------------------------------------------
{
    record(module_renderer, "Add %p (%p) del %p", callback, arg, del);
    Widget::Tao()->layout->Add(new ModuleRenderer(callback, arg, del));
    return true;
}


bool ModuleRenderer::AddToLayout2(ModuleApi::render_fn callback,
                                  ModuleApi::render_fn identify, void *arg,
                                  ModuleApi::delete_fn del)
// ----------------------------------------------------------------------------
//  Create a ModuleRendererPrivate object attached to current layout with
//  identify function
// ----------------------------------------------------------------------------
{
    record(module_renderer, "Add %p (%p) del %p identify %p",
           callback, arg, del, identify);
    Widget::Tao()->layout->Add(new ModuleRenderer(callback,identify,arg,del));
    return true;
}


bool ModuleRenderer::SetTextures()
// ----------------------------------------------------------------------------
//   Apply the textures as defined by current layout attributes
// ----------------------------------------------------------------------------
{
    return Shape::setTexture(currentLayout);
}


bool ModuleRenderer::SetShader(int id)
// ----------------------------------------------------------------------------
//   Set the shader defined by its id to the current layout attributes
// ----------------------------------------------------------------------------
{
    if(id >= 0)
        currentLayout->programId = id;

    return Shape::setShader(currentLayout);
}


bool ModuleRenderer::SetFillColor()
// ----------------------------------------------------------------------------
//   Set the fill color as defined by the current layout attributes
// ----------------------------------------------------------------------------
{
    return Shape::setFillColor(currentLayout);
}


bool ModuleRenderer::SetLineColor()
// ----------------------------------------------------------------------------
//   Set the the outline color as defined by the current layout attributes
// ----------------------------------------------------------------------------
{
    return Shape::setLineColor(currentLayout);
}


Matrix4 ModuleRenderer::ModelMatrix()
// ----------------------------------------------------------------------------
//   Module interface for currentModelMatrix
// ----------------------------------------------------------------------------
{
    return Widget::Tao()->layout->model;
}


void ModuleRenderer::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Draw stuff in layout by calling previously registered render callback
// ----------------------------------------------------------------------------
{
    record(module_renderer, "Draw [%p] in layout %p", this, where);

    // Synchronise GL states
    GL.Sync();

    try
    {
        currentLayout = where;
        callback(arg);
    }
    catch(...)
    {
        record(module_renderer_error,
               "Exception raised from module renderer %p drawing with %p",
               this, callback);
    }

    GL.Invalidate();
}


void ModuleRenderer::Identify(Layout *where)
// ----------------------------------------------------------------------------
//   Identify object under cursor
// ----------------------------------------------------------------------------
{
    // Don't even try to call module call back if in mouse move, see #2563
    Widget *widget = where->Display();
    if (!widget->inMouseMove())
    {
        XL::Save<bool> inIdentify(where->inIdentify, true);
        GL.UseProgram(0); // Necessary for #1464
        currentLayout = where;

        // Synchronise GL states
        GL.Sync();

        try
        {
            if(identify)
                identify(arg);
            else
                callback(arg);
        }
        catch(...)
        {
            record(module_renderer_error,
                   "Exception raised from module renderer %p identify %p",
                   this, identify);
        }
    }

    GL.Invalidate();
}

} // namespace Tao
