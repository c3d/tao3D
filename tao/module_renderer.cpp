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
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************


#include "tao/module_api.h"
#include "module_renderer.h"
#include "widget.h"
#include "shapes.h"
#include "application.h"
#include "attributes.h"

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
    Widget::Tao()->layout->Add(new ModuleRenderer(callback, identify, arg, del));
    return true;
}



bool ModuleRenderer::EnableTexCoords(double* texCoords, uint64 mask)
// ----------------------------------------------------------------------------
//   Enable specified coordinates for active textures in the current layout
// ----------------------------------------------------------------------------
{
    Shape::enableTexCoord(texCoords, mask);
    return true;
}


bool ModuleRenderer::DisableTexCoords(uint64 mask)
// ----------------------------------------------------------------------------
//   Disable coordinates for active textures in the current layout
// ----------------------------------------------------------------------------
{
    Shape::disableTexCoord(mask);
    return true;
}


uint ModuleRenderer::TextureUnit()
// ----------------------------------------------------------------------------
//  Return last activated texture unit
// ----------------------------------------------------------------------------
{
    return GL.ActiveTextureUnitIndex() - GL_TEXTURE0;
}


uint64 ModuleRenderer::TextureUnits()
// ----------------------------------------------------------------------------
//  Return bitmask of all current activated texture units
// ----------------------------------------------------------------------------
{
    return GL.ActiveTextureUnits();
}


void ModuleRenderer::SetTextureUnits(uint64 mask)
// ----------------------------------------------------------------------------
//  Set bitmask of current activated texture units
// ----------------------------------------------------------------------------
{
    GL.ActivateTextureUnits(mask);
}


bool ModuleRenderer::SetTextures()
// ----------------------------------------------------------------------------
//   Apply the textures as defined by current layout attributes
// ----------------------------------------------------------------------------
{
    return Shape::setTexture(currentLayout);
}


bool ModuleRenderer::BindTexture(unsigned int id, unsigned int type)
// ----------------------------------------------------------------------------
//   Bind the texture as defined by current layout attributes
// ----------------------------------------------------------------------------
{
    Layout *layout = Widget::Tao()->layout;
    layout->Add(new FillTexture(id, type));
    layout->hasAttributes = true;
    GL.BindTexture(type, id);
    return false;
}


bool ModuleRenderer::HasTexture(uint texUnit)
// ----------------------------------------------------------------------------
//  Check if a texture is bound at the specified unit
// ----------------------------------------------------------------------------
{
    return (GL.ActiveTextureUnits() & (1ULL << texUnit)) ? true : false;
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


void ModuleRenderer::BindTexture2D(unsigned int id,
                                   unsigned int width,
                                   unsigned int height)
// ----------------------------------------------------------------------------
//   Add 2D texture to current layout
// ----------------------------------------------------------------------------
{
    GL.BindTexture(GL_TEXTURE_2D, id);
    GL.TextureSize(width, height);
    if (Layout *layout = Widget::Tao()->layout)
    {
        layout->Add(new FillTexture(id, GL_TEXTURE_2D));
        layout->hasAttributes = true;
    }
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


bool ModuleRenderer::HasPixelBlur(bool enable)
// ----------------------------------------------------------------------------
//   Enable or deactivate pixel blur.
// ----------------------------------------------------------------------------
{
    currentLayout->hasPixelBlur = enable;
    return true;
}

uint ModuleRenderer::EnabledLights()
// ----------------------------------------------------------------------------
//  Return a bitmask of all current enabled lights
// ----------------------------------------------------------------------------
{
    return currentLayout->currentLights;
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
    // Load matrix before draw
    GL.LoadMatrix();

    currentLayout = where;
    callback(arg);
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
        // Load matrix before identify
        GL.LoadMatrix();
        
        XL::Save<bool> inIdentify(where->inIdentify, true);
        GL.UseProgram(0); // Necessary for #1464
        currentLayout = where;
        
        if(identify)
            identify(arg);
        else
            callback(arg);
    }
}

}
