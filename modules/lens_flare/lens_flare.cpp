// *****************************************************************************
// lens_flare.cpp                                                  Tao3D project
// *****************************************************************************
//
// File description:
//
//     Implementation of the XL primitives for the "lens flare" module.
//
//
//
//
//
//
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2011,2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2011, Jérôme Forissier <jerome@taodyne.com>
// (C) 2011, Soulisse Baptiste <baptiste.soulisse@taodyne.com>
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
#include "lens_flare.h"
#include "main.h"
#include "runtime.h"
#include "base.h"

XL_DEFINE_TRACES

LensFlare* lens;



Tree_p lens_flare(Context *context, Tree_p, Tree_p prog)
// ----------------------------------------------------------------------------
//    Generate a lens flare
// ----------------------------------------------------------------------------
{
    lens = new LensFlare();
    context->Evaluate(prog);
    LensFlare::tao->addToLayout(LensFlare::render_callback, lens,
                                LensFlare::delete_callback);

    return xl_true;
}

Tree_p lens_flare_target(Tree_p tree, Real_p tx, Real_p ty, Real_p tz)
// ----------------------------------------------------------------------------
//    Define the lens flare target
// ----------------------------------------------------------------------------
{
    if(!lens)
    {
        Ooops("LensFlare: No lens flare for target in $1 ", tree);
        return xl_false;
    }
    lens->setTarget(Vector3(tx, ty, tz));

    return xl_true;
}


Tree_p lens_flare_source(Tree_p tree, Real_p sx, Real_p sy, Real_p sz)
// ----------------------------------------------------------------------------
//    Define the lens flare source
// ----------------------------------------------------------------------------
{
    if(!lens)
    {
        Ooops("LensFlare: No lens flare for source in $1 ", tree);
        return xl_false;
    }
    lens->setSource(Vector3(sx, sy, sz));

    return xl_true;
}

Tree_p add_flare(Tree_p tree, GLuint id, Real_p loc, Real_p scale, double r,
                 double g, double b, double a)
// ----------------------------------------------------------------------------
//    Add flares to the current lens flare
// ----------------------------------------------------------------------------
{
    if(!lens)
    {
        Ooops("LensFlare: No lens flare while adding flare in $1 ", tree);
        return xl_false;
    }
    lens->addFlare(id, loc, scale, r, g, b, a);

    return xl_true;
}

Tree_p lens_flare_depth_test(Tree_p self, Name_p name)
// ----------------------------------------------------------------------------
//    Enable or disable depth test on the lens flare
// ----------------------------------------------------------------------------
{
    if(!lens)
    {
        Ooops("LensFlare: No lens flare for depth test in $1", self);
        return xl_false;
    }

    bool enable = false;
    if (name == xl_false || name->value == "OFF")
    {
        enable = false;
    }
    else if (name == XL::xl_true || name->value == "ON")
    {
        enable = true;
    }
    else
    {
        Ooops("LensFlare: Invalid name '$2' in '$1'", self, name);
        return xl_false;
    }
    lens->enableDephTest(enable);

    return xl_true;
}

int module_init(const Tao::ModuleApi *api, const Tao::ModuleInfo *)
// ----------------------------------------------------------------------------
//   Initialize the Tao module
// ----------------------------------------------------------------------------
{
    XL_INIT_TRACES();
    LensFlare::tao = api;
    return 0;
}


int module_exit()
// ----------------------------------------------------------------------------
//   Uninitialize the Tao module
// ----------------------------------------------------------------------------
{
    return 0;
}
