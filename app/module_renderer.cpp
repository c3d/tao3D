// ****************************************************************************
//  module_renderer.cpp                                            Tao project
// ****************************************************************************
//
//   File Description:
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
// ****************************************************************************
// This software is licensed under the GNU General Public License v3.
// See file COPYING for details.
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************


#include "tao/module_api.h"
#include "module_renderer.h"
#include "widget.h"
#include "shapes.h"
#include "application.h"
#include "attributes.h"
#include "flight_recorder.h"

namespace Tao {

Layout* ModuleRenderer::currentLayout = NULL;

ModuleRenderer::~ModuleRenderer()
// ----------------------------------------------------------------------------
//   Destructor
// ----------------------------------------------------------------------------
{
    if (del)
        del(arg);
}


bool ModuleRenderer::ScheduleRender(ModuleApi::render_fn callback, void *arg)
// ----------------------------------------------------------------------------
//   Create a ModuleRendererPrivate object attached to current layout
// ----------------------------------------------------------------------------
{
    Widget::Tao()->layout->Add(new ModuleRenderer(callback, arg));
    return true;
}


bool ModuleRenderer::AddToLayout(ModuleApi::render_fn callback, void *arg,
                                 ModuleApi::delete_fn del)
// ----------------------------------------------------------------------------
//   Create a ModuleRendererPrivate object attached to current layout
// ----------------------------------------------------------------------------
{
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
    // Synchronise GL states
    GL.Sync();

    try
    {
        currentLayout = where;
        callback(arg);
    }
    catch(...)
    {
        RECORD(ALWAYS, "Exception raised from module renderer",
               "call", (intptr_t) callback);
        std::cerr << "Exception raised from module renderer callback "
                  << (intptr_t) callback << "\n";
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
            RECORD(ALWAYS, "Exception raised from module identify",
                   "call", (intptr_t) callback,
                   "id", (intptr_t) identify);
            std::cerr << "Exception raised from module renderer callback "
                      << (intptr_t) callback << " "
                      << (intptr_t) identify << "\n";
        }
    }

    GL.Invalidate();
}

} // namespace Tao
