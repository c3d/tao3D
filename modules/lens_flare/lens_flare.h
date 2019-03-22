#ifndef LENS_FLARE_H
#define LENS_FLARE_H
// *****************************************************************************
// lens_flare.h                                                    Tao3D project
// *****************************************************************************
//
// File description:
//
//    Prototype of the function used by lens_flare.tbl to
//    implement the new XL primitive
//
//
//
//
//
//
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2011, Soulisse Baptiste <soulisse@polytech.unice.fr>
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
