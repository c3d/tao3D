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
        glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
        glEnable(GL_COLOR_MATERIAL);
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

TAO_END
