#ifndef LIGHTING_H
#define LIGHTING_H
// *****************************************************************************
// lighting.h                                                      Tao3D project
// *****************************************************************************
//
// File description:
//
//     Lighting attributes
//
//
//
//
//
//
//
//
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2011,2013, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
// (C) 2010, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010-2011,2013-2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2011,2014, Jérôme Forissier <jerome@taodyne.com>
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

#include "drawing.h"
#include "color.h"
#include "attributes.h"
#include "tao_gl.h"
#include "info_trash_can.h"
#include <vector>
#include <QGLShaderProgram>

TAO_BEGIN

struct Lighting : Attribute
// ----------------------------------------------------------------------------
//   Base class for all lighting attributes
// ----------------------------------------------------------------------------
{
    Lighting(): Attribute() {}
};


struct ShaderProgramInfo : XL::Info, InfoTrashCan
// ----------------------------------------------------------------------------
//   Hold info associated to a tree
// ----------------------------------------------------------------------------
{
    ShaderProgramInfo(QGLShaderProgram *program)
        : program(program), inError(false) {}
    ~ShaderProgramInfo() { delete program; }
    virtual void Delete() { trash.push_back(this); }
    typedef QGLShaderProgram *data_t;
    operator data_t() { return program; }
    enum { SHADER_TYPES = 3 };

    QGLShaderProgram *program;
    text              shaderSource[SHADER_TYPES];
    bool              inError;
};


struct ShaderProgram : Lighting
// ----------------------------------------------------------------------------
//   Activate a shader program
// ----------------------------------------------------------------------------
{
    ShaderProgram(QGLShaderProgram *program = NULL): program(program) {}
    virtual void Draw(Layout *where);
    virtual void Evaluate(Layout *where) { Draw(where); }
    QGLShaderProgram *program;
};


struct ShaderValue : Lighting
// ----------------------------------------------------------------------------
//   Set a shader uniform or attribute value
// ----------------------------------------------------------------------------
{
    typedef std::vector<float> Values;
    ShaderValue(GLuint location, GLuint type, Values values)
        : location(location), type(type), values(values) {}
    virtual void Draw(Layout *where);
    GLuint location;
    GLuint type;
    Values values;
};


struct ShaderAttribute : ShaderValue
// ----------------------------------------------------------------------------
//   Set a shader attribute
// ----------------------------------------------------------------------------
{
    ShaderAttribute(GLuint location, GLuint type, Values values)
        : ShaderValue(location, type, values) {}
    virtual void Draw(Layout *where);
};


struct PerPixelLighting : Lighting
// ----------------------------------------------------------------------------
//   Enable or disable per pixel lighting
// ----------------------------------------------------------------------------
{
    PerPixelLighting(bool enable = true);
    ~PerPixelLighting();
    virtual void Draw(Layout *where);
    static uint PerPixelLightingShader() { return pgm ? pgm->programId() : 0; }

    bool enable;
    ShaderProgram* shader;

    static QGLShaderProgram* pgm;
    static bool failed;
};


struct LightId : Lighting
// ----------------------------------------------------------------------------
//   Select which light we are using
// ----------------------------------------------------------------------------
{
    LightId(uint id, bool enable);
    ~LightId();
    virtual void Draw(Layout *where);
    virtual void Evaluate(Layout *where) { Draw(where); }
    uint id;
    bool enable;
    PerPixelLighting* perPixelLighting;
};


struct Light : Lighting
// ----------------------------------------------------------------------------
//   Light attribute (glLight)
// ----------------------------------------------------------------------------
{
    typedef std::vector<GLfloat> Arguments;
    Light(GLenum f, Arguments &a): Lighting(), function(f), args(a) {}
    Light(GLenum f, GLfloat a): Lighting(), function(f), args()
    {
        args.push_back(a);
    }
    Light(GLenum f, GLfloat x, GLfloat y, GLfloat z):
        Lighting(), function(f), args()
    {
        args.push_back(x);
        args.push_back(y);
        args.push_back(z);
    }
    Light(GLenum f, GLfloat r, GLfloat g, GLfloat b, GLfloat a):
        Lighting(), function(f), args()
    {
        args.push_back(r);
        args.push_back(g);
        args.push_back(b);
        args.push_back(a);
    }
    virtual void Draw(Layout *where);
    GLenum      function;
    Arguments   args;
};


struct Material : Light
// ----------------------------------------------------------------------------
//   Material attribute (glMaterial)
// ----------------------------------------------------------------------------
{
    Material(GLenum fc, GLenum f, GLfloat a): Light(f, a), face(fc) {}
    Material(GLenum fc, GLenum f, GLfloat a, GLfloat b, GLfloat c, GLfloat d)
        : Light(f, a, b, c, d), face(fc) {}
    virtual void Draw(Layout *where);
    GLenum      face;
};



// ============================================================================
//
//   Entering lighting primitives in the symbols table
//
// ============================================================================

extern void EnterLighting();
extern void DeleteLighting();

TAO_END

#endif // LIGHTING_H
