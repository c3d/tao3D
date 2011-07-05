#ifndef TAO_MODULE_RENDERER_H
#define TAO_MODULE_RENDERER_H
// ****************************************************************************
//  module_renderer.h                                              Tao project
// ****************************************************************************
//
//   File Description:
//
//    Enables a module to render OpenGL graphics in a Tao document
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
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "tao/module_api.h"
#include "drawing.h"

namespace Tao
{

struct Widget;

struct ModuleRenderer : Drawing
// ------------------------------------------------------------------------
//   A special kind of drawing that invokes a callback to do the rendering
// ------------------------------------------------------------------------
{
    ModuleRenderer(ModuleApi::render_fn callback, void * arg)
        : Drawing(), callback(callback), arg(arg), del(NULL) {}
    ModuleRenderer(ModuleApi::render_fn callback, void * arg,
                   ModuleApi::delete_fn del)
        : Drawing(), callback(callback), arg(arg), del(del) {}
    virtual ~ModuleRenderer();

    virtual text  getType() { return "ModuleRenderer";}
    // Drawing interface
    virtual void  Draw(Layout *where);

    // Exported to ModuleApi
    static bool   ScheduleRender(ModuleApi::render_fn callback, void *arg);
    static bool   AddToLayout(ModuleApi::render_fn callback, void *arg,
                              ModuleApi::delete_fn del);

private:
    ModuleApi::render_fn  callback;
    void *                arg;
    ModuleApi::delete_fn  del;
};

}

#endif // TAO_MODULE_RENDERER_H
