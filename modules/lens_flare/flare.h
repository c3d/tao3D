#ifndef FLARE_H
#define FLARE_H
// *****************************************************************************
// flare.h                                                         Tao3D project
// *****************************************************************************
//
// File description:
//
//      Draw a lens flare at a specific location and heading toward a
//      defined target.
//
//
//
//
//
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2011,2013, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
// (C) 2011,2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2011,2013, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can redistribute it and/or modify
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


#include "tao/coords3d.h"
#include "tao/module_api.h"
#include "tao/tao_gl.h"
#include "tao/graphic_state.h"
#include <QObject>
#include <vector>

using namespace std;
using namespace Tao;

struct Flare
// ----------------------------------------------------------------------------
//   Representation of a single flare
// ----------------------------------------------------------------------------
{
  GLuint id;            // Flare texture index
  float scale;          // Scaling rate
  float loc;            // Position on axis
  float color[4];       // Mixing color
};

struct LensFlare : public QObject
// ----------------------------------------------------------------------------
//   Define a complete lens flare
// ----------------------------------------------------------------------------
{
    // Constructor and destructor
    LensFlare();
    ~LensFlare();

    // Specify lens flare parameters
    void            setTarget(Vector3 target);
    void            setSource(Vector3 source);
    void            enableDephTest(bool enable);

    // Add a flare to the list
    void            addFlare(GLuint id, float location, float scale, GLfloat r, GLfloat g, GLfloat b, GLfloat a);

    // Draw lens flare
    void            Draw();

    static void     render_callback(void *arg);
    static void     delete_callback(void *arg);

private:
    // Draw single flare
    void            DrawFlare(Flare flare, Vector3 position);
    // Depth test for the lens flare
    bool            isOccluded(Vector3 p);

public:
    // Pointer to Tao functions
    static const Tao::ModuleApi *tao;

private:
    GLuint query;
    // List of attached flares
    vector<Flare> lens_flare;
    // Depth test status
    bool          depth_test;
    float         test_size;
    // Define source and target of the lens flare
    Vector3       target, source;
};

#endif
