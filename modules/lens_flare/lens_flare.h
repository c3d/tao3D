#ifndef LENS_FLARE_H
#define LENS_FLARE_H
// ****************************************************************************
//  lens_flare.h                                                    Tao project
// ****************************************************************************
//
//   File Description:
//
//    Prototype of the function used by lens_flare.tbl to
//    implement the new XL primitive
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

#include "flare.h"
#include "tree.h"
#include "tao/module_api.h"

using namespace XL;

Tree_p lens_flare(Context *context, Tree_p tree, Tree_p prog);
Tree_p lens_flare_target(Tree_p tree, Real_p tx, Real_p ty, Real_p tz);
Tree_p lens_flare_source(Tree_p tree, Real_p sx, Real_p sy, Real_p sz);
Tree_p add_flare(Tree_p tree, GLuint id, Real_p loc, Real_p scale, double r, double g, double b, double a);
Tree_p lens_flare_depth_test(Tree_p tree, Name_p name);

#endif // LENS_FLARE_H
