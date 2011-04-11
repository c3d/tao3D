// ****************************************************************************
//  lighting.cpp                                                    Tao project
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


TAO_BEGIN

void LightId::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Select and enable or disable a light
// ----------------------------------------------------------------------------
{
    where->lightId = GL_LIGHT0 + id;
    if (enable)
    {
        glEnable(where->lightId);
        glEnable(GL_LIGHTING);
        glEnable(GL_COLOR_MATERIAL);
        glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
        glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    }
    else
    {
        glDisable(where->lightId);
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
}


void ShaderProgram::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Activate the given shader program
// ----------------------------------------------------------------------------
{
    if (!where->globalProgramId)
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
    if (where->programId)
    {
        ShaderUniformInfo   *uniform   = name->GetInfo<ShaderUniformInfo>();
        ShaderAttributeInfo *attribute = name->GetInfo<ShaderAttributeInfo>();
        if (!uniform && !attribute)
        {
            kstring cname = name->value.c_str();
            GLuint uni = glGetUniformLocation(where->programId, cname);
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

            //Check if glGetActiveUniformsiv exists in order to obtain type of the uniform (according to glew)
            if(glGetActiveUniformsiv != NULL)
            {
                glGetActiveUniformsiv(where->programId, 1, &id, GL_UNIFORM_TYPE, &type);
            }
            else
            {
                GLint uniformMaxLength = 0;
                glGetProgramiv( where->programId, GL_ACTIVE_UNIFORM_MAX_LENGTH, &uniformMaxLength );

                GLint size = 0;
                GLchar* uniformName = new GLchar[uniformMaxLength];
                glGetActiveUniform( where->programId, id, uniformMaxLength, NULL, &size, (GLenum*) &type, uniformName);
                delete uniformName;
            }

            switch (type)
            {
            case GL_SAMPLER_1D:
            case GL_SAMPLER_2D:
            case GL_SAMPLER_3D:
            case GL_SAMPLER_CUBE:
                glUniform1i(id, values[0]);
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
