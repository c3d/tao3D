// ****************************************************************************
//  lighting.cpp						    Tao project
// ****************************************************************************
//
//   File Description:
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
// ****************************************************************************
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Jérôme Forissier <jerome@taodyne.com>
//  (C) 2010 Catherine Burvelle <cathy@taodyne.com>
//  (C) 2010 Lionel Schaffhauser <lionel@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

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
        }
    }
    if (pgm)
	shader = new ShaderProgram(pgm);
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
    if(shader)
    {
	if(enable)
	{
	    shader->Draw(where);
	    where->perPixelLighting = where->programId;
	}
	else
	{
	    // If there is no other shaders, then deactivate it
	    if(where->perPixelLighting == where->programId)
		where->programId = 0;

	    where->perPixelLighting = 0;
	}

    }
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
	where->currentLights |= 1 << id;
	glEnable(where->lightId);
	glEnable(GL_LIGHTING);
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);

	if(TaoApp->useShaderLighting)
	    perPixelLighting->Draw(where);
    }
    else
    {
	where->currentLights ^= 1 << id;
	glDisable(where->lightId);
	if(! where->currentLights)
	{
	    glDisable(GL_LIGHTING);
	    glDisable(GL_COLOR_MATERIAL);
	}
    }
}


void Light::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Send the corresponding GL attribute
// ----------------------------------------------------------------------------
{
    glLightfv(where->lightId, function, &args[0]);
}


void Material::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Send the corresponding GL material attribute
// ----------------------------------------------------------------------------
{
    where->hasMaterial = true;
    glDisable(GL_COLOR_MATERIAL);
    glMaterialfv(face, function, &args[0]);

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
	program->bind();
	where->programId = program->programId();
    }
}


void ShaderValue::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Set the shader value
// ----------------------------------------------------------------------------
{
    if (where->programId && !where->InIdentify())
    {
	ShaderUniformInfo   *uniform   = name->GetInfo<ShaderUniformInfo>();
	ShaderAttributeInfo *attribute = name->GetInfo<ShaderAttributeInfo>();
	if (!uniform && !attribute)
	{
	    kstring cname = name->value.c_str();
	    GLint uni = glGetUniformLocation(where->programId, cname);
	    if (uni >= 0)
	    {
		uniform = new ShaderUniformInfo(uni);
		name->SetInfo<ShaderUniformInfo>(uniform);
	    }
	    else
	    {
		GLint attri = glGetAttribLocation(where->programId, cname);
		if (attri >= 0)
		{
		    attribute = new ShaderAttributeInfo(attri);
		    name->SetInfo<ShaderAttributeInfo>(attribute);
		}
	    }
	}


	if (uniform)
	{
	    uint id = uniform->id;
	    GLint type = 0;

	    GLint uniformMaxLength = 0;
	    glGetProgramiv(where->programId,
			   GL_ACTIVE_UNIFORM_MAX_LENGTH, &uniformMaxLength);

	    GLint uniformActive = 0;
	    glGetProgramiv(where->programId,
			   GL_ACTIVE_UNIFORMS, &uniformActive);

	    //Get type of current uniform variable
	    GLint size = 0;
			GLint length = 0;
	    GLchar* uniformName = new GLchar[uniformMaxLength + 1];
	    for(int index = 0; index < uniformActive; index++)
	    {
		glGetActiveUniform (where->programId,
				    index, uniformMaxLength + 1,
				    &length, &size, (GLenum*) &type,
				    uniformName);

		// If uniform is an array,
		// compare just name without []
		if(length >= 3 && uniformName[length - 1] == ']')
		    if(! strncmp(uniformName,name->value.c_str(), length - 3))
			break;

		// Otherwise juste compare
		if(! strcmp(uniformName,name->value.c_str()))
		    break;
            }
            delete[] uniformName;

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
                glUniform1i(id, values[0]);
                break;
            case GL_FLOAT_VEC2:
                glUniform2fv(id, (int) (values.size() / 2), &values[0]);
                break;
            case GL_FLOAT_VEC3:
                glUniform3fv(id, (int) (values.size() / 3), &values[0]);
                break;
            case GL_FLOAT_VEC4:
                glUniform4fv(id, (int) (values.size() / 4), &values[0]);
                break;
            case GL_FLOAT_MAT2:
                glUniformMatrix2fv(id, (int) (values.size()/4), 0, &values[0]);
                break;
            case GL_FLOAT_MAT3:
                glUniformMatrix3fv(id, (int) (values.size()/9), 0, &values[0]);
                break;
            case GL_FLOAT_MAT4:
                glUniformMatrix4fv(id, (int) (values.size()/16), 0, &values[0]);
                break;
            default:
                glUniform1fv(id, values.size(), &values[0]);
                break;
            }
        }
        else if (attribute)
        {
            switch(values.size())
            {
            case 1: glVertexAttrib1fv(uniform->id, &values[0]); break;
            case 2: glVertexAttrib2fv(uniform->id, &values[0]); break;
            case 3: glVertexAttrib3fv(uniform->id, &values[0]); break;
            case 4: glVertexAttrib4fv(uniform->id, &values[0]); break;
            }
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
