// ****************************************************************************
//  states.cpp                                                      Tao project
// ****************************************************************************
//
//   File Description:
//
//     Manage Opengl states
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
//  (C) 2010 Lionel Schaffhauser <lionel@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "graphic_state.h"

TAO_BEGIN

// Current graphic state
GraphicState* GraphicState::current = NULL;

// Vendors list
text GraphicState::vendorsList[LAST] = { "ATI Technologies Inc.", "NVIDIA Corporation", "Intel" };


GraphicState::GraphicState()
// ----------------------------------------------------------------------------
//    Constructor
// ----------------------------------------------------------------------------
    : maxTextureCoords(0), maxTextureUnits(0), matrixMode(GL_MODELVIEW)
{
    // Ask graphic card constructor to OpenGL
    vendor = text ( (const char*)glGetString ( GL_VENDOR ) );
    int vendorNum = 0;

    // Search in vendors list
    for(int i = 0; i < LAST; i++)
    {
        if(! vendor.compare(vendorsList[i]))
        {
            vendorNum = i;
            break;
        }
    }

    switch(vendorNum)
    {
    case 0: vendorID = ATI; break;
    case 1: vendorID = NVIDIA; break;
    case 2: vendorID = INTEL; break;
    }

    const GLubyte *str;

    // Get OpenGL supported version
    str = glGetString(GL_VERSION);
    version = (const char*) str;

    // Get OpenGL supported extentions
    str = glGetString(GL_EXTENSIONS);
    extensionsAvailable = (const char*) str;

    // Get OpenGL renderer (GPU)
    str = glGetString(GL_RENDERER);
    renderer = (const char*) str;

    // Get number of maximum texture units and coords in fragment shaders
    // (texture units are limited to 4 otherwise)
    glGetIntegerv(GL_MAX_TEXTURE_COORDS,(GLint*) &maxTextureCoords);
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS,(GLint*) &maxTextureUnits);


    // Set defaut color
    color[0] = 1.0;
    color[1] = 1.0;
    color[2] = 1.0;
    color[3] = 1.0;

    current = this;
}


void GraphicState::projectionMatrix()
// ----------------------------------------------------------------------------
//    Set projection matrix
// ----------------------------------------------------------------------------
{
    // Setup the projection matrix
    if(matrixMode != GL_PROJECTION)
    {
        matrixMode = GL_PROJECTION;
        glMatrixMode(GL_PROJECTION);
    }
}


void GraphicState::modelViewMatrix()
// ----------------------------------------------------------------------------
//    Setup modelview matrix
// ----------------------------------------------------------------------------
{
    // Setup the projection matrix
    if(matrixMode != GL_MODELVIEW)
    {
        matrixMode = GL_MODELVIEW;
        glMatrixMode(GL_MODELVIEW);
    }
}


void GraphicState::textureMatrix()
// ----------------------------------------------------------------------------
//    Setup texture matrix
// ----------------------------------------------------------------------------
{
    // Setup the projection matrix
    if(matrixMode != GL_TEXTURE)
    {
        matrixMode = GL_TEXTURE;
        glMatrixMode(GL_TEXTURE);
    }
}


void GraphicState::printMatrix(GLuint model)
// ----------------------------------------------------------------------------
//    Print GL matrix on stderr
// ----------------------------------------------------------------------------
{
    GLdouble matrix[16];
    std::cerr << "Current matrix is " << matrixMode <<std::endl;
    if (model != matrixMode)
    {
        glMatrixMode(model);
        std::cerr << "Matrix mode set to " << model <<std::endl;
        glGetDoublev(model, matrix);
        glMatrixMode(matrixMode);
        std::cerr << "Matrix mode restored to " << matrixMode <<std::endl;
    }
    else
        glGetDoublev(matrixMode, matrix);

    for (int i = 0; i < 16; i+=4)
    {
        std::cerr << matrix[i] << "  " << matrix[i+1] << "  " << matrix[i+2]
                << "  " <<matrix[i+3] << "  " <<std::endl;
    }

}


void GraphicState::setColor(float r, float g, float b, float a)
// ----------------------------------------------------------------------------
//    Setup color
// ----------------------------------------------------------------------------
{
    bool changed = false;

    // Compare with previous color
    if(r != color[0])
    {
        color[0] = r;
        changed = true;
    }
    if(g != color[1])
    {
        color[1] = g;
        changed = true;
    }
    if(b != color[2])
    {
        color[2] = b;
        changed = true;
    }
    if(a != color[3])
    {
        color[3] = a;
        changed = true;
    }

    // Do not need to setup color if it has not changed
    if(changed)
        glColor4f(r, g, b, a);
}


void GraphicState::translate(double x, double y, double z)
// ----------------------------------------------------------------------------
//    Setup translation
// ----------------------------------------------------------------------------
{
    // Do not need to translate if all values are null
    if(x != 0.0 || y != 0.0 || z != 0.0)
        glTranslatef(x, y, z);
}


void GraphicState::rotate(double a, double x, double y, double z)
// ----------------------------------------------------------------------------
//    Setup rotation
// ----------------------------------------------------------------------------
{
    // Do not need to rotate if all values are null
    if(a != 0.0 && (x != 0.0 || y != 0.0 || z != 0.0))
        glRotated(a, x, y, z);
}


void GraphicState::scale(double x, double y, double z)
// ----------------------------------------------------------------------------
//    Setup scale
// ----------------------------------------------------------------------------
{
    // Do not need to scale if all values are equals to 1
    if((x != 1.0) || (y != 1.0) || (z != 1.0))
        glScaled(x, y, z);
}


void GraphicState::enable(GLenum cap)
// ----------------------------------------------------------------------------
//    Enable capability
// ----------------------------------------------------------------------------
{
    glEnable(cap);
}


void GraphicState::disable(GLenum cap)
// ----------------------------------------------------------------------------
//    Disable capability
// ----------------------------------------------------------------------------
{
    glDisable(cap);
}



TAO_END

