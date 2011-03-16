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

namespace Tao {


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


void ModuleRenderer::Draw(Layout */*where*/)
// ----------------------------------------------------------------------------
//   Draw stuff in layout by calling previously registered render callback
// ----------------------------------------------------------------------------
{
    callback(arg);
}

}
