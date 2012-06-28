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

    // Setup default depth func
    depthFunc = GL_LESS;

    current = this;
}


// ============================================================================
//
//                        Matrix management functions
//
// ============================================================================


void GraphicState::PushMatrix()
// ----------------------------------------------------------------------------
//    Push current matrix in the stack
// ----------------------------------------------------------------------------
{
    switch(matrixMode)
    {
    case GL_PROJECTION: projStack.push(*currentMatrix); break;
    case GL_MODELVIEW: mvStack.push(*currentMatrix); break;
    default: break;
    }
}


void GraphicState::PopMatrix()
// ----------------------------------------------------------------------------
//    Pop last matrix of the stack
// ----------------------------------------------------------------------------
{
    switch(matrixMode)
    {
    case GL_PROJECTION: (*currentMatrix) = projStack.top(); projStack.pop(); break;
    case GL_MODELVIEW: (*currentMatrix) = mvStack.top(); mvStack.pop(); break;
    default: break;
    }

    currentMatrix->needUpdate = true;
    LoadMatrix();
}


void GraphicState::MatrixMode(GLenum mode)
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
        case GL_PROJECTION: currentMatrix = &projMatrix; break;
        case GL_MODELVIEW:  currentMatrix = &mvMatrix; break;
        default: break;
        }

        // TO REMOVE
        glMatrixMode(mode);
    }
}


void GraphicState::LoadMatrix()
// ----------------------------------------------------------------------------
//    Load current matrix
// ----------------------------------------------------------------------------
{
    if(currentMatrix->needUpdate)
        glLoadMatrixd(currentMatrix->matrix.Data(false));

    currentMatrix->needUpdate = false;
}


void GraphicState::LoadIdentity()
// ----------------------------------------------------------------------------
//    Load identity matrix
// ----------------------------------------------------------------------------
{
    currentMatrix->matrix.LoadIdentity();
    currentMatrix->needUpdate = true;
}


void GraphicState::PrintMatrix(GLuint model)
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

// ============================================================================
//
//                        Transformations functions
//
// ============================================================================


void GraphicState::Translate(double x, double y, double z)
// ----------------------------------------------------------------------------
//    Setup translation
// ----------------------------------------------------------------------------
{
    // Do not need to translate if all values are null
    if(x != 0.0 || y != 0.0 || z != 0.0)
    {
        currentMatrix->matrix.Translate(x, y, z);
        currentMatrix->needUpdate = true;
    }
}


void GraphicState::Rotate(double a, double x, double y, double z)
// ----------------------------------------------------------------------------
//    Setup rotation
// ----------------------------------------------------------------------------
{
    // Do not need to rotate if all values are null
    if(a != 0.0 && (x != 0.0 || y != 0.0 || z != 0.0))
    {
        currentMatrix->matrix.Rotate(a, x, y, z);
        currentMatrix->needUpdate = true;
    }
}


void GraphicState::Scale(double x, double y, double z)
// ----------------------------------------------------------------------------
//    Setup scale
// ----------------------------------------------------------------------------
{
    // Do not need to scale if all values are equals to 1
    if((x != 1.0) || (y != 1.0) || (z != 1.0))
    {
        currentMatrix->matrix.Scale(x, y, z);
        currentMatrix->needUpdate = true;
    }
}


// ============================================================================
//
//                       Camera management functions.
//
// ============================================================================


void GraphicState::PickMatrix(float x, float y, float width, float height,
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


void GraphicState::Frustum(float left, float right,
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


void GraphicState::Perspective(float fovy, float aspect, float nearZ, float farZ)
// ----------------------------------------------------------------------------
//    Set up a perspective projection matrix
// ----------------------------------------------------------------------------
{
   float frustumW, frustumH;

   frustumH = tanf( fovy / 360.0f * M_PI ) * nearZ;
   frustumW = frustumH * aspect;

   Frustum(-frustumW, frustumW, -frustumH, frustumH, nearZ, farZ );
}


void GraphicState::Ortho(float left, float right,
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


void GraphicState::Ortho2D(float left, float right, float bottom, float top)
// ----------------------------------------------------------------------------
//    Multiply the current matrix with an 2D orthographic matrix
// ----------------------------------------------------------------------------
{
    Ortho(left, right, bottom, top, -1, 1);
}


void GraphicState::LookAt(float eyeX, float eyeY, float eyeZ,
                             float centerX, float centerY, float centerZ,
                             float upX, float upY, float upZ)
// ----------------------------------------------------------------------------+
//    Multiply the current matrix with a viewing matrix
// ----------------------------------------------------------------------------+
{
    Vector3 eye(eyeX, eyeY, eyeZ);
    Vector3 center(centerX, centerY, centerZ);
    Vector3 up(upX, upY, upZ);

    LookAt(eye, center, up);
}


void GraphicState::LookAt(Vector3 eye, Vector3 center, Vector3 up)
// ----------------------------------------------------------------------------+
//    Multiply the current matrix with a viewing matrix
// ----------------------------------------------------------------------------+
{
    // Compute forward vector
    Vector3 forward = (center - eye).Normalize();

    // Compute side vector
    Vector3 side = forward;
    up.Normalize();
    side.Cross(up).Normalize();

    // Compute new up vector
    Vector3 upVector = side;
    upVector.Cross(forward).Normalize();

    Matrix4 view(false);

    view(0, 0) = side.x;
    view(1, 0) = side.y;
    view(2, 0) = side.z;
    view(0, 1) = upVector.x;
    view(1, 1) = upVector.y;
    view(2, 1) = upVector.z;
    view(0, 2) = -forward.x;
    view(1, 2) = -forward.y;
    view(2, 2) = -forward.z;
    view(3, 3) = 1.0;

    view(3, 0) = view(3, 1) = view(3, 2) = 0.0;
    view(0, 3) = view(1, 3) = view(2, 3) = 0.0;

    // Update current matrix
    currentMatrix->matrix *= view;

    // TO REMOVE
    currentMatrix->needUpdate = true;
    currentMatrix->matrix.Translate(-eye.x, -eye.y, -eye.z);
    LoadMatrix();
}


void GraphicState::Viewport(int x, int y, int w, int h)
// ----------------------------------------------------------------------------
//    Set the viewport
// ----------------------------------------------------------------------------
{
    // Do not need to setup viewport if it has not changed
    if((x == viewport[0]) && (y == viewport[1]) &&
       (w == viewport[2]) && (h == viewport[3]))
        return;

    // Update viewport
    viewport[0] = x;
    viewport[1] = y;
    viewport[2] = w;
    viewport[3] = h;
    glViewport(x, y, w, h);
}


// ============================================================================
//
//                       Draw management functions.
//
// ============================================================================

void GraphicState::Color(float r, float g, float b, float a)
// ----------------------------------------------------------------------------
//    Setup color
// ----------------------------------------------------------------------------
{
    // Do not need to setup color if it has not changed
    if((r == color[0]) && (g == color[1]) &&
       (b == color[2]) && (a == color[3]))
        return;

    // Update color
    color[0] = r;
    color[1] = g;
    color[2] = b;
    color[3] = a;
    glColor4f(r, g, b, a);
}


void GraphicState::ClearColor(float r, float g, float b, float a)
// ----------------------------------------------------------------------------
//    Setup clear color
// ----------------------------------------------------------------------------
{
    // Do not need to setup clear color if it has not changed
    if((r == clearColor[0]) && (g == clearColor[1]) &&
       (b == clearColor[2]) && (a == clearColor[3]))
        return;

    // Update clear color
    clearColor[0] = r;
    clearColor[1] = g;
    clearColor[2] = b;
    clearColor[3] = a;
    glClearColor(r, g, b, a);
}


void GraphicState::Clear(GLuint mask)
// ----------------------------------------------------------------------------
//    Clear buffers to preset values
// ----------------------------------------------------------------------------
{
    // Do not need to be optimised
    glClear(mask);
}


void GraphicState::LineWidth(float width)
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


void GraphicState::LineStipple(GLint factor, GLushort pattern)
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


void GraphicState::DepthMask(GLboolean flag)
// ----------------------------------------------------------------------------
//    Enable or disable writing into the depth buffer
// ----------------------------------------------------------------------------
{
    if(depthMask != flag)
    {
        depthMask = flag;
        glDepthMask(flag);
    }
}


void GraphicState::DepthFunc(GLenum func)
// ----------------------------------------------------------------------------
//    Specify the value used for depth buffer comparisons
// ----------------------------------------------------------------------------
{
    if(depthFunc != func)
    {
        depthFunc = func;
        glDepthFunc(func);
    }
}


// ============================================================================
//
//                       Misc functions.
//
// ============================================================================

void GraphicState::Enable(GLenum cap)
// ----------------------------------------------------------------------------
//    Enable capability
// ----------------------------------------------------------------------------
{
    glEnable(cap);
}


void GraphicState::Disable(GLenum cap)
// ----------------------------------------------------------------------------
//    Disable capability
// ----------------------------------------------------------------------------
{
    glDisable(cap);
}


void GraphicState::ShadeModel(GLenum mode)
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

