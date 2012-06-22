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
    : maxTextureCoords(0), maxTextureUnits(0), matrixMode(GL_MODELVIEW),
      shadeMode(GL_SMOOTH), lineWidth(1), stippleFactor(1), stipplePattern(1)
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


void GraphicState::pushMatrix()
// ----------------------------------------------------------------------------
//    Push current matrix in the stack
// ----------------------------------------------------------------------------
{
    switch(matrixMode)
    {
    case GL_PROJECTION: projStack.push(*currentMatrix); break;
    default: break;
    }
}


void GraphicState::popMatrix()
// ----------------------------------------------------------------------------
//    Pop last matrix of the stack
// ----------------------------------------------------------------------------
{
    switch(matrixMode)
    {
    case GL_PROJECTION: (*currentMatrix) = projStack.top(); projStack.pop(); break;
    default: break;
    }

    currentMatrix->needUpdate = true;
    loadMatrix();
}


void GraphicState::setMatrixMode(GLenum mode)
// ----------------------------------------------------------------------------
//    Setup texture matrix
//    Set matrix mode
// ----------------------------------------------------------------------------
{
    // Setup the new matrix mode
    if(matrixMode != mode)
    {
        matrixMode = mode;
        switch(mode)
        {
        case GL_PROJECTION: currentMatrix = &projectionMatrix; break;
        default: break;
        }
        glMatrixMode(mode);
    }
}


void GraphicState::loadMatrix()
// ----------------------------------------------------------------------------
//    Load current matrix
// ----------------------------------------------------------------------------
{
    if(currentMatrix->needUpdate)
        glLoadMatrixd(currentMatrix->matrix.Data());

    currentMatrix->needUpdate = false;
}


void GraphicState::loadIdentity()
// ----------------------------------------------------------------------------
//    Load identity matrix
// ----------------------------------------------------------------------------
{
    currentMatrix->matrix.LoadIdentity();
    currentMatrix->needUpdate = true;
}


void GraphicState::printMatrix(GLuint model)
// ----------------------------------------------------------------------------
//    Print GL matrix on stderr : GL_MODELVIEW/GL_PROJECTION/GL_TEXTURE
// ----------------------------------------------------------------------------
{
    GLenum matrixName;
    switch(model)
    {
    case GL_PROJECTION: matrixName = GL_PROJECTION_MATRIX; break;
    case GL_TEXTURE:    matrixName = GL_TEXTURE_MATRIX;    break;
    default :           matrixName = GL_MODELVIEW_MATRIX;  break;
    }

    GLdouble matrix[16];
    std::cerr << "Current matrix is " << matrixMode <<std::endl;
    if (model != matrixMode)
    {
        glMatrixMode(model);
        std::cerr << "Matrix mode set to " << model <<std::endl;
        glGetDoublev(matrixName, matrix);
        glMatrixMode(matrixMode);
        std::cerr << "Matrix mode restored to " << matrixMode <<std::endl;
    }
    else
        glGetDoublev(matrixName, matrix);

    for (int i = 0; i < 16; i+=4)
    {
        std::cerr << matrix[i] << "  " << matrix[i+1] << "  " << matrix[i+2]
                << "  " <<matrix[i+3] << "  " <<std::endl;
    }
}

void GraphicState::setFrustum(float left, float right,
                              float bottom, float top,
                              float nearZ, float farZ)
// ----------------------------------------------------------------------------
//     Multiply the current matrix by a perspective matrix
// ----------------------------------------------------------------------------
{
    float       deltaX = right - left;
    float       deltaY = top - bottom;
    float       deltaZ = farZ - nearZ;
    Matrix4     frust(false);

    if ( (nearZ <= 0.0f) || (farZ <= 0.0f) ||
         (deltaX <= 0.0f) || (deltaY <= 0.0f) || (deltaZ <= 0.0f) )
         return;

    frust(0, 0) = 2.0f * nearZ / deltaX;
    frust(1, 0) = frust(2, 0) = frust(3, 0) = 0.0f;

    frust(1, 1) = 2.0f * nearZ / deltaY;
    frust(0, 1) = frust(2, 1) = frust(3, 1) = 0.0f;

    frust(0, 2) = (right + left) / deltaX;
    frust(1, 2) = (top + bottom) / deltaY;
    frust(2, 2) = -(nearZ + farZ) / deltaZ;
    frust(3, 2) = -1.0f;

    frust(2, 3) = -2.0f * nearZ * farZ / deltaZ;
    frust(0, 3) = frust(1, 3) = frust(3, 3) = 0.0f;

    // Update current matrix
    currentMatrix->matrix    *= frust;
    currentMatrix->needUpdate = true;
}


void GraphicState::setPerspective(float fovy, float aspect, float nearZ, float farZ)
// ----------------------------------------------------------------------------
//    Set up a perspective projection matrix
// ----------------------------------------------------------------------------
{
   float frustumW, frustumH;

   frustumH = tanf( fovy / 360.0f * M_PI ) * nearZ;
   frustumW = frustumH * aspect;

   setFrustum(-frustumW, frustumW, -frustumH, frustumH, nearZ, farZ );
}


void GraphicState::setOrtho(float left, float right,
                            float bottom, float top,
                            float nearZ, float farZ)
// ----------------------------------------------------------------------------
//    Multiply the current matrix with an orthographic matrix
// ----------------------------------------------------------------------------
{
    float       deltaX = right - left;
    float       deltaY = top - bottom;
    float       deltaZ = farZ - nearZ;
    Matrix4     ortho(false);

    if ( (deltaX == 0.0f) || (deltaY == 0.0f) || (deltaZ == 0.0f) )
        return;

    ortho(0, 0) = 2.0 / deltaX;
    ortho(0, 3) = -(right + left) / deltaX;

    ortho(1, 1) = 2.0 / deltaY;
    ortho(1, 3) = -(top + bottom) / deltaY;

    ortho(2, 2) =  -2.0 / deltaZ;
    ortho(2, 3) = -(nearZ + farZ) / deltaZ;

    ortho(3, 3) = 1.0;

    ortho(0, 1) = ortho(0, 2) = 0.0;
    ortho(1, 0) = ortho(1, 2) = 0.0;
    ortho(2, 0) = ortho(2, 1) = 0.0;
    ortho(3, 0) = ortho(3, 1) = ortho(3, 2) = 0.0;

    // Update current matrix
    currentMatrix->matrix    *= ortho;
    currentMatrix->needUpdate = true;
}


void GraphicState::setOrtho2D(float left, float right, float bottom, float top)
// ----------------------------------------------------------------------------
//    Multiply the current matrix with an 2D orthographic matrix
// ----------------------------------------------------------------------------
{
    setOrtho(left, right, bottom, top, -1, 1);
}


void GraphicState::pickMatrix(float x, float y, float width, float height,
                              int viewport[4])
// ----------------------------------------------------------------------------
//    Define a picking region
// ----------------------------------------------------------------------------
{
    if (width <= 0 || height <= 0)
        return;

    float sx = viewport[2] / width;
    float sy = viewport[3] / height;
    float tx = (viewport[2] + 2.0 * (viewport[0] - x)) / width;
    float ty = (viewport[3] + 2.0 * (viewport[1] - y)) / height;

    currentMatrix->matrix.Translate(tx, ty, 0);
    currentMatrix->matrix.Scale(sx, sy, 1.0);
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
    {
        glTranslatef(x, y, z);
        currentMatrix->needUpdate = true;
    }
}


void GraphicState::rotate(double a, double x, double y, double z)
// ----------------------------------------------------------------------------
//    Setup rotation
// ----------------------------------------------------------------------------
{
    // Do not need to rotate if all values are null
    if(a != 0.0 && (x != 0.0 || y != 0.0 || z != 0.0))
    {
        glRotated(a, x, y, z);
        currentMatrix->needUpdate = true;
    }
}


void GraphicState::scale(double x, double y, double z)
// ----------------------------------------------------------------------------
//    Setup scale
// ----------------------------------------------------------------------------
{
    // Do not need to scale if all values are equals to 1
    if((x != 1.0) || (y != 1.0) || (z != 1.0))
    {
        glScaled(x, y, z);
        currentMatrix->needUpdate = true;
    }
}


void GraphicState::setLineWidth(float width)
// ----------------------------------------------------------------------------
//    Specify the width of rasterized lines
// ----------------------------------------------------------------------------
{
    if(width != lineWidth)
    {
        lineWidth = width;
        glLineWidth(width);
    }
}


void GraphicState::setLineStipple(GLint factor, GLushort pattern)
// ----------------------------------------------------------------------------
//    Specify the line stipple pattern
// ----------------------------------------------------------------------------
{
    if((factor != stippleFactor) || (pattern != stipplePattern))
    {
        stippleFactor = factor;
        stipplePattern = pattern;
        glLineStipple(factor, pattern);
    }
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


void GraphicState::shadeModel(GLenum mode)
// ----------------------------------------------------------------------------
//    Select shading mode
// ----------------------------------------------------------------------------
{
    if(mode != shadeMode)
        glShadeModel(mode);
}


std::ostream & GraphicState::debug()
// ----------------------------------------------------------------------------
//   Convenience method to log with a common prefix
// ----------------------------------------------------------------------------
{
    std::cerr << "[GraphicState] ";
    return std::cerr;
}

TAO_END

