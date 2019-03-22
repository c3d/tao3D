// *****************************************************************************
// lighting.cpp                                                    Tao3D project
// *****************************************************************************
//
// File description:
//
//
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
// (C) 2011-2013, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
// (C) 2010-2011, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010-2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2012,2014, Jérôme Forissier <jerome@taodyne.com>
// (C) 2010-2011, Lionel Schaffhauser <lionel@taodyne.com>
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

#include "lighting.h"
#include "layout.h"
#include "tao_gl.h"
#include "application.h"
#include "widget.h"
#include "tao_utf8.h"

TAO_BEGIN

QGLShaderProgram *PerPixelLighting::pgm = NULL;
bool		  PerPixelLighting::failed = false;

PerPixelLighting::PerPixelLighting(bool enable) : enable(enable), shader(NULL)
// ----------------------------------------------------------------------------
//   Construction
// ----------------------------------------------------------------------------
{
    if(!pgm && !failed)
    {
        // Use per pixel lighting only if GL_EXT_gpu_shader4 is supported
        if(Widget::isGLExtensionAvailable("GL_EXT_gpu_shader4"))
        {
            pgm = new QGLShaderProgram();
            QString path = Application::applicationDirPath();
            QString vs = path + "/lighting.vs";
            QString fs = path + "/lighting.fs";

            bool ok = false;
            if (pgm->addShaderFromSourceFile(QGLShader::Vertex, vs))
            {
                if (pgm->addShaderFromSourceFile(QGLShader::Fragment, fs))
                {
                    ok = true;
                }
                else
                {
                    std::cerr << "Error loading shader code: " << +fs << "\n";
                    std::cerr << +pgm->log();
                }
            }
            else
            {
                std::cerr << "Error loading shader code: " << +vs << "\n";
                std::cerr << +pgm->log();
            }
            if (!ok)
            {
                delete pgm;
                pgm = NULL;
                failed = true;
            }
            else
            {
                pgm->link();
            }
        }
        else
        {
            std::cerr << "Per-pixel lighting disabled due to "
                         "missing extension: GL_EXT_gpu_shader4\n";
            failed = true;
        }
    }

}


PerPixelLighting::~PerPixelLighting()
// ----------------------------------------------------------------------------
//   Destruction
// ----------------------------------------------------------------------------
{
    if (shader)
        delete shader;
}


void PerPixelLighting::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Enable or disable per pixel lighting
// ----------------------------------------------------------------------------
{
    where->perPixelLighting = enable;
}


LightId::LightId(uint id, bool enable) : Lighting(), id(id), enable(enable)
// ----------------------------------------------------------------------------
//   Construction
// ----------------------------------------------------------------------------
{
    perPixelLighting = new PerPixelLighting();
}


LightId::~LightId()
// ----------------------------------------------------------------------------
//   Destruction
// ----------------------------------------------------------------------------
{
    delete perPixelLighting;
}


void LightId::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Select and enable or disable a light
// ----------------------------------------------------------------------------
{
    where->lightId = GL_LIGHT0 + id;
    if (enable)
    {
        GL.Enable(where->lightId);
        GL.Enable(GL_LIGHTING);
        GL.LightModel(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    }
    else
    {
        GL.Disable(where->lightId);
        if(! GL.LightsMask())
        {
            GL.Disable(GL_LIGHTING);
            GL.Disable(GL_COLOR_MATERIAL);
        }
    }
}


void Light::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Send the corresponding GL attribute
// ----------------------------------------------------------------------------
{
    GL.Light(where->lightId, function, &args[0]);
}


void Material::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Send the corresponding GL material attribute
// ----------------------------------------------------------------------------
{
    GL.Disable(GL_COLOR_MATERIAL);
    GL.Materialfv(face, function, &args[0]);

    // Determine is the diffuse material
    // is visible or not (use for transparency)
    if(function == GL_DIFFUSE)
        where->visibility = args[args.size() - 1];
}


void ShaderProgram::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Activate the given shader program
// ----------------------------------------------------------------------------
{
    if (!where->InIdentify())
    {
        if(program)
        {
            program->bind();
            where->programId = program->programId();
            where->blendOrShade = true;
        }
        else
        {
            where->programId = 0;
        }
        GL.UseProgram(where->programId);
    }
}


void ShaderValue::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Set the shader value
// ----------------------------------------------------------------------------
{
    if (where->programId && !where->InIdentify())
    {
        GLint id = location;
        int sz = values.size();

        switch (type)
        {
        case GL_BOOL:
        case GL_INT:
        case GL_SAMPLER_1D:
        case GL_SAMPLER_2D:
        case GL_SAMPLER_3D:
        case GL_SAMPLER_CUBE:
#ifdef GL_SAMPLER_2D_RECT
        case GL_SAMPLER_2D_RECT:
#endif
            if (sz == 1)
            {
                GL.Uniform(id, (int) values[0]);
            }
            else
            {
                std::vector<GLint> ivalues;
                for (int i = 0; i < sz; i++)
                    ivalues.push_back((int) values[i]);
                GL.Uniform1iv(id, sz, &ivalues[0]);
            }
            break;
        case GL_FLOAT:
            GL.Uniform1fv(id, sz, &values[0]);
            break;
        case GL_FLOAT_VEC2:
            GL.Uniform2fv(id, sz/2, &values[0]);
            break;
        case GL_FLOAT_VEC3:
            GL.Uniform3fv(id, sz/3, &values[0]);
            break;
        case GL_FLOAT_VEC4:
            GL.Uniform4fv(id, sz/4, &values[0]);
            break;
        case GL_FLOAT_MAT2:
            GL.UniformMatrix2fv(id, sz/4, 0, &values[0]);
            break;
        case GL_FLOAT_MAT3:
            GL.UniformMatrix3fv(id, sz/9, 0, &values[0]);
            break;
        case GL_FLOAT_MAT4:
            GL.UniformMatrix4fv(id, sz/16, 0, &values[0]);
            break;
        default:
            std::cerr << "Unsupported uniform type: " << type << "\n";
            break;
        }
    }
}


void ShaderAttribute::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Set the shader attribute
// ----------------------------------------------------------------------------
{
    if (where->programId && !where->InIdentify())
    {
        switch(values.size())
        {
        case 1: GL.VertexAttrib1fv(location, &values[0]); break;
        case 2: GL.VertexAttrib2fv(location, &values[0]); break;
        case 3: GL.VertexAttrib3fv(location, &values[0]); break;
        case 4: GL.VertexAttrib4fv(location, &values[0]); break;
        }
    }
}

TAO_END



// ****************************************************************************
//
//    Code generation from shapes.tbl
//
// ****************************************************************************

#include "graphics.h"
#include "opcodes.h"
#include "options.h"
#include "widget.h"
#include "types.h"
#include "drawing.h"
#include "layout.h"
#include "module_manager.h"
#include <iostream>


// ============================================================================
//
//    Top-level operation
//
// ============================================================================

#include "widget.h"

using namespace XL;

#include "opcodes_declare.h"
#include "lighting.tbl"

namespace Tao
{

#include "lighting.tbl"


void EnterLighting()
// ----------------------------------------------------------------------------
//   Enter all the basic operations defined in attributes.tbl
// ----------------------------------------------------------------------------
{
    XL::Context *context = MAIN->context;
#include "opcodes_define.h"
#include "lighting.tbl"
}


void DeleteLighting()
// ----------------------------------------------------------------------------
//   Delete all the global operations defined in attributes.tbl
// ----------------------------------------------------------------------------
{
#include "opcodes_delete.h"
#include "lighting.tbl"
}

}
