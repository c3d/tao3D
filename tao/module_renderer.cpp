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

std::map<int, double*> ModuleRenderer::texList;
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

bool ModuleRenderer::SetTexCoords(int unit, double* texCoord)
// ----------------------------------------------------------------------------
//   Set the texture coordinates in the ModuleRenderer according
//   to the current layout attributes
// ----------------------------------------------------------------------------
{
    int maxTexCoord = TaoApp->maxTextureCoords;
    if(unit >= -1 && unit < maxTexCoord)
    {
        if(unit == -1)
            for(int i = 0; i < maxTexCoord; i++)
                texList[i] = texCoord;
        else
            texList[unit] = texCoord;

        return true;
    }

    return false;
}

bool ModuleRenderer::SetTexture(unsigned int id, unsigned int type)
// ----------------------------------------------------------------------------
//   Set the texture in the ModuleRenderer according
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
    for(uint i = 0; i < texList.size(); i++)
        if(texList[i] && where->fillTextures[i].id)
            Shape::enableTexCoord(i, texList[i]);

    currentLayout  = where;
    Shape::setTexture(where);

    callback(arg);

    for(uint i = 0; i < texList.size(); i++)
        if(texList[i] && where->fillTextures[i].id)
            Shape::disableTexCoord(i);
}

}
