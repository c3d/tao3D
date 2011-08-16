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

bool ModuleRenderer::BindTexCoords(double* texCoord)
// ----------------------------------------------------------------------------
//   Bind the texture coordinates in the ModuleRenderer according
//   to the current layout attributes
// ----------------------------------------------------------------------------
{
    if(! texCoord)
        return false;

    std::map<uint, TextureState>::iterator it;
    for(it = currentLayout->fillTextures.begin(); it != currentLayout->fillTextures.end(); it++)
        if(((*it).second).id)
            Shape::enableTexCoord((*it).first, texCoord);

    return true;
}

bool ModuleRenderer::UnBindTexCoords()
// ----------------------------------------------------------------------------
//   Unbind the texture coordinates in the ModuleRenderer according
//   to the current layout attributes
// ----------------------------------------------------------------------------
{
    std::map<uint, TextureState>::iterator it;
    for(it = currentLayout->fillTextures.begin(); it != currentLayout->fillTextures.end(); it++)
        if(((*it).second).id)
            Shape::disableTexCoord((*it).first);

    return true;
}

bool ModuleRenderer::SetTextures()
// ----------------------------------------------------------------------------
//   Apply the textures in the ModuleRenderer according
//   to the current layout attributes
// ----------------------------------------------------------------------------
{
    return Shape::setTexture(currentLayout);
}


bool ModuleRenderer::BindTexture(unsigned int id, unsigned int type)
// ----------------------------------------------------------------------------
//   Bind the texture in the ModuleRenderer according
//   to the current layout attributes
// ----------------------------------------------------------------------------
{
    GLuint unit = Widget::Tao()->layout->currentTexture.unit;

    Widget::Tao()->layout->currentTexture.id = id;
    Widget::Tao()->layout->currentTexture.type = type;

    Widget::Tao()->layout->Add(new FillTexture(id, unit, type));
    return false;
}

bool ModuleRenderer::SetFillColor()
// ----------------------------------------------------------------------------
//   Set the fill color in the ModuleRenderer according
//   to the current layout attributes
// ----------------------------------------------------------------------------
{
    return Shape::setFillColor(currentLayout);
}

bool ModuleRenderer::SetLineColor()
// ----------------------------------------------------------------------------
//   Set the the outline color in the ModuleRenderer according
//   to the current layout attributes
// ----------------------------------------------------------------------------
{
    return Shape::setLineColor(currentLayout);
}

void ModuleRenderer::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Draw stuff in layout by calling previously registered render callback
// ----------------------------------------------------------------------------
{
    currentLayout  = where;

    callback(arg);
}

}
