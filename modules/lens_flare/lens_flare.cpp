// ****************************************************************************
//  lens_flare.h                                                    Tao project
// ****************************************************************************
//
//   File Description:
//
//     Implementation of the XL primitives for the "lens flare" module.
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
    LensFlare::tao->scheduleRender(LensFlare::render_callback, lens);

    return xl_true;
}

Tree_p lens_flare_target(Tree_p tree, Real_p tx, Real_p ty, Real_p tz)
// ----------------------------------------------------------------------------
//    Define the lens flare target
// ----------------------------------------------------------------------------
{
    if(! lens)
    {
        Ooops("No lens flare defined '$1' ", tree);
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
    if(! lens)
    {
        Ooops("No lens flare defined '$1' ", tree);
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
    if(! lens)
    {
        Ooops("No lens flare defined '$1' ", tree);
        return xl_false;
    }
    lens->addFlare(id, loc, scale, r, g, b, a);

    return xl_true;
}

Tree_p lens_flare_depth_test(Tree_p tree, Name_p name)
// ----------------------------------------------------------------------------
//    Enable or disable depth test on the lens flare
// ----------------------------------------------------------------------------
{
    if(! lens)
    {
        Ooops("No lens flare defined '$1' ", tree);
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
