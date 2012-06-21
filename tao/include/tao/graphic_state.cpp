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

GraphicState::GraphicState()
// ----------------------------------------------------------------------------
//    Constructor
// ----------------------------------------------------------------------------
    : matrixMode(GL_MODELVIEW)
{
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


void GraphicState::translate(float x, float y, float z)
// ----------------------------------------------------------------------------
//    Setup translation
// ----------------------------------------------------------------------------
{
    // Do not need to translate if all values are null
    if(x != 0.0 || y != 0.0 || z != 0.0)
        glTranslatef(x, y, z);
}


void GraphicState::rotate(float a, float x, float y, float z)
// ----------------------------------------------------------------------------
//    Setup rotation
// ----------------------------------------------------------------------------
{
    // Do not need to rotate if all values are null
    if(a != 0.0 && (x != 0.0 || y != 0.0 || z != 0.0))
        glRotatef(a, x, y, z);
}


void GraphicState::scale(float x, float y, float z)
// ----------------------------------------------------------------------------
//    Setup scale
// ----------------------------------------------------------------------------
{
    // Do not need to scale if all values are equals to 1
    if((x != 1.0) || (y != 1.0) || (z != 1.0))
        glScalef(x, y, z);
}


TAO_END

