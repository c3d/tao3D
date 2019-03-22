#ifndef TAO_MODULE_RENDERER_H
#define TAO_MODULE_RENDERER_H
// *****************************************************************************
// module_renderer.h                                               Tao3D project
// *****************************************************************************
//
// File description:
//
//    Enables a module to render OpenGL graphics in a Tao document
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
// (C) 2010-2011,2013-2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2012, Jérôme Forissier <jerome@taodyne.com>
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
        : Drawing(), callback(callback), identify(NULL), arg(arg), del(NULL) {}
    ModuleRenderer(ModuleApi::render_fn callback, void * arg,
                   ModuleApi::delete_fn del)
        : Drawing(), callback(callback), identify(NULL), arg(arg), del(del) {}
    ModuleRenderer(ModuleApi::render_fn callback,
                   ModuleApi::render_fn identify, void * arg,
                   ModuleApi::delete_fn del)
        : Drawing(), callback(callback), identify(identify), arg(arg), del(del){}

    virtual ~ModuleRenderer();

    // Drawing interface
    virtual void  Draw(Layout *where);
    virtual void  Identify(Layout *where);

    // Exported to ModuleApi
    static bool   ScheduleRender(ModuleApi::render_fn callback, void *arg);
    static bool   AddToLayout(ModuleApi::render_fn callback, void *arg,
                              ModuleApi::delete_fn del);
    static bool   AddToLayout2(ModuleApi::render_fn callback,
                               ModuleApi::render_fn identify,
                               void *arg,
                               ModuleApi::delete_fn del);


    // Set drawing attributes
    static bool     SetTextures();
    static bool     SetShader(int id);

    static bool     SetFillColor();
    static bool     SetLineColor();

    static Matrix4  ModelMatrix();

private:
    ModuleApi::render_fn   callback, identify;
    void *                 arg;
    ModuleApi::delete_fn   del;
    static Layout* currentLayout;
};

}

#endif // TAO_MODULE_RENDERER_H
