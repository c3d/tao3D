// ****************************************************************************
//  opengl_state.cpp                                               Tao project
// ****************************************************************************
//
//   File Description:
//
//     Manage OpenGL states
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
//  (C) 2012 Baptiste Soulisse <baptiste.soulisse@taodyne.com>
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Lionel Schaffhauser <lionel@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "opengl_state.h"
#include "opengl_save.h"
#include "application.h"
#include "texture_cache.h"
#include <cassert>
#include <ostream>


TAO_BEGIN

text OpenGLState::vendorsList[LAST_VENDOR] =
// ----------------------------------------------------------------------------
//   List of vendors (we use that for some optimizations)
// ----------------------------------------------------------------------------
{
    "Unknown vendor",
    "ATI Technologies Inc.",
    "NVIDIA Corporation",
    "Intel"
};


// FIXME: Is there a better place for this than here?
GraphicState *GraphicState::current = NULL;


// Test if we need to save a state
#define CHANGE(name, value)                     \
    do                                          \
    {                                           \
        if (name != value)                      \
        {                                       \
            if (save)                           \
                save->save_##name(name);        \
            name##_isDirty = true;              \
            name = value;                       \
        }                                       \
    } while (0)


#define SAVE(name)                              \
    do                                          \
    {                                           \
        if (save)                               \
            save->save_##name(name);            \
    } while (0)


#define CHANGE_MATRIX(Code)                             \
    do                                                  \
    {                                                   \
        switch(matrixMode)                              \
        {                                               \
        case GL_MODELVIEW:                              \
        {                                               \
            SAVE(mvMatrix);                             \
            Matrix4 &matrix = mvMatrix;                 \
            Code;                                       \
            mvMatrix_isDirty = true;                    \
            break;                                      \
        }                                               \
        case GL_PROJECTION:                             \
        {                                               \
            SAVE(projMatrix);                           \
            Matrix4 &matrix = projMatrix;               \
            Code;                                       \
            projMatrix_isDirty = true;                  \
            break;                                      \
        }                                               \
        case GL_TEXTURE:                                \
        {                                               \
            SAVE(textures);                             \
            Matrix4 &matrix = TextureMatrix();          \
            Code;                                       \
            textures_isDirty = true;                    \
            break;                                      \
        }                                               \
        case GL_COLOR:                                  \
        {                                               \
            SAVE(colorMatrix);                          \
            Matrix4 &matrix = colorMatrix;              \
            Code;                                       \
            colorMatrix_isDirty = true;                 \
            break;                                      \
        }                                               \
        }                                               \
    } while(0)


OpenGLState::OpenGLState()
// ----------------------------------------------------------------------------
//    Constructor make sure we are a singleton, and initializes it
// ----------------------------------------------------------------------------
    : GraphicState(),
#define GS(type, name)          name##_isDirty(true),
#include "opengl_state.tbl"
      maxTextureCoords(0), maxTextureUnits(0),
      currentUnit(NULL), matrixMode(GL_MODELVIEW),
      viewport(0, 0, 0, 0), listBase(0), pointSize(1),
      color(1,1,1,1), clearColor(0,0,0,1),
      frontAmbient(0,0,0,0), frontDiffuse(0,0,0,0),
      frontSpecular(0,0,0,0), frontEmission(0,0,0,0), frontShininess(0),
      backAmbient(0,0,0,0), backDiffuse(0,0,0,0),
      backSpecular(0,0,0,0), backEmission(0,0,0,0), backShininess(0),
      shadeMode(GL_SMOOTH), lineWidth(1),
      stipple(1, -1), cullMode(GL_BACK),
      depthMask(true), depthFunc(GL_LESS),
      textureCompressionHint(GL_DONT_CARE),
      perspectiveCorrectionHint(GL_DONT_CARE),
      blendFunction(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO),
      blendEquation(GL_FUNC_ADD), alphaFunc(GL_ALWAYS, 0.0),
      renderMode(GL_RENDER), shaderProgram(0), activeTexture(0),
#define GS(type, name)
#define GFLAG(name)     glflag_##name(false),
#include "opengl_state.tbl"
      save(NULL)
{
    // Ask graphic card constructor to OpenGL
    vendor = text ( (const char*)glGetString ( GL_VENDOR ) );

    // Search in vendors list
    vendorID = UNKNOWN;
    for(uint i = UNKNOWN; i < LAST_VENDOR; i++)
    {
        if(!vendor.compare(vendorsList[i]))
        {
            vendorID = (enum VendorID) i;
            break;
        }
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

    // As we don't known if GL context is single or double buffered,
    // we initialize bufferMode to 0
    bufferMode = 0;

    if (maxTextureUnits > MAX_TEXTURE_UNITS)
        maxTextureUnits = MAX_TEXTURE_UNITS;

    // Initialize texture unit 0
    textures.textures.push_back(TextureState());
    currentUnit = &textures.textures[0];
}


std::ostream & OpenGLState::debug()
// ----------------------------------------------------------------------------
//   Convenience method to log with a common prefix
// ----------------------------------------------------------------------------
{
    std::cerr << "[OpenGLState] ";
    return std::cerr;
}


// ============================================================================
// 
//    Save/restore state
// 
// ============================================================================

GraphicSave *OpenGLState::Save()
// ----------------------------------------------------------------------------
//   Prepare to save the state
// ----------------------------------------------------------------------------
{
    return new OpenGLSave(this);
}


void OpenGLState::Restore(GraphicSave *saved)
// ----------------------------------------------------------------------------
//   Restore the state that was saved in 'saved' and deletes 'saved'
// ----------------------------------------------------------------------------
{
    delete saved;
}


uint OpenGLState::ShowErrors()
// ----------------------------------------------------------------------------
//   Display all OpenGL errors in the error window
// ----------------------------------------------------------------------------
{
    static GLenum last = GL_NO_ERROR;
    static unsigned int count = 0;
    GLenum err = glGetError();
    uint errorsFound = 0;
    while (err != GL_NO_ERROR)
    {
        errorsFound++;
        if (!count)
        {
            char *p = (char *) gluErrorString(err);
            std::cerr << "GL Error: " << p
                      << "[error code: " << err << "]\n";
            last = err;
        }
        if (err != last || count == 100)
        {
            std::cerr << "GL Error: Error " << last
                      << " repeated " << count << " times\n";
            count = 0;
        }
        else
        {
            count++;
        }
        err = glGetError();
    }

    return errorsFound;
}


void OpenGLState::Sync(ulonglong which)
// ----------------------------------------------------------------------------
//    Synchronize all pending changes and send them to the card
// ----------------------------------------------------------------------------
{
    bool needSync =
#define GS(type, name) name##_isDirty |
#include "opengl_state.tbl"
        false;
    if (!needSync)
        return;

#define SYNC(name, Code)                                \
    do                                                  \
    {                                                   \
        if (name##_isDirty && (which & STATE_##name))   \
        {                                               \
            Code;                                       \
            name##_isDirty = false;                     \
            ShowErrors();                               \
        }                                               \
    } while(0)

    GLenum mmode = matrixMode;
    SYNC(textures,
         currentTextures.Sync(textures);
         matrixMode_isDirty = true /* Pessimistic: we don't know if set */);
    SYNC(mvMatrix,
         if (matrixMode_isDirty || mmode != GL_MODELVIEW)
         {
             glMatrixMode(GL_MODELVIEW);
             mmode = GL_MODELVIEW;
             matrixMode_isDirty = mmode != matrixMode;
         }
         glLoadMatrixd(mvMatrix.Data(false)));
    SYNC(projMatrix,
         if (matrixMode_isDirty || mmode != GL_PROJECTION)
         {
             glMatrixMode(GL_PROJECTION);
             mmode = GL_PROJECTION;
             matrixMode_isDirty = mmode != matrixMode;
         }
         glLoadMatrixd(projMatrix.Data(false)));
    SYNC(colorMatrix,
         if (matrixMode_isDirty || mmode != GL_COLOR)
         {
             glMatrixMode(GL_COLOR);
             mmode = GL_COLOR;
             matrixMode_isDirty = mmode != matrixMode;
         }
         glLoadMatrixd(colorMatrix.Data(false)));
    SYNC(matrixMode,
         glMatrixMode(matrixMode));
    SYNC(bufferMode,
         glDrawBuffer(bufferMode));
    SYNC(viewport,
         glViewport(viewport.x, viewport.y, viewport.w, viewport.h));
    SYNC(pointSize,
         glPointSize(pointSize));
    SYNC(color,
         glColor4f(color.red, color.green, color.blue, color.alpha));
    SYNC(clearColor,
         Tao::Color &c = clearColor;
         glClearColor(c.red,c.green,c.blue,c.alpha));
    SYNC(frontAmbient,
         glMaterialfv(GL_FRONT, GL_AMBIENT, frontAmbient.Data()));
    SYNC(frontDiffuse,
         glMaterialfv(GL_FRONT, GL_DIFFUSE, frontDiffuse.Data()));
    SYNC(frontSpecular,
         glMaterialfv(GL_FRONT, GL_SPECULAR, frontSpecular.Data()));
    SYNC(frontEmission,
         glMaterialfv(GL_FRONT, GL_EMISSION, frontEmission.Data()));
    SYNC(frontShininess,
         glMaterialf(GL_FRONT, GL_SHININESS, frontShininess));
    SYNC(backAmbient,
         glMaterialfv(GL_BACK, GL_AMBIENT, backAmbient.Data()));
    SYNC(backDiffuse,
         glMaterialfv(GL_BACK, GL_DIFFUSE, backDiffuse.Data()));
    SYNC(backSpecular,
         glMaterialfv(GL_BACK, GL_SPECULAR, backSpecular.Data()));
    SYNC(backEmission,
         glMaterialfv(GL_BACK, GL_EMISSION, backEmission.Data()));
    SYNC(backShininess,
         glMaterialf(GL_BACK, GL_SHININESS, backShininess));
    SYNC(shadeMode,
         glShadeModel(shadeMode));
    SYNC(lineWidth,
         glLineWidth(lineWidth));
    SYNC(stipple,
         glLineStipple(stipple.factor, stipple.pattern));
    SYNC(cullMode,
         glCullFace(cullMode));
    SYNC(depthMask,
         glDepthMask(depthMask));
    SYNC(depthFunc,
         glDepthFunc(depthFunc));
    SYNC(textureCompressionHint,
         glHint(GL_TEXTURE_COMPRESSION_HINT, textureCompressionHint));
    SYNC(perspectiveCorrectionHint,
         glHint(GL_PERSPECTIVE_CORRECTION_HINT, perspectiveCorrectionHint));
    SYNC(blendFunction,
         BlendFunctionState &b = blendFunction;
         glBlendFuncSeparate(b.srcRgb, b.destRgb, b.srcAlpha, b.destAlpha));
    SYNC(blendEquation,
         glBlendEquation(blendEquation));
    SYNC(alphaFunc,
         glAlphaFunc(alphaFunc.func, alphaFunc.ref));
    SYNC(listBase,
         glListBase(listBase));
    SYNC(shaderProgram,
         glUseProgram(shaderProgram));

#define GS(type, name)
#define GFLAG(name)                             \
    SYNC(glflag_##name,                         \
    if (glflag_##name)                          \
        glEnable(name);                         \
    else                                        \
        glDisable(name));
#include "opengl_state.tbl"

#undef SYNC
}



// ============================================================================
//
//                        Matrix management functions
//
// ============================================================================

void OpenGLState::MatrixMode(GLenum mode)
// ----------------------------------------------------------------------------
//    Set matrix mode
// ----------------------------------------------------------------------------
{
    // Setup the new matrix mode
    CHANGE(matrixMode, mode);
}


void OpenGLState::LoadMatrix()
// ----------------------------------------------------------------------------
//    Load current matrix
// ----------------------------------------------------------------------------
{
    Sync(STATE_mvMatrix | STATE_projMatrix);
}


void OpenGLState::LoadIdentity()
// ----------------------------------------------------------------------------
//    Load identity matrix
// ----------------------------------------------------------------------------
{
    CHANGE_MATRIX(matrix.LoadIdentity());
}


void OpenGLState::MultMatrices(const coord *m1, const coord *m2, coord *result)
// ----------------------------------------------------------------------------
//    Multiply two 4x4 matrices (source: glu)
// ----------------------------------------------------------------------------
{
    for (int i= 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            result[i * 4 + j] = m1[i * 4 + 0] * m2[0 * 4 + j] +
                                m1[i * 4 + 1] * m2[1 * 4 + j] +
                                m1[i * 4 + 2] * m2[2 * 4 + j] +
                                m1[i * 4 + 3] * m2[3 * 4 + j];
}


void OpenGLState::MultMatrixVec(const coord matrix[16], const coord in[4],
                                 coord out[4])
// ----------------------------------------------------------------------------
//    Multiply a matrix by a vector (source: glu)
// ----------------------------------------------------------------------------
{
    for (int i = 0; i < 4; i++)
        out[i] = in[0] * matrix[0 * 4 + i] +
                 in[1] * matrix[1 * 4 + i] +
                 in[2] * matrix[2 * 4 + i] +
                 in[3] * matrix[3 * 4 + i];
}


bool OpenGLState::InvertMatrix(const GLdouble m[16], GLdouble invOut[16])
// ----------------------------------------------------------------------------
//    Compute inverse of a matrix (source: glu)
// ----------------------------------------------------------------------------
{
    double inv[16], det;
    int i;

    // First column
    inv[0] =    m[5]*m[10]*m[15] - m[5]*m[11]*m[14] - m[9]*m[6]*m[15]
              + m[9]*m[7]*m[14] + m[13]*m[6]*m[11] - m[13]*m[7]*m[10];
    inv[1] =  - m[1]*m[10]*m[15] + m[1]*m[11]*m[14] + m[9]*m[2]*m[15]
              - m[9]*m[3]*m[14] - m[13]*m[2]*m[11] + m[13]*m[3]*m[10];
    inv[2] =    m[1]*m[6]*m[15] - m[1]*m[7]*m[14] - m[5]*m[2]*m[15]
              + m[5]*m[3]*m[14] + m[13]*m[2]*m[7] - m[13]*m[3]*m[6];
    inv[3] =  - m[1]*m[6]*m[11] + m[1]*m[7]*m[10] + m[5]*m[2]*m[11]
              - m[5]*m[3]*m[10] - m[9]*m[2]*m[7] + m[9]*m[3]*m[6];

    // Second column
    inv[4] =  - m[4]*m[10]*m[15] + m[4]*m[11]*m[14] + m[8]*m[6]*m[15]
              - m[8]*m[7]*m[14] - m[12]*m[6]*m[11] + m[12]*m[7]*m[10];
    inv[5] =    m[0]*m[10]*m[15] - m[0]*m[11]*m[14] - m[8]*m[2]*m[15]
              + m[8]*m[3]*m[14] + m[12]*m[2]*m[11] - m[12]*m[3]*m[10];
    inv[6] =  - m[0]*m[6]*m[15] + m[0]*m[7]*m[14] + m[4]*m[2]*m[15]
              - m[4]*m[3]*m[14] - m[12]*m[2]*m[7] + m[12]*m[3]*m[6];
    inv[7] =    m[0]*m[6]*m[11] - m[0]*m[7]*m[10] - m[4]*m[2]*m[11]
              + m[4]*m[3]*m[10] + m[8]*m[2]*m[7] - m[8]*m[3]*m[6];

    // Third column
    inv[8] =    m[4]*m[9]*m[15] - m[4]*m[11]*m[13] - m[8]*m[5]*m[15]
              + m[8]*m[7]*m[13] + m[12]*m[5]*m[11] - m[12]*m[7]*m[9];
    inv[9] =  - m[0]*m[9]*m[15] + m[0]*m[11]*m[13] + m[8]*m[1]*m[15]
              - m[8]*m[3]*m[13] - m[12]*m[1]*m[11] + m[12]*m[3]*m[9];
    inv[10] =   m[0]*m[5]*m[15] - m[0]*m[7]*m[13] - m[4]*m[1]*m[15]
              + m[4]*m[3]*m[13] + m[12]*m[1]*m[7] - m[12]*m[3]*m[5];
    inv[11] = - m[0]*m[5]*m[11] + m[0]*m[7]*m[9] + m[4]*m[1]*m[11]
              - m[4]*m[3]*m[9] - m[8]*m[1]*m[7] + m[8]*m[3]*m[5];

    // Fourht column
    inv[12] = - m[4]*m[9]*m[14] + m[4]*m[10]*m[13] + m[8]*m[5]*m[14]
              - m[8]*m[6]*m[13] - m[12]*m[5]*m[10] + m[12]*m[6]*m[9];
    inv[13] =   m[0]*m[9]*m[14] - m[0]*m[10]*m[13] - m[8]*m[1]*m[14]
              + m[8]*m[2]*m[13] + m[12]*m[1]*m[10] - m[12]*m[2]*m[9];
    inv[14] = - m[0]*m[5]*m[14] + m[0]*m[6]*m[13] + m[4]*m[1]*m[14]
              - m[4]*m[2]*m[13] - m[12]*m[1]*m[6] + m[12]*m[2]*m[5];
    inv[15] =   m[0]*m[5]*m[10] - m[0]*m[6]*m[9] - m[4]*m[1]*m[10]
              + m[4]*m[2]*m[9] + m[8]*m[1]*m[6] - m[8]*m[2]*m[5];

    // Compute determinant
    det = m[0]*inv[0] + m[1]*inv[4] + m[2]*inv[8] + m[3]*inv[12];
    if (det == 0)
        return false;

    det = 1.0 / det;
    for (i = 0; i < 16; i++)
        invOut[i] = inv[i] * det;

    return true;
}


void OpenGLState::PrintMatrix(GLuint model)
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

void OpenGLState::Translate(double x, double y, double z)
// ----------------------------------------------------------------------------
//    Setup translation
// ----------------------------------------------------------------------------
{
    // Do not need to translate if all values are null
    if (x != 0.0 || y != 0.0 || z != 0.0)
        CHANGE_MATRIX(matrix.Translate(x, y, z));
}


void OpenGLState::Rotate(double a, double x, double y, double z)
// ----------------------------------------------------------------------------
//    Setup rotation
// ----------------------------------------------------------------------------
{
    // Do not need to rotate if all values are null
    if (a != 0.0 && (x != 0.0 || y != 0.0 || z != 0.0))
        CHANGE_MATRIX(matrix.Rotate(a, x, y, z));
}


void OpenGLState::Scale(double x, double y, double z)
// ----------------------------------------------------------------------------
//    Setup scale
// ----------------------------------------------------------------------------
{
    // Do not need to scale if all values are equals to 1
    if (x != 1.0 || y != 1.0 || z != 1.0)
        CHANGE_MATRIX(matrix.Scale(x, y, z));
}



// ============================================================================
//
//                       Camera management functions.
//
// ============================================================================

bool OpenGLState::Project(coord objx, coord objy, coord objz,
                          const coord* mv, const coord* proj,
                          const int* viewport,
                          coord *winx, coord *winy, coord *winz)
// ----------------------------------------------------------------------------
//     Map object coordinates to window coordinates (source: glu)
// ----------------------------------------------------------------------------
{
    // Transformations matrices
    double in[4];
    double out[4];

    in[0] = objx;
    in[1] = objy;
    in[2] = objz;
    in[3] = 1.0;

    // Compute vectors
    MultMatrixVec(mv, in, out);
    MultMatrixVec(proj, out, in);

    if (in[3] == 0.0)
        return false;

    in[0] /= in[3];
    in[1] /= in[3];
    in[2] /= in[3];

    // Map x, y and z to range 0-1
    in[0] = in[0] * 0.5 + 0.5;
    in[1] = in[1] * 0.5 + 0.5;
    in[2] = in[2] * 0.5 + 0.5;

    // Map x,y to viewport
    in[0] = in[0] * viewport[2] + viewport[0];
    in[1] = in[1] * viewport[3] + viewport[1];

    *winx = in[0];
    *winy = in[1];
    *winz = in[2];

    return true;
}


bool OpenGLState::UnProject(coord winx, coord winy, coord winz,
                             const coord* mv, const coord* proj,
                             const int* viewport,
                             coord *objx, coord *objy, coord *objz)
// ----------------------------------------------------------------------------
//     Map window coordinates to object coordinates (source: glu)
// ----------------------------------------------------------------------------
{
    // Transformations matrices
    double finalMatrix[16];
    double in[4];
    double out[4];

    MultMatrices(mv, proj, finalMatrix);
    if (!InvertMatrix(finalMatrix, finalMatrix))
        return false;

    in[0] = winx;
    in[1] = winy;
    in[2] = winz;
    in[3] = 1.0;

    // Map x and y from window coordinates
    in[0] = (in[0] - viewport[0]) / viewport[2];
    in[1] = (in[1] - viewport[1]) / viewport[3];

    // Map to range -1 to 1
    in[0] = in[0] * 2 - 1;
    in[1] = in[1] * 2 - 1;
    in[2] = in[2] * 2 - 1;

    MultMatrixVec(finalMatrix, in, out);
    if (out[3] == 0.0)
        return false;

    out[0] /= out[3];
    out[1] /= out[3];
    out[2] /= out[3];

    *objx = out[0];
    *objy = out[1];
    *objz = out[2];

    return true;
}


void OpenGLState::PickMatrix(float x, float y,
                             float width, float height,
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

    CHANGE_MATRIX(matrix.Translate(tx, ty, 0).Scale(sx, sy, 1.0));
}


void OpenGLState::Frustum(float left, float right,
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
    CHANGE_MATRIX(matrix *= frust);
}


void OpenGLState::Perspective(float fovy, float aspect,
                              float nearZ, float farZ)
// ----------------------------------------------------------------------------
//    Set up a perspective projection matrix
// ----------------------------------------------------------------------------
{
   float frustumW, frustumH;

   frustumH = tanf( fovy / 360.0f * M_PI ) * nearZ;
   frustumW = frustumH * aspect;

   Frustum(-frustumW, frustumW, -frustumH, frustumH, nearZ, farZ );
}


void OpenGLState::Ortho(float left, float right,
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
    CHANGE_MATRIX(matrix *= ortho);
}


void OpenGLState::Ortho2D(float left, float right, float bottom, float top)
// ----------------------------------------------------------------------------
//    Multiply the current matrix with an 2D orthographic matrix
// ----------------------------------------------------------------------------
{
    Ortho(left, right, bottom, top, -1, 1);
}


void OpenGLState::LookAt(float eyeX, float eyeY, float eyeZ,
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


void OpenGLState::LookAt(Vector3 eye, Vector3 center, Vector3 up)
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
    CHANGE_MATRIX(matrix *= view;
                  /* TO REMOVE */
                  matrix.Translate(-eye.x, -eye.y, -eye.z));

    // TO REMOVE?
    LoadMatrix();
}



// ============================================================================
//
//                            Drawing functions.
//
// ============================================================================

void OpenGLState::DrawBuffer(GLenum mode)
// ----------------------------------------------------------------------------
//    Specify which color buffers are to be drawn into
// ----------------------------------------------------------------------------
{
    CHANGE(bufferMode, mode);
}


void OpenGLState::Begin(GLenum mode)
// ----------------------------------------------------------------------------
//    Delimit the vertices of a primitive or a group of like primitives
// ----------------------------------------------------------------------------
{
    // Not need to be optimised
    glBegin(mode);
}


void OpenGLState::End()
// ----------------------------------------------------------------------------
//    Delimit the vertices of a primitive or a group of like primitives
// ----------------------------------------------------------------------------
{
    // Not need to be optimised
    glEnd();
}


void OpenGLState::Vertex(coord x, coord y, coord z, coord w)
// ----------------------------------------------------------------------------
//    Specify a vertex within Begin/End
// ----------------------------------------------------------------------------
{
    // Not need to be optimised
    glVertex4d(x, y, z, w);
}


void OpenGLState::Vertex3v(const coord *array)
// ----------------------------------------------------------------------------
//    Specify a set of vertices
// ----------------------------------------------------------------------------
{
    // Not need to be optimised
    glVertex3dv(array);
}


void OpenGLState::Normal(coord nx, coord ny, coord nz)
// ----------------------------------------------------------------------------
//    Specify a normal within Begin/End
// ----------------------------------------------------------------------------
{
    // Not need to be optimised
    glNormal3d(nx, ny, nz);
}


void OpenGLState::TexCoord(coord s, coord t)
// ----------------------------------------------------------------------------
//    Specify a texture coordinate within Begin/End
// ----------------------------------------------------------------------------
{
    // Not need to be optimised
    glTexCoord2d(s, t);
}


void OpenGLState::MultiTexCoord3v(GLenum target, const coord *array)
// ----------------------------------------------------------------------------
//    Specify the coordinate of a texture unit
// ----------------------------------------------------------------------------
{
    // Not need to be optimised
    glMultiTexCoord3dv(target, array);
}


void OpenGLState::EnableClientState(GLenum cap)
// ----------------------------------------------------------------------------
//    Enable client-side capability
// ----------------------------------------------------------------------------
{
    glEnableClientState(cap);
}


void OpenGLState::DisableClientState(GLenum cap)
// ----------------------------------------------------------------------------
//    Disable client-side capability
// ----------------------------------------------------------------------------
{
    glDisableClientState(cap);
}


void OpenGLState::DrawArrays(GLenum mode, int first, int count)
// ----------------------------------------------------------------------------
//    Render primitives from array data
// ----------------------------------------------------------------------------
{
    // Not need to be optimised
    glDrawArrays(mode, first, count);
}


void OpenGLState::VertexPointer(int size, GLenum type, int stride, const void* pointer)
// ----------------------------------------------------------------------------
//    Define an array of vertex data
// ----------------------------------------------------------------------------
{
    // Not need to be optimised
    glVertexPointer(size, type, stride, pointer);
}


void OpenGLState::NormalPointer(GLenum type, int stride, const void* pointer)
// ----------------------------------------------------------------------------
//    Define an array of normal data
// ----------------------------------------------------------------------------
{
    // Not need to be optimised
    glNormalPointer(type, stride, pointer);
}


void OpenGLState::TexCoordPointer(int size, GLenum type, int stride, const void* pointer)
// ----------------------------------------------------------------------------
//    Define an array of texture coordinates
// ----------------------------------------------------------------------------
{
    // Not need to be optimised
    glTexCoordPointer(size, type, stride, pointer);
}


void OpenGLState::ColorPointer(int size, GLenum type, int stride, const void* pointer)
// ----------------------------------------------------------------------------
//    Define an array of color data
// ----------------------------------------------------------------------------
{
    // Not need to be optimised
    glColorPointer(size, type, stride, pointer);
}


void OpenGLState::NewList(uint list, GLenum mode)
// ----------------------------------------------------------------------------
//    Create a display list
// ----------------------------------------------------------------------------
{
    // Not need to be optimised
    glNewList(list, mode);
}


void OpenGLState::EndList()
// ----------------------------------------------------------------------------
//    Replace a display list
// ----------------------------------------------------------------------------
{
    // Not need to be optimised
    glEndList();
}


uint OpenGLState::GenLists(uint range)
// ----------------------------------------------------------------------------
//    Generate a contiguous set of empty display lists
// ----------------------------------------------------------------------------
{
    // Not need to be optimised
    return glGenLists(range);
}


void OpenGLState::DeleteLists(uint list, uint range)
// ----------------------------------------------------------------------------
//    Delete a contiguous group of display lists
// ----------------------------------------------------------------------------
{
    // Not need to be optimised
    glDeleteLists(list, range);
}


void OpenGLState::CallList(uint list)
// ----------------------------------------------------------------------------
//    Execute a display list
// ----------------------------------------------------------------------------
{
    // Not need to be optimised
    glCallList(list);
}


void OpenGLState::CallLists(uint size, GLenum type, const void *pointer)
// ----------------------------------------------------------------------------
//    Execute a list of display lists
// ----------------------------------------------------------------------------
{
    // Not need to be optimised
    glCallLists(size, type, pointer);
}


void OpenGLState::ListBase(uint base)
// ----------------------------------------------------------------------------
//    Set the display-list base for glCallLists
// ----------------------------------------------------------------------------
{
    glListBase(base);
}


void OpenGLState::Bitmap(uint  width,  uint  height, coord  xorig,
                         coord  yorig,  coord  xmove, coord  ymove,
                         const uchar *  bitmap)
// ----------------------------------------------------------------------------
//    Draw a bitmap
// ----------------------------------------------------------------------------
{
    // Not need to be optimised
    glBitmap(width, height, xorig, yorig, xmove, ymove, bitmap);
}



// ============================================================================
//
//                       Attribute management functions.
//
// ============================================================================

void OpenGLState::Viewport(int x, int y, int w, int h)
// ----------------------------------------------------------------------------
//    Set the viewport
// ----------------------------------------------------------------------------
{
    // Do not need to setup viewport if it has not changed
    ViewportState newViewport(x, y, w, h);
    CHANGE(viewport, newViewport);
}


void OpenGLState::RasterPos(coord x, coord y, coord z, coord w)
// ----------------------------------------------------------------------------
//    Specify the raster position in window coordinates for pixel operations
// ----------------------------------------------------------------------------
{
    // Not optimised because depending of too much
    // settings (modelview and proj matrices, viewport, etc.) and
    // not often used in Tao.
    glRasterPos4d(x, y, z, w);
}


void OpenGLState::WindowPos(coord x, coord y, coord z, coord w)
// ----------------------------------------------------------------------------
//    Specify the raster position in window coordinates for pixel operations
// ----------------------------------------------------------------------------
{
    // Not optimised because depending of too much
    // settings (modelview and proj matrices, viewport, etc.) and
    // not often used in Tao.
    glWindowPos3d(x, y, z);
}


void OpenGLState::PixelStorei(GLenum pname,  int param)
// ----------------------------------------------------------------------------
//    Set pixel storage modes
// ----------------------------------------------------------------------------
{
    // Not need to be optimised
    glPixelStorei(pname, param);
}


void OpenGLState::PointSize(coord size)
// ----------------------------------------------------------------------------
//    Specify the diameter of rasterized pointsC Specification
// ----------------------------------------------------------------------------
{
    CHANGE(pointSize, size);
}


void OpenGLState::Color(float r, float g, float b, float a)
// ----------------------------------------------------------------------------
//    Setup color
// ----------------------------------------------------------------------------
{
    // Do not need to setup color if it has not changed
    Tao::Color c = Tao::Color(r, g, b, a);
    CHANGE(color, c);
}


void OpenGLState::Materialfv(GLenum face, GLenum pname, const GLfloat *val)
// ----------------------------------------------------------------------------
//    Change material parameters
// ----------------------------------------------------------------------------
{
    bool front = face == GL_FRONT || face == GL_FRONT_AND_BACK;
    bool back  = face == GL_BACK || face == GL_FRONT_AND_BACK;

    if (front || back)
    {
        switch(pname)
        {
        case GL_AMBIENT:
        {
            Tao::Color ambient(val[0], val[1], val[2], val[3]);
            if (front)  CHANGE(frontAmbient, ambient);
            if (back)   CHANGE(backAmbient, ambient);
            return;
        }
        case GL_DIFFUSE:
        {
            Tao::Color diffuse(val[0], val[1], val[2], val[3]);
            if (front)  CHANGE(frontDiffuse, diffuse);
            if (back)   CHANGE(backDiffuse, diffuse);
            return;
        }
        case GL_SPECULAR:
        {
            Tao::Color specular(val[0], val[1], val[2], val[3]);
            if (front)  CHANGE(frontSpecular, specular);
            if (back)   CHANGE(backSpecular, specular);
            return;
        }
        case GL_EMISSION:
        {
            Tao::Color emission(val[0], val[1], val[2], val[3]);
            if (front)  CHANGE(frontEmission, emission);
            if (back)   CHANGE(backEmission, emission);
            return;
        }
        case GL_SHININESS:
        {
            float shininess = val[0];
            if (front)  CHANGE(frontShininess, shininess);
            if (back)   CHANGE(backShininess, shininess);
            return;
        }

        default:
            // Other parameters fall through and are executed immediately
            break;
        }
    }

    // Default case: just apply it directly
    glMaterialfv(face, pname, val);
}



void OpenGLState::ClearColor(float r, float g, float b, float a)
// ----------------------------------------------------------------------------
//    Setup clear color
// ----------------------------------------------------------------------------
{
    // Do not need to setup clear color if it has not changed
    Tao::Color c = Tao::Color(r, g, b, a);
    CHANGE(clearColor, c);
}


void OpenGLState::Clear(GLuint mask)
// ----------------------------------------------------------------------------
//    Clear buffers to preset values
// ----------------------------------------------------------------------------
{
    Sync(STATE_clearColor);     // Make sure we have right clear color
    glClear(mask);              // Immediate execution
}


void OpenGLState::LineWidth(float width)
// ----------------------------------------------------------------------------
//    Specify the width of rasterized lines
// ----------------------------------------------------------------------------
{
    CHANGE(lineWidth, width);
}


void OpenGLState::LineStipple(GLint factor, GLushort pattern)
// ----------------------------------------------------------------------------
//    Specify the line stipple pattern
// ----------------------------------------------------------------------------
{
    LineStippleState newStipple(factor, pattern);
    CHANGE(stipple, newStipple);
}


void OpenGLState::CullFace(GLenum mode)
// ----------------------------------------------------------------------------
//    Specify whether front- or back-facing facets can be culled
// ----------------------------------------------------------------------------
{
    CHANGE(cullMode, mode);
}


void OpenGLState::DepthMask(GLboolean flag)
// ----------------------------------------------------------------------------
//    Enable or disable writing into the depth buffer
// ----------------------------------------------------------------------------
{
    CHANGE(depthMask, flag);
}


void OpenGLState::DepthFunc(GLenum func)
// ----------------------------------------------------------------------------
//    Specify the value used for depth buffer comparisons
// ----------------------------------------------------------------------------
{
    CHANGE(depthFunc, func);
}


void OpenGLState::ShadeModel(GLenum mode)
// ----------------------------------------------------------------------------
//    Select shading mode
// ----------------------------------------------------------------------------
{
    CHANGE(shadeMode, mode);
}


void OpenGLState::Hint(GLenum target, GLenum mode)
// ----------------------------------------------------------------------------
//   Specify implementation-specific hints
// ----------------------------------------------------------------------------
{
    switch(target)
    {
    case GL_PERSPECTIVE_CORRECTION_HINT:
        CHANGE(perspectiveCorrectionHint, mode);
        break;
    case GL_TEXTURE_COMPRESSION_HINT:
        CHANGE(textureCompressionHint, mode);
        break;
    default:   // Others hints are not used in Tao, still execute them
        glHint(target, mode);
        break;
    }
}


void OpenGLState::Enable(GLenum cap)
// ----------------------------------------------------------------------------
//    Enable capability
// ----------------------------------------------------------------------------
{
    switch(cap)
    {
#define GS(type, name)
#define GFLAG(name)                             \
    case name:                                  \
        CHANGE(glflag_##name, true);            \
        return;
#include "opengl_state.tbl"

    case GL_TEXTURE_1D:
    case GL_TEXTURE_2D:
    case GL_TEXTURE_3D:
    {
        TextureState &ts = ActiveTexture();
        ts.type = cap;
        ts.active = true;
        break;
    }

    default:
        // Other enable/disable operations are not cached
        glEnable(cap);
        break;
    }
}


void OpenGLState::Disable(GLenum cap)
// ----------------------------------------------------------------------------
//    Disable capability
// ----------------------------------------------------------------------------
{
    switch(cap)
    {
#define GS(type, name)
#define GFLAG(name) case name: CHANGE(glflag_##name, false); return;
#include "opengl_state.tbl"

    case GL_TEXTURE_1D:
    case GL_TEXTURE_2D:
    case GL_TEXTURE_3D:
    {
        TextureState &ts = ActiveTexture();
        ts.type = cap;
        ts.active = false;
        break;
    }

    default:
        // Other enable/disable operations are not cached
        glDisable(cap);
        break;
    }

}


void OpenGLState::BlendFunc(GLenum sfactor, GLenum dfactor)
// ----------------------------------------------------------------------------
//   Specify pixel arithmetic
// ----------------------------------------------------------------------------
{
    BlendFunctionState bfs(sfactor, dfactor, sfactor, dfactor);
    CHANGE(blendFunction, bfs);
}


void OpenGLState::BlendFuncSeparate(GLenum sRgb, GLenum dRgb,
                                    GLenum sAlpha, GLenum dAlpha)
// ----------------------------------------------------------------------------
//   Specify pixel arithmetic for RGB and alpha components separately
// ----------------------------------------------------------------------------
{
    BlendFunctionState bfs(sRgb, dRgb, sAlpha, dAlpha);
    CHANGE(blendFunction, bfs);
}


void OpenGLState::BlendEquation(GLenum mode)
// ----------------------------------------------------------------------------
//   Specify the equation used for RGB and Alpha blend equation
// ----------------------------------------------------------------------------
{
    CHANGE(blendEquation, mode);
}


void OpenGLState::AlphaFunc(GLenum func, float ref)
// ----------------------------------------------------------------------------
//   Specify the alpha test function
// ----------------------------------------------------------------------------
{
    AlphaFunctionState af(func, ref);
    CHANGE(alphaFunc, af);
}



// ============================================================================
//
//                       Selection functions.
//
// ============================================================================

int OpenGLState::RenderMode(GLenum mode)
// ----------------------------------------------------------------------------
//   Set rasterization mode
// ----------------------------------------------------------------------------
{
    CHANGE(renderMode, mode);
    if(renderMode_isDirty)
        return glRenderMode(mode);

    return 0;
}


void OpenGLState::SelectBuffer(int size, uint* buffer)
// ----------------------------------------------------------------------------
//   Establish a buffer for selection mode values
// ----------------------------------------------------------------------------
{
    // Not need to be optimised
    glSelectBuffer(size, buffer);
}


void OpenGLState::InitNames()
// ----------------------------------------------------------------------------
//   Initialize the name stack
// ----------------------------------------------------------------------------
{
    // Must be reimplemented ?
    glInitNames();
}


void OpenGLState::LoadName(uint name)
// ----------------------------------------------------------------------------
//   Load a name onto the name stack
// ----------------------------------------------------------------------------
{
    // Must be reimplemented ?
    glLoadName(name);
}


void OpenGLState::PushName(uint name)
// ----------------------------------------------------------------------------
//    Specifies a name that will be pushed onto the name stack.
// ----------------------------------------------------------------------------
{
    // Must be reimplemented ?
    glPushName(name);
}


void OpenGLState::PopName()
// ----------------------------------------------------------------------------
//    Pop the last name out the name stack.
// ----------------------------------------------------------------------------
{
    // Must be reimplemented ?
    glPopName();
}



// ============================================================================
//
//                       Shader management functions.
//
// ============================================================================

void OpenGLState::UseProgram(uint prg)
// ----------------------------------------------------------------------------
//   Set shader program
// ----------------------------------------------------------------------------
{
    CHANGE(shaderProgram, prg);
}


// ============================================================================
//
//                       Texture management functions.
//
// ============================================================================

void OpenGLState::ActiveTexture(GLenum active)
// ----------------------------------------------------------------------------
//   Set the currently active texture
// ----------------------------------------------------------------------------
{
    if (active >= GL_TEXTURE0 && active < GL_TEXTURE0 + maxTextureUnits)
        CHANGE(activeTexture, active - GL_TEXTURE0);
}


void OpenGLState::BindTexture(GLenum type, GLuint texture)
// ----------------------------------------------------------------------------
//   Bind a texture on the current texture unit
// ----------------------------------------------------------------------------
{
    TextureState &ts = ActiveTexture();
    ts.type = type;
    ts.id = texture;
}


void OpenGLState::TexParameteri(GLenum type, GLenum pname, GLint param)
// ----------------------------------------------------------------------------
//   Change parameters for a texture
// ----------------------------------------------------------------------------
{
    TextureState &ts = ActiveTexture();
    ts.type = type;
    switch(pname)
    {
    case GL_TEXTURE_MIN_FILTER:
        ts.minFilt = param;
        break;
    case GL_TEXTURE_MAG_FILTER:
        ts.magFilt = param;
        break;
    case GL_TEXTURE_WRAP_S:
        ts.wrapS = param == GL_REPEAT;
        break;
    case GL_TEXTURE_WRAP_T:
        ts.wrapT = param == GL_REPEAT;
        break;
    case GL_TEXTURE_WRAP_R:
        ts.wrapR = param == GL_REPEAT;
        break;
    default:
        Sync(STATE_textures);
        glTexParameteri(type, pname, param);
    }
}


void OpenGLState::TexEnvi(GLenum type, GLenum pname, GLint param)
// ----------------------------------------------------------------------------
//   Change environment for a texture
// ----------------------------------------------------------------------------
{
    if (type == GL_TEXTURE_ENV && pname == GL_TEXTURE_ENV_MODE)
    {
        TextureState &ts = ActiveTexture();
        ts.mode = param;
    }
    else
    {
        glTexEnvi(type, pname, param);
    }
}


void OpenGLState::TexImage2D(GLenum target, GLint level, GLint internalformat,
                             GLsizei width, GLsizei height, GLint border,
                             GLenum format, GLenum type,
                             const GLvoid *pixels )
// ----------------------------------------------------------------------------
//    Define an uncompressed image
// ----------------------------------------------------------------------------
{
    Sync(STATE_textures);
    glTexImage2D(target, level, internalformat, width, height, border,
                 format, type, pixels);
}


void OpenGLState::CompressedTexImage2D(GLenum target, GLint level,
                                       GLenum internalformat,
                                       GLsizei width, GLsizei height,
                                       GLint border, GLsizei imgSize,
                                       const GLvoid *data)
// ----------------------------------------------------------------------------
//    Define a compressed image
// ----------------------------------------------------------------------------
{
    Sync(STATE_textures);
    glCompressedTexImage2D(target, level, internalformat,
                           width, height, border, imgSize, data);
}


TextureState &OpenGLState::ActiveTexture()
// ----------------------------------------------------------------------------
//    Return the current active texture
// ----------------------------------------------------------------------------
{
    if (activeTexture >= textures.textures.size())
        textures.textures.resize(activeTexture + 1);
    textures.dirty |= 1ULL << activeTexture;
    SAVE(textures);
    textures_isDirty = true;
    return textures.textures[activeTexture];
}


// ============================================================================
// 
//    TextureState class
// 
// ============================================================================

TextureState::TextureState()
// ----------------------------------------------------------------------------
//   Initialize a texture state
// ----------------------------------------------------------------------------
  : type(GL_TEXTURE_2D), mode(GL_MODULATE),
    unit(0), id(0), width(0), height(0),
    minFilt(TextureCache::instance()->minFilter()),
    magFilt(TextureCache::instance()->magFilter()),
    matrix(),
    active(false),
    wrapS(false), wrapT(false), wrapR(false),
    mipmap(false)
{}


void TextureState::Sync(GLuint unit, const TextureState &ts, bool force)
// ----------------------------------------------------------------------------
//   Sync a texture state with a new value
// ----------------------------------------------------------------------------
{
    if (force || *this != ts)
        glActiveTexture(GL_TEXTURE0 + unit);

#define SYNC_TEXTURE(name, Code)                \
    do                                          \
    {                                           \
        if (force || name != ts.name)           \
        {                                       \
            name = ts.name;                     \
            Code;                               \
            OpenGLState::ShowErrors();          \
        }                                       \
    } while(0)

    if (force || type != ts.type || id != ts.id)
    {
        glBindTexture(ts.type, ts.id);
        type = ts.type;
        id = ts.id;
    }

    SYNC_TEXTURE(active,
                 if (active) glEnable(type); else glDisable(type));
    SYNC_TEXTURE(mode,
                 glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, mode));
    SYNC_TEXTURE(minFilt,
                 glTexParameteri (type, GL_TEXTURE_MIN_FILTER, minFilt));
    SYNC_TEXTURE(magFilt,
                 glTexParameteri (type, GL_TEXTURE_MAG_FILTER, magFilt));
    SYNC_TEXTURE(matrix,
                 glMatrixMode(GL_TEXTURE);
                 glLoadMatrixd(matrix.Data(false)));
    SYNC_TEXTURE(wrapS,
                 glTexParameteri(type, GL_TEXTURE_WRAP_S,
                                 wrapS ? GL_REPEAT : GL_CLAMP_TO_EDGE));
    SYNC_TEXTURE(wrapT,
                 glTexParameteri(type, GL_TEXTURE_WRAP_T,
                                 wrapT ? GL_REPEAT : GL_CLAMP_TO_EDGE));
    SYNC_TEXTURE(wrapR,
                 glTexParameteri(type, GL_TEXTURE_WRAP_R,
                                 wrapR ? GL_REPEAT : GL_CLAMP_TO_EDGE));

#undef SYNC_TEXTURE
}


TexturesState &TexturesState::operator=(const TexturesState &o)
// ----------------------------------------------------------------------------
//    Copy a texture state
// ----------------------------------------------------------------------------
{
    uint max = textures.size();
    uint omax = o.textures.size();
    ulonglong d = 0;

    // Compare all the texture states we write to mark changed ones as dirty
    uint tmax = max < omax ? max : omax;
    for (uint t = 0; t < tmax; t++)
    {
        if (textures[t] != o.textures[t])
            d |= 1ULL << t;
        textures[t] = o.textures[t];
    }

    // Check if number of textures changed
    if (omax < max)
    {
        // Number of textures diminished, truncate my textures to match
        textures.resize(omax);
    }
    else if (omax > max)
    {
        // Number of textures increased, append new textures and mark dirty
        for (uint t = max; t < omax; t++)
        {
            const TextureState &ts = o.textures[t];
            textures.push_back(ts);
            d |= 1ULL << t;
        }
    }

    dirty = d;
    return *this;
}


void TexturesState::Sync(TexturesState &nt)
// ----------------------------------------------------------------------------
//   Sync with a new texture state
// ----------------------------------------------------------------------------
{
    uint max = textures.size();
    uint nmax = nt.textures.size();
    ulonglong dirty = nt.dirty;

    // Quick bail out if nothing to do
    if (dirty == 0 && max == nmax)
        return;

    // Synchronize all textures that are common between the two states
    uint tmax = max < nmax ? max : nmax;
    for (uint t = 0; t < tmax; t++)
    {
        if (dirty & (1ULL<<t))
        {
            TextureState &ts = textures[t];
            TextureState &nts = nt.textures[t];
            ts.Sync(t, nts);
        }
    }

    // Check if number of textures changed
    if (nmax < max)
    {
        // Number of textures decreased, deactivate extra textures
        for (uint t = nmax; t < max; t++)
        {
            TextureState &ts = textures[t];
            glActiveTexture(GL_TEXTURE0 + t);
            glBindTexture(ts.type, 0);
            glDisable(ts.type);
        }
        textures.resize(nmax);
    }
    else if (nmax > max)
    {
        // Number of textures increased, activate extra textures
        for (uint t = max; t < nmax; t++)
        {
            TextureState &nts = nt.textures[t];
            textures.push_back(nts);
            TextureState &ts = textures.back();
            ts.Sync(t, nts, true);
        }
    }

    nt.dirty = 0;
}

TAO_END

