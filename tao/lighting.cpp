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
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
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
    (void) where;
    glMaterialfv(face, function, &args[0]);
}


void ShaderProgram::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Activate the given shader program
// ----------------------------------------------------------------------------
{
    program->bind();
    where->programId = program->programId();
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
            glUniform1fv(uniform->id, values.size(), &values[0]);
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
