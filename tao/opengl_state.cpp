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
//  (C) 2010 Jérôme Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "opengl_state.h"
#include "opengl_save.h"
#include "application.h"
#include "texture_cache.h"
#include "tao_main.h"
#include "module_manager.h"
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
OpenGLState *OpenGLState::current = NULL;


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
            /* TextureMatrix() calls                    \
               ActiveTextureUnit() which saves          \
               the texture unit, matrix included. */    \
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

      vendorID(UNKNOWN),
      maxTextureCoords(4), maxTextureUnits(4), maxTextureSize(256),
      vendor("Uninitialized Bitblaster Corp."),
      renderer("Random Bits Pusher"),
      version("Unreleased Internal version pi"),
      extensionsAvailable("None"),

      matrixMode(GL_MODELVIEW),
      viewport(0, 0, 0, 0), listBase(0), pointSize(1),
      color(1,1,1,1), clearColor(0,0,0,1),
      frontAmbient(0.2,0.2,0.2,1.0), frontDiffuse(0.8,0.8,0.8,1.0),
      frontSpecular(0,0,0,1), frontEmission(0,0,0,1), frontShininess(0),
      backAmbient(0.2,0.2,0.2,1.0), backDiffuse(0.8,0.8,0.8,1.0),
      backSpecular(0,0,0,1), backEmission(0,0,0,1), backShininess(0),
      shadeMode(GL_SMOOTH), lineWidth(1),
      stipple(1, -1), cullMode(GL_BACK),
      depthMask(true), depthFunc(GL_LESS),
      textureCompressionHint(GL_DONT_CARE),
      perspectiveCorrectionHint(GL_DONT_CARE),
      blendFunction(GL_ONE, GL_ZERO, GL_ONE, GL_ZERO),
      blendEquation(GL_FUNC_ADD), alphaFunc(GL_ALWAYS, 0.0),
      renderMode(GL_RENDER), shaderProgram(0),
      activeTexture(GL_TEXTURE0), clientActiveTexture(GL_TEXTURE0),

#define GS(type, name)
#define GFLAG(name)             glflag_##name(false),
#define GCLIENTSTATE(name)      glclientstate_##name(false),
#include "opengl_state.tbl"
      save(NULL)
{}


void OpenGLState::MakeCurrent()
// ----------------------------------------------------------------------------
//   Make this object the current OpenGLState
// ----------------------------------------------------------------------------
{
    current = this;
    ModuleManager::moduleManager()->updateGraphicStatePointers(current);

    // Ask graphic card constructor to OpenGL
    vendor = text ((const char*) glGetString (GL_VENDOR));

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
    glGetIntegerv(GL_MAX_TEXTURE_SIZE,(GLint*) &maxTextureSize);

    // We never use single buffered contexts, so the OpenGL default is
    // always GL_BACK
    bufferMode = GL_BACK;

    if (maxTextureUnits > MAX_TEXTURE_UNITS)
        maxTextureUnits = MAX_TEXTURE_UNITS;

    if (maxTextureCoords > MAX_TEXTURE_COORDS)
        maxTextureCoords = MAX_TEXTURE_COORDS;
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


uint OpenGLState::ShowErrors(kstring msg)
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
            std::cerr << "GL Error: " << p;
            if (msg)
                std::cerr << " (" << msg << ") ";
            std::cerr << "[error code: " << err << "]\n";
            IFTRACE(glerrors)
                tao_stack_trace(2);
            last = err;
        }
        if (err != last || count == 100)
        {
            std::cerr << "GL Error: Error " << last;
            if (msg)
                std::cerr << " in " << msg << " ";
            std::cerr << " repeated " << count << " times\n";
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


void OpenGLState::Sync(uint64 which)
// ----------------------------------------------------------------------------
//    Synchronize pending changes and send them to the card
// ----------------------------------------------------------------------------
{
    bool needSync =
#define GS(type, name) (name##_isDirty && (which & STATE_##name)) ||
#include "opengl_state.tbl"
        false;
    if (!needSync)
        return;

    bool traceErrors = XLTRACE(glerrors);
#define SYNC(name, Code)                                \
    do                                                  \
    {                                                   \
        if (name##_isDirty && (which & STATE_##name))   \
        {                                               \
            if (traceErrors)                            \
                ShowErrors("before " #name);            \
            Code;                                       \
            name##_isDirty = false;                     \
            if (traceErrors)                            \
                ShowErrors("after " #name);             \
        }                                               \
    } while(0)

    // Current texture unit, then texture bindings - ORDER MATTERS
    SYNC(textureUnits, currentTextureUnits.Sync(textureUnits, activeTexture));
    SYNC(textures,
         currentTextures.Sync(textures, ActiveTextureUnit());
         matrixMode_isDirty = true /* Pessimistic: we don't know if set */);
    SYNC(clientTextureUnits, currentClientTextureUnits.Sync(clientTextureUnits, clientActiveTexture));
    GLenum mmode = matrixMode;
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
         if (glflag_##name)                     \
             glEnable(name);                    \
         else                                   \
             glDisable(name));
#define GCLIENTSTATE(name)                      \
    SYNC(glclientstate_##name,                  \
         if (glclientstate_##name)              \
             glEnableClientState(name);         \
         else                                   \
             glDisableClientState(name));
#include "opengl_state.tbl"

    SYNC(lights,
         currentLights.Sync(lights));
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

#undef SYNC
}


void OpenGLState::Invalidate(uint64 which)
// ----------------------------------------------------------------------------
//   Invalidate selected elements, e.g. because of known direct calls
// ----------------------------------------------------------------------------
{
#define GS(type, name) if (which & STATE_##name) name##_isDirty = true;
#include "opengl_state.tbl"
}



// ============================================================================
//
//                        State query functions
//
// ============================================================================

void OpenGLState::Get(GLenum pname, GLboolean * params)
// ----------------------------------------------------------------------------
//    Return the value or values of a selected parameter
// ----------------------------------------------------------------------------
{
    // REVISIT read known states from cache
    Sync();
    glGetBooleanv(pname, params);
}

void OpenGLState::Get(GLenum pname, GLfloat * params)
// ----------------------------------------------------------------------------
//    Return the value or values of a selected parameter
// ----------------------------------------------------------------------------
{
    // REVISIT read known states from cache
    Sync();
    glGetFloatv(pname, params);
}

void OpenGLState::Get(GLenum pname, GLint * params)
// ----------------------------------------------------------------------------
//    Return the value or values of a selected parameter
// ----------------------------------------------------------------------------
{
    // REVISIT read known states from cache
    Sync();
    glGetIntegerv(pname, params);
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
    Sync();
    glBegin(mode);
}


void OpenGLState::End()
// ----------------------------------------------------------------------------
//    Delimit the vertices of a primitive or a group of like primitives
// ----------------------------------------------------------------------------
{
    glEnd();
}


void OpenGLState::Vertex(coord x, coord y, coord z, coord w)
// ----------------------------------------------------------------------------
//    Specify a vertex within Begin/End
// ----------------------------------------------------------------------------
{
    glVertex4d(x, y, z, w);
}


void OpenGLState::Vertex3v(const coord *array)
// ----------------------------------------------------------------------------
//    Specify a set of vertices
// ----------------------------------------------------------------------------
{
    glVertex3dv(array);
}


void OpenGLState::Normal(coord nx, coord ny, coord nz)
// ----------------------------------------------------------------------------
//    Specify a normal within Begin/End
// ----------------------------------------------------------------------------
{
    glNormal3d(nx, ny, nz);
}


void OpenGLState::TexCoord(coord s, coord t)
// ----------------------------------------------------------------------------
//    Specify a texture coordinate within Begin/End
// ----------------------------------------------------------------------------
{
    glTexCoord2d(s, t);
}


void OpenGLState::MultiTexCoord3v(GLenum target, const coord *array)
// ----------------------------------------------------------------------------
//    Specify the coordinate of a texture unit
// ----------------------------------------------------------------------------
{
    glMultiTexCoord3dv(target, array);
}


void OpenGLState::EnableClientState(GLenum cap)
// ----------------------------------------------------------------------------
//    Enable client-side capability
// ----------------------------------------------------------------------------
{
    switch(cap)
    {
#define GS(type, name)
#define GCLIENTSTATE(name)                      \
    case name:                                  \
        CHANGE(glclientstate_##name, true);     \
        return;
#include "opengl_state.tbl"
    case GL_TEXTURE_COORD_ARRAY:
    {
        ClientTextureUnitState &cs = ClientActiveTexture();
        cs.Set(cap, true);
        break;
    }
    default:
        // Other enable/disable operations are not cached
        glEnableClientState(cap);
        break;
    }
}


void OpenGLState::DisableClientState(GLenum cap)
// ----------------------------------------------------------------------------
//    Disable client-side capability
// ----------------------------------------------------------------------------
{
    switch(cap)
    {
#define GS(type, name)
#define GCLIENTSTATE(name)                      \
    case name:                                  \
        CHANGE(glclientstate_##name, false);    \
        return;
#include "opengl_state.tbl"
    case GL_TEXTURE_COORD_ARRAY:
    {
        ClientTextureUnitState &cs = ClientActiveTexture();
        cs.Set(cap, false);
        break;
    }
    default:
        // Other enable/disable operations are not cached
        glDisableClientState(cap);
        break;
    }
}


void OpenGLState::ClientActiveTexture(GLenum tex)
// ----------------------------------------------------------------------------
//   Activate a given client active texture
// ----------------------------------------------------------------------------
{
    if(tex >= GL_TEXTURE0 && tex < GL_TEXTURE0 + maxTextureCoords)
        CHANGE(clientActiveTexture, tex);
}


ClientTextureUnitState &OpenGLState::ClientActiveTexture()
// ----------------------------------------------------------------------------
//   Return current client active texture state
// ----------------------------------------------------------------------------
{
     Q_ASSERT(clientActiveTexture >= GL_TEXTURE0);
     Q_ASSERT(clientActiveTexture < GL_TEXTURE0+MAX_TEXTURE_COORDS);
     uint at = clientActiveTexture - GL_TEXTURE0;
     if (at >= clientTextureUnits.units.size())
         clientTextureUnits.units.resize(at + 1);
     clientTextureUnits.dirty |=  1ULL << at;

     // Save clint texture unit state on the state stack
     clientTextureUnits_isDirty = true;
     if (save)
         save->save_clientTextureUnits(clientTextureUnits);

     return clientTextureUnits.units[at];
}


void OpenGLState::DrawArrays(GLenum mode, int first, int count)
// ----------------------------------------------------------------------------
//    Render primitives from array data
// ----------------------------------------------------------------------------
{
    Sync();
    glDrawArrays(mode, first, count);
}


void OpenGLState::VertexPointer(int size, GLenum type,
                                int stride, const void* pointer)
// ----------------------------------------------------------------------------
//    Define an array of vertex data
// ----------------------------------------------------------------------------
{
    Sync();
    glVertexPointer(size, type, stride, pointer);
}


void OpenGLState::NormalPointer(GLenum type, int stride, const void* pointer)
// ----------------------------------------------------------------------------
//    Define an array of normal data
// ----------------------------------------------------------------------------
{
    Sync();
    glNormalPointer(type, stride, pointer);
}


void OpenGLState::TexCoordPointer(int size, GLenum type,
                                  int stride, const void* pointer)
// ----------------------------------------------------------------------------
//    Define an array of texture coordinates
// ----------------------------------------------------------------------------
{
    Sync();
    glTexCoordPointer(size, type, stride, pointer);
}


void OpenGLState::ColorPointer(int size, GLenum type,
                               int stride, const void* pointer)
// ----------------------------------------------------------------------------
//    Define an array of color data
// ----------------------------------------------------------------------------
{
    Sync();
    glColorPointer(size, type, stride, pointer);
}


void OpenGLState::NewList(uint list, GLenum mode)
// ----------------------------------------------------------------------------
//    Create a display list
// ----------------------------------------------------------------------------
{
    // Avoid storing operations flushed from the cache into the new list
    Sync();
    glNewList(list, mode);
}


void OpenGLState::EndList()
// ----------------------------------------------------------------------------
//    Replace a display list
// ----------------------------------------------------------------------------
{
    Sync();
    glEndList();
}


uint OpenGLState::GenLists(uint range)
// ----------------------------------------------------------------------------
//    Generate a contiguous set of empty display lists
// ----------------------------------------------------------------------------
{
    return glGenLists(range);
}


void OpenGLState::DeleteLists(uint list, uint range)
// ----------------------------------------------------------------------------
//    Delete a contiguous group of display lists
// ----------------------------------------------------------------------------
{
    glDeleteLists(list, range);
}


void OpenGLState::CallList(uint list)
// ----------------------------------------------------------------------------
//    Execute a display list
// ----------------------------------------------------------------------------
{
    Sync();
    glCallList(list);
}


void OpenGLState::CallLists(uint size, GLenum type, const void *pointer)
// ----------------------------------------------------------------------------
//    Execute a list of display lists
// ----------------------------------------------------------------------------
{
    Sync();
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
    glBitmap(width, height, xorig, yorig, xmove, ymove, bitmap);
}


void OpenGLState::DrawPixels(GLsizei width, GLsizei height, GLenum format,
                             GLenum type, const GLvoid *data)
// ----------------------------------------------------------------------------
//   Write a block of pixels to the frame buffer
// ----------------------------------------------------------------------------
{
    Sync();
    glDrawPixels(width, height, format, type, data);
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
    Sync();
    glRasterPos4d(x, y, z, w);
}


void OpenGLState::WindowPos(coord x, coord y, coord z, coord w)
// ----------------------------------------------------------------------------
//    Specify the raster position in window coordinates for pixel operations
// ----------------------------------------------------------------------------
{
    (void) w;

    // Not optimised because depending of too much
    // settings (modelview and proj matrices, viewport, etc.) and
    // not often used in Tao.
    Sync();
    glWindowPos3d(x, y, z);
}


void OpenGLState::PixelStorei(GLenum pname,  int param)
// ----------------------------------------------------------------------------
//    Set pixel storage modes
// ----------------------------------------------------------------------------
{
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
    case GL_TEXTURE_CUBE_MAP:
    {
        TextureUnitState &st = ActiveTextureUnit();
        st.Set(cap, true);
        break;
    }
    case GL_LIGHT0:
    case GL_LIGHT1:
    case GL_LIGHT2:
    case GL_LIGHT3:
    case GL_LIGHT4:
    case GL_LIGHT5:
    case GL_LIGHT6:
    case GL_LIGHT7:
    {
        SetLight(cap, true);
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
    case GL_TEXTURE_CUBE_MAP:
    {
        TextureUnitState &st = ActiveTextureUnit();
        st.Set(cap, false);
        break;
    }
    case GL_LIGHT0:
    case GL_LIGHT1:
    case GL_LIGHT2:
    case GL_LIGHT3:
    case GL_LIGHT4:
    case GL_LIGHT5:
    case GL_LIGHT6:
    case GL_LIGHT7:
    {
        SetLight(cap, false);
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


#define SHADER(body)                            \
{                                               \
    bool traceErrors = XLTRACE(glerrors);       \
    if (traceErrors)                            \
        ShowErrors("before " #body);            \
    Sync(STATE_shaderProgram);                  \
    body;                                       \
    if (traceErrors)                            \
        ShowErrors("after " #body);             \
}


void OpenGLState::GetProgram(uint prg, GLenum pname, int *params)
// ----------------------------------------------------------------------------
//   Returns a parameter from a program object
// ----------------------------------------------------------------------------
{
    SHADER(glGetProgramiv(prg, pname, params));
}


void OpenGLState::GetActiveUniform(uint prg, uint id,
                                   uint bufSize, GLsizei *length,
                                   GLsizei* size, GLenum *type, char* name)
// ----------------------------------------------------------------------------
//   Returns information about an active uniform variable
// ----------------------------------------------------------------------------
{
    SHADER(glGetActiveUniform(prg, id, bufSize, length, size, type, name));
}


int OpenGLState::GetAttribLocation(uint program, const char* name)
// ----------------------------------------------------------------------------
//   Returns the location of an attribute variable
// ----------------------------------------------------------------------------
{
    Sync(STATE_shaderProgram);
    return glGetAttribLocation(program, name);
}


int OpenGLState::GetUniformLocation(uint program, const char* name)
// ----------------------------------------------------------------------------
//   Returns the location of a uniform variable
// ----------------------------------------------------------------------------
{
    Sync(STATE_shaderProgram);
    return glGetUniformLocation(program, name);
}


void OpenGLState::VertexAttrib1fv(uint id, const float *v)
// ----------------------------------------------------------------------------
//   Vertex attributes
// ----------------------------------------------------------------------------
{
    SHADER(glVertexAttrib1fv(id, v));
}


void OpenGLState::VertexAttrib2fv(uint id, const float *v)
// ----------------------------------------------------------------------------
//   Vertex attributes
// ----------------------------------------------------------------------------
{
    SHADER(glVertexAttrib2fv(id, v));
}


void OpenGLState::VertexAttrib3fv(uint id, const float *v)
// ----------------------------------------------------------------------------
//   Vertex attributes
// ----------------------------------------------------------------------------
{
    SHADER(glVertexAttrib3fv(id, v));
}


void OpenGLState::VertexAttrib4fv(uint id, const float *v)
// ----------------------------------------------------------------------------
//   Vertex attributes
// ----------------------------------------------------------------------------
{
    SHADER(glVertexAttrib4fv(id, v));
}


void OpenGLState::Uniform(uint id, float v)
// ----------------------------------------------------------------------------
//   Setting uniform values
// ----------------------------------------------------------------------------
{
    SHADER(glUniform1f(id, v));
}


void OpenGLState::Uniform(uint id, float v0, float v1)
// ----------------------------------------------------------------------------
//    Setting uniform values
// ----------------------------------------------------------------------------
{
    SHADER(glUniform2f(id, v0, v1));
}


void OpenGLState::Uniform(uint id, float v0, float v1, float v2)
// ----------------------------------------------------------------------------
//    Setting uniform values
// ----------------------------------------------------------------------------
{
    SHADER(glUniform3f(id, v0, v1, v2));
}


void OpenGLState::Uniform(uint id, float v0, float v1, float v2, float v3)
// ----------------------------------------------------------------------------
//    Setting uniform values
// ----------------------------------------------------------------------------
{
    SHADER(glUniform4f(id, v0, v1, v2, v3));
}


void OpenGLState::Uniform(uint id, int v)
// ----------------------------------------------------------------------------
//    Setting uniform values
// ----------------------------------------------------------------------------
{
    SHADER(glUniform1i(id, v));
}


void OpenGLState::Uniform(uint id, int v0, int v1)
// ----------------------------------------------------------------------------
//    Setting uniform values
// ----------------------------------------------------------------------------
{
    SHADER(glUniform2i(id, v0, v1));
}


void OpenGLState::Uniform(uint id, int v0, int v1, int v2)
// ----------------------------------------------------------------------------
//    Setting uniform values
// ----------------------------------------------------------------------------
{
    SHADER(glUniform3i(id, v0, v1, v2));
}


void OpenGLState::Uniform(uint id, int v0, int v1, int v2, int v3)
// ----------------------------------------------------------------------------
//    Setting uniform values
// ----------------------------------------------------------------------------
{
    SHADER(glUniform4i(id, v0, v1, v2, v3));
}


void OpenGLState::Uniform1fv(uint id, GLsizei size, const float* v)
// ----------------------------------------------------------------------------
//    Setting uniform values
// ----------------------------------------------------------------------------
{
    SHADER(glUniform1fv(id, size, v));
}


void OpenGLState::Uniform2fv(uint id, GLsizei size, const float* v)
// ----------------------------------------------------------------------------
//    Setting uniform values
// ----------------------------------------------------------------------------
{
    SHADER(glUniform2fv(id, size, v));
}


void OpenGLState::Uniform3fv(uint id, GLsizei size, const float* v)
// ----------------------------------------------------------------------------
//    Setting uniform values
// ----------------------------------------------------------------------------
{
    SHADER(glUniform3fv(id, size, v));
}


void OpenGLState::Uniform4fv(uint id, GLsizei size, const float* v)
// ----------------------------------------------------------------------------
//    Setting uniform values
// ----------------------------------------------------------------------------
{
    SHADER(glUniform4fv(id, size, v));
}


void OpenGLState::Uniform1iv(uint id, GLsizei size, const int* v)
// ----------------------------------------------------------------------------
//    Setting uniform values
// ----------------------------------------------------------------------------
{
    SHADER(glUniform1iv(id, size, v));
}


void OpenGLState::Uniform2iv(uint id, GLsizei size, const int* v)
// ----------------------------------------------------------------------------
//    Setting uniform values
// ----------------------------------------------------------------------------
{
    SHADER(glUniform2iv(id, size, v));
}


void OpenGLState::Uniform3iv(uint id, GLsizei size, const int* v)
// ----------------------------------------------------------------------------
//    Setting uniform values
// ----------------------------------------------------------------------------
{
    SHADER(glUniform3iv(id, size, v));
}


void OpenGLState::Uniform4iv(uint id, GLsizei size, const int* v)
// ----------------------------------------------------------------------------
//    Setting uniform values
// ----------------------------------------------------------------------------
{
    SHADER(glUniform4iv(id, size, v));
}


void OpenGLState::UniformMatrix2fv(uint id, GLsizei size,
                                  bool transp, const float* m)
// ----------------------------------------------------------------------------
//    Setting uniform values
// ----------------------------------------------------------------------------
{
    SHADER(glUniformMatrix2fv(id, size, transp, m));
}


void OpenGLState::UniformMatrix3fv(uint id, GLsizei size,
                                  bool transp, const float* m)
// ----------------------------------------------------------------------------
//    Setting uniform values
// ----------------------------------------------------------------------------
{
    SHADER(glUniformMatrix3fv(id, size, transp, m));
}


void OpenGLState::UniformMatrix4fv(uint id, GLsizei size,
                                  bool transp, const float* m)
// ----------------------------------------------------------------------------
//    Setting uniform values
// ----------------------------------------------------------------------------
{
    SHADER(glUniformMatrix4fv(id, size, transp, m));
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
        CHANGE(activeTexture, active);
}


TextureState &OpenGLState::ActiveTexture()
// ----------------------------------------------------------------------------
//    Return the current active texture
// ----------------------------------------------------------------------------
{
    GLuint bound = ActiveTextureUnit().texture;
    TextureState &st = textures.textures[bound];
    textures.dirty.insert(bound);

    // Mark texture as dirty and push changes on the save stack
    textures_isDirty = true;
    if (save)
        save->save_textures(st);

    return st;
}


TextureUnitState &OpenGLState::ActiveTextureUnit()
// ----------------------------------------------------------------------------
//    Return the state of the active texture unit
// ----------------------------------------------------------------------------
{
    Q_ASSERT(activeTexture >= GL_TEXTURE0);
    Q_ASSERT(activeTexture < GL_TEXTURE0+MAX_TEXTURE_UNITS);
    uint at = activeTexture - GL_TEXTURE0;
    if (at >= textureUnits.units.size())
        textureUnits.units.resize(at + 1);
    textureUnits.dirty |=  1ULL << at;

    // Save texture unit state on the state stack
    textureUnits_isDirty = true;
    if (save)
        save->save_textureUnits(textureUnits);

    return textureUnits.units[at];
}


void OpenGLState::GenTextures(uint n, GLuint *textures)
// ----------------------------------------------------------------------------
//   Generate texture names
// ----------------------------------------------------------------------------
{
    glGenTextures(n, textures);
}


void OpenGLState::DeleteTextures(uint n, GLuint *  textures)
// ----------------------------------------------------------------------------
//   Delete named textures
// ----------------------------------------------------------------------------
{
    glDeleteTextures(n, textures);
}


void OpenGLState::BindTexture(GLenum type, GLuint texture)
// ----------------------------------------------------------------------------
//   Bind a texture on the current texture unit
// ----------------------------------------------------------------------------
{
    // Bind the texture to the current texture unit
    TextureUnitState &tunit = ActiveTextureUnit();
    tunit.texture = texture;
    tunit.target = type;

    // Mark texture as dirty and push changes on the save stack
    TextureState &ts = textures.textures[texture];
    textures.dirty.insert(texture);
    textures_isDirty = true;
    if (save)
        save->save_textures(ts);

    // Change texture state once we saved the old one
    ts.type = type;
    ts.id = texture;
}


void OpenGLState::TexParameter(GLenum type, GLenum pname, GLint param)
// ----------------------------------------------------------------------------
//   Change parameters for a texture
// ----------------------------------------------------------------------------
{
    TextureState &ts = ActiveTexture();
    Q_ASSERT (ts.type == type);
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
        Sync(STATE_textures | STATE_textureUnits | STATE_activeTexture);
        glTexParameteri(type, pname, param);
    }
}


void OpenGLState::TexEnv(GLenum type, GLenum pname, GLint param)
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
        Sync(STATE_textures | STATE_textureUnits | STATE_activeTexture);
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
    Sync(STATE_textures | STATE_textureUnits | STATE_activeTexture);
    glTexImage2D(target, level, internalformat, width, height, border,
                 format, type, pixels);
    TextureSize(width, height);
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
    Sync(STATE_textures | STATE_textureUnits | STATE_activeTexture);
    glCompressedTexImage2D(target, level, internalformat,
                           width, height, border, imgSize, data);
    TextureSize(width, height);
}


void OpenGLState::TextureSize(uint width, uint height)
// ----------------------------------------------------------------------------
//   Set the dimension of the given texture
// ----------------------------------------------------------------------------
{
    TextureState &ts = ActiveTexture();
    ts.width = width;
    ts.height = height;
}


uint OpenGLState::TextureWidth()
// ----------------------------------------------------------------------------
//   Get the width of the current texture
// ----------------------------------------------------------------------------
{
    TextureState &ts = ActiveTexture();
    return ts.width;
}


uint OpenGLState::TextureHeight()
// ----------------------------------------------------------------------------
//   Get the height of the current texture
// ----------------------------------------------------------------------------
{
    TextureState &ts = ActiveTexture();
    return ts.height;
}


uint OpenGLState::TextureType()
// ----------------------------------------------------------------------------
//   Get the type of the current texture
// ----------------------------------------------------------------------------
{
    TextureState &ts = ActiveTexture();
    return ts.type;
}


uint OpenGLState::TextureMode()
// ----------------------------------------------------------------------------
//   Get the mode of the current texture
// ----------------------------------------------------------------------------
{
    TextureState &ts = ActiveTexture();
    return ts.mode;
}


uint OpenGLState::TextureID()
// ----------------------------------------------------------------------------
//   Get the ID of the current texture
// ----------------------------------------------------------------------------
{
    TextureState &ts = ActiveTexture();
    return ts.id;
}


void OpenGLState::ActivateTextureUnits(uint64 mask)
// ----------------------------------------------------------------------------
//   Activate textures marked in the mask
// ----------------------------------------------------------------------------
{
    uint64 todo = mask ^ currentTextureUnits.active;
    uint unit = 0;
    while(todo)
    {
        if (todo & (1ULL << unit))
        {
            Q_ASSERT(unit < currentTextureUnits.units.size());
            TextureUnitState &tus = currentTextureUnits.units[unit];
            tus.Set(tus.target, (mask & (1ULL << unit)) ? true : false);
            todo &= ~(1ULL << unit);
        }
        unit++;
    }
}


uint OpenGLState::ActiveTextureUnitIndex()
// ----------------------------------------------------------------------------
//    Return the ID of the current active texture
// ----------------------------------------------------------------------------
{
    return activeTexture;
}


uint OpenGLState::ActiveTextureUnitsCount()
// ----------------------------------------------------------------------------
//   Return the number of bound and active textures in current state
// ----------------------------------------------------------------------------
{
    // OK, it's a bit dirty to use a GCC-specific thing, but C ought to have it
    return __builtin_popcount(currentTextureUnits.active);
}


uint64 OpenGLState::ActiveTextureUnits()
// ----------------------------------------------------------------------------
//   Return the mask of active textures in the current state     
// ----------------------------------------------------------------------------
{
    return currentTextureUnits.active;
}



// ============================================================================
//
//                       Lighting management functions.
//
// ============================================================================

void OpenGLState::SetLight(GLenum light, bool active)
// ----------------------------------------------------------------------------
//   Enable or disable a light
// ----------------------------------------------------------------------------
{
    if (light >= GL_LIGHT0 && light < GL_LIGHT0 + MAX_LIGHTS)
    {
        uint id = light - GL_LIGHT0;
        if (id >= lights.lights.size())
        {
            lights.lights.resize(id + 1);
            (lights.lights[id]) = LightState(id);
        }

        lights.dirty |= 1ULL << id;
        SAVE(lights);
        lights_isDirty = true;
        (lights.lights[id]).active = active;
    }
}


void OpenGLState::Light(GLenum light, GLenum pname, const float* params)
// ----------------------------------------------------------------------------
//   Initialize a light state
// ----------------------------------------------------------------------------
{
    // Save current light and set it as dirty
    uint id = light - GL_LIGHT0;
    LightState &ls = lights.lights[id];
    lights.dirty |= 1ULL << id;
    SAVE(lights);
    lights_isDirty = true;

    switch(pname)
    {
    case GL_AMBIENT:
    {
        Tao::Color ambient(params[0], params[1], params[2], params[3]);
        ls.ambient = ambient;
        break;
    }
    case GL_DIFFUSE:
    {
        Tao::Color diffuse(params[0], params[1], params[2], params[3]);
        ls.diffuse = diffuse;
        break;
    }
    case GL_SPECULAR:
    {
        Tao::Color specular(params[0], params[1], params[2], params[3]);
        ls.specular = specular;
        break;
    }
    case GL_POSITION:
    {
        ls.position = Vector4(params[0], params[1], params[2], params[3]);
        break;
    }
    case GL_SPOT_DIRECTION:
    {
        ls.spotDirection = Vector3(params[0], params[1], params[2]);
        break;
    }
    case GL_SPOT_EXPONENT:
        ls.spotExponent = (*params);
        break;
    case GL_SPOT_CUTOFF:
        ls.spotCutoff = (*params);
        break;
    case GL_CONSTANT_ATTENUATION:
        ls.constantAttenuation = (*params);
        break;
    case GL_LINEAR_ATTENUATION:
        ls.linearAttenuation = (*params);
        break;
    case GL_QUADRATIC_ATTENUATION:
        ls.quadricAttenuation = (*params);
        break;
    default:
        glLightfv(light, pname, params);
    }
}

// ============================================================================
//
//    LightState class
//
// ============================================================================

LightState::LightState(uint id)
// ----------------------------------------------------------------------------
//   Initialize a light state
// ----------------------------------------------------------------------------
    : id(id),
      position(0, 0, 1, 0), spotDirection(0, 0, -1),
      spotExponent(0), spotCutoff(180),
      constantAttenuation(1.0), linearAttenuation(0),
      quadricAttenuation(0), active(false)
{
    // Setup initial colors
    // Note: According to OpenGL, diffuse and
    // specular color is white for light 0
    // and black for others.
    ambient = Tao::Color(0, 0, 0, 1);
    if(! id)
        diffuse = specular = Tao::Color(1, 1, 1, 1);
    else
        diffuse = specular = Tao::Color(0, 0, 0, 1);

    // Setup initial position
    position = Vector4(0, 0, 1, 0);

    // Setup initial spot direction
    spotDirection = Vector3(0, 0, -1);
}


void LightState::Sync(const LightState &ls, bool force)
// ----------------------------------------------------------------------------
//   Sync a light state with a new value
// ----------------------------------------------------------------------------
{
    bool traceErrors = XLTRACE(glerrors);

#define SYNC_LIGHT(name, Code)                          \
            do                                          \
            {                                           \
                if (force || name != ls.name)           \
                {                                       \
                    name = ls.name;                     \
                    Code;                               \
                    if (traceErrors)                    \
                        OpenGLState::ShowErrors(#name); \
                }                                       \
            } while(0)

    if (force || id != ls.id)
        id = ls.id;

    GLenum light    = GL_LIGHT0 + id;
    GLfloat pos[4]  = {ls.position.x, ls.position.y, ls.position.z, ls.position.w};
    GLfloat spot[3] = {ls.spotDirection.x, ls.spotDirection.y, ls.spotDirection.z};

    // Synchronise lighting parameters
    SYNC_LIGHT(active,
        if (active) glEnable(light); else glDisable(light));
    SYNC_LIGHT(ambient,
        glLightfv(light, GL_AMBIENT, ambient.Data()));
    SYNC_LIGHT(diffuse,
        glLightfv(light, GL_DIFFUSE, diffuse.Data()));
    SYNC_LIGHT(specular,
        glLightfv(light, GL_SPECULAR, specular.Data()));
    SYNC_LIGHT(position,
        glLightfv(light, GL_POSITION, pos));
    SYNC_LIGHT(spotDirection,
            glLightfv(light, GL_SPOT_DIRECTION, spot));
    SYNC_LIGHT(spotExponent,
        glLightfv(light, GL_SPOT_EXPONENT, &spotExponent));
    SYNC_LIGHT(spotCutoff,
        glLightfv(light, GL_SPOT_CUTOFF, &spotCutoff));
    SYNC_LIGHT(constantAttenuation,
        glLightfv(light, GL_CONSTANT_ATTENUATION, &constantAttenuation));
    SYNC_LIGHT(linearAttenuation,
        glLightfv(light, GL_LINEAR_ATTENUATION, &linearAttenuation));
    SYNC_LIGHT(quadricAttenuation,
        glLightfv(light, GL_QUADRATIC_ATTENUATION, &quadricAttenuation));

#undef SYNC_LIGHT
}


LightsState &LightsState::operator=(const LightsState &o)
// ----------------------------------------------------------------------------
//    Copy a lighting state
// ----------------------------------------------------------------------------
{
    uint max = lights.size();
    uint omax = o.lights.size();
    uint64 d = 0;

    // Compare all the light states we write to mark changed ones as dirty
    uint lmax = max < omax ? max : omax;
    for (uint l = 0; l < lmax; l++)
    {
        if (lights[l] != o.lights[l])
            d |= 1ULL << l;
        lights[l] = o.lights[l];
    }

    // Check if number of lights changed
    if (omax < max)
    {
        // Number of lights diminished, truncate my lights to match
        lights.resize(omax);
    }
    else if (omax > max)
    {
        // Number of lights increased, append new lights and mark dirty
        for (uint l = max; l < omax; l++)
        {
            const LightState &ls = o.lights[l];
            lights.push_back(ls);
            d |= 1ULL << l;
        }
    }

    dirty = d;
    return *this;
}


void LightsState::Sync(LightsState &nl)
// ----------------------------------------------------------------------------
//   Sync with new light states
// ----------------------------------------------------------------------------
{
    uint max = lights.size();
    uint nmax = nl.lights.size();
    uint64 dirty = nl.dirty;

    // Quick bail out if nothing to do
    if (dirty == 0 && max == nmax)
        return;

    // Synchronize all lights that are common between the two states
    uint lmax = max < nmax ? max : nmax;
    for (uint l = 0; l < lmax; l++)
    {
        if (dirty & (1ULL<<l))
        {
            LightState &ls = lights[l];
            LightState &nls = nl.lights[l];
            ls.Sync(nls);
        }
    }

    // Check if number of lights changed
    if (nmax < max)
    {
        // Number of lights decreased, deactivate extra lights
        for (uint l = nmax; l < max; l++)
            glDisable(GL_LIGHT0 + l);
        lights.resize(nmax);
    }
    else if (nmax > max)
    {
        // Number of lights increased, activate extra lights
        for (uint l = max; l < nmax; l++)
        {
            LightState &nls = nl.lights[l];
            lights.push_back(nls);
            LightState &ls = lights.back();
            ls.Sync(nls, true);
        }
    }

    nl.dirty = 0;
}



// ============================================================================
//
//    Texture units
//
// ============================================================================

void TextureUnitState::Sync(TextureUnitState &ns, bool force)
// ----------------------------------------------------------------------------
//   Sync the state of a given texture unit to the new state
// ----------------------------------------------------------------------------
{
    // Check if the mode changed
    if (force || mode != ns.mode)
    {
        mode = ns.mode;
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, mode);
    }

    // Check if the matrix changed
    if (force || matrix != ns.matrix)
    {
        glMatrixMode(GL_TEXTURE);
        matrix = ns.matrix;
        glLoadMatrixd(matrix.Data(false));
    }
    
#define SYNC_CAP(cap, state)                    \
    if (force || state != ns.state)             \
    {                                           \
        if (ns.state)                           \
            glEnable(cap);                      \
        else                                    \
            glDisable(cap);                     \
        state = ns.state;                       \
    }
    SYNC_CAP(GL_TEXTURE_1D,             tex1D);
    SYNC_CAP(GL_TEXTURE_2D,             tex2D);
    SYNC_CAP(GL_TEXTURE_3D,             tex3D);
    SYNC_CAP(GL_TEXTURE_CUBE_MAP,       texCube);
#undef SYNC_CAP

    // Check if there was a change in the texture bound to the unit
    if (force || texture != ns.texture || target != ns.target)
    {
        texture = ns.texture;
        target = ns.target;
        glBindTexture(target, texture);
    }
}


bool TextureUnitsState::Sync(TextureUnitsState &ns, uint activeUnit)
// ----------------------------------------------------------------------------
//   Sync with the new state for all texture units
// ----------------------------------------------------------------------------
{
    uint max = units.size();
    uint nmax = ns.units.size();
    uint64 ndirty = ns.dirty;
    uint lastUnit = activeUnit;

    // Quick bail out if nothing to do
    if (ndirty == 0 && max == nmax)
        return false;

    // Reset count of active texture units
    active = 0;

    // Synchronize all texture units that are common between the two states
    uint umax = max < nmax ? max : nmax;
    for (uint u = 0; u < umax; u++)
    {
        TextureUnitState &us = units[u];
        uint unit = GL_TEXTURE0 + u;
        if (ndirty && (1ULL << u))
        {
            TextureUnitState &nus = ns.units[u];
            if (lastUnit != unit)
            {
                glActiveTexture(unit);
                lastUnit = unit;
            }
            us.Sync(nus, false);
        }
        if (us.tex1D || us.tex2D || us.tex3D || us.texCube)
            active |= 1ULL << u;
    }

    // Check if number of texture units changed
    if (nmax < max)
    {
        // Number of texture units decreased, deactivate extra units
        for (uint u = nmax; u < max; u++)
        {
            TextureUnitState &oldtu = units[u];
            uint unit = GL_TEXTURE0 + u;
            if (lastUnit != unit)
            {
                glActiveTexture(unit);
                lastUnit = unit;
            }
            if (oldtu.tex1D)   glDisable(GL_TEXTURE_1D);
            if (oldtu.tex2D)   glDisable(GL_TEXTURE_2D);
            if (oldtu.tex3D)   glDisable(GL_TEXTURE_3D);
            if (oldtu.texCube) glDisable(GL_TEXTURE_CUBE_MAP);
        }
        units.resize(nmax);
    }
    else if (nmax > max)
    {
        // Number of texture units increased, activate new ones
        for (uint u = max; u < nmax; u++)
        {
            // Initialize with a default unit state
            units.push_back(TextureUnitState());

            // Sync that state now
            TextureUnitState &nus = ns.units[u];
            TextureUnitState &us = units.back();
            uint unit = GL_TEXTURE0 + u;
            if (lastUnit != unit)
            {
                glActiveTexture(unit);
                lastUnit = unit;
            }
            us.Sync(nus, true);
            if (us.tex1D || us.tex2D || us.tex3D || us.texCube)
                active |= 1ULL << u;
        }
    }

    // We are done with current texture changes
    dirty = ns.dirty = 0;

    // Indicate if we changed the active unit compared to OpenGLState's
    return lastUnit != activeUnit;
}



// ============================================================================
//
//    Client active texture units
//
// ============================================================================

void ClientTextureUnitState::Sync(ClientTextureUnitState &ns, bool force)
// ----------------------------------------------------------------------------
//   Sync the state of a given client texture unit to the new state
// ----------------------------------------------------------------------------
{
#define SYNC_CAP(cap, state)                    \
    if (force || state != ns.state)             \
    {                                           \
        if (ns.state)                           \
            glEnableClientState(cap);           \
        else                                    \
            glDisableClientState(cap);          \
        state = ns.state;                       \
    }
    SYNC_CAP(GL_TEXTURE_COORD_ARRAY,             texCoordArray);
#undef SYNC_CAP

}


bool ClientTextureUnitsState::Sync(ClientTextureUnitsState &ns, uint clientActiveUnit)
// ----------------------------------------------------------------------------
//   Sync with the new state for all client texture units
// ----------------------------------------------------------------------------
{
    uint max = units.size();
    uint nmax = ns.units.size();
    uint64 ndirty = ns.dirty;
    uint lastUnit = clientActiveUnit;

    // Quick bail out if nothing to do
    if (ndirty == 0 && max == nmax)
        return false;

    // Reset count of active texture units
    active = 0;

    // Synchronize all texture units that are common between the two states
    uint umax = max < nmax ? max : nmax;
    for (uint u = 0; u < umax; u++)
    {
        ClientTextureUnitState &us = units[u];
        uint unit = GL_TEXTURE0 + u;
        if (ndirty && (1ULL << u))
        {
            ClientTextureUnitState &nus = ns.units[u];
            if (lastUnit != unit)
            {
                glClientActiveTexture(unit);
                lastUnit = unit;
            }
            us.Sync(nus, false);
        }
        if (us.texCoordArray)
            active |= 1ULL << u;
    }

    // Check if number of texture units changed
    if (nmax < max)
    {
        // Number of texture units decreased, deactivate extra units
        for (uint u = nmax; u < max; u++)
        {
            ClientTextureUnitState &oldtu = units[u];
            uint unit = GL_TEXTURE0 + u;
            if (lastUnit != unit)
            {
                glClientActiveTexture(unit);
                lastUnit = unit;
            }
            if (oldtu.texCoordArray)   glDisableClientState(GL_TEXTURE_COORD_ARRAY);
        }
        units.resize(nmax);
    }
    else if (nmax > max)
    {
        // Number of texture units increased, activate new ones
        for (uint u = max; u < nmax; u++)
        {
            // Initialize with a default unit state
            units.push_back(ClientTextureUnitState());

            // Sync that state now
            ClientTextureUnitState &nus = ns.units[u];
            ClientTextureUnitState &us = units.back();
            uint unit = GL_TEXTURE0 + u;
            if (lastUnit != unit)
            {
                glClientActiveTexture(unit);
                lastUnit = unit;
            }
            us.Sync(nus, true);
            if (us.texCoordArray)
                active |= 1ULL << u;
        }
    }

    // We are done with current texture changes
    dirty = ns.dirty = 0;

    // Indicate if we changed the active unit compared to OpenGLState's
    return lastUnit != clientActiveUnit;
}



// ============================================================================
// 
//    Texture states
// 
// ============================================================================

TextureState::TextureState(GLuint id)
// ----------------------------------------------------------------------------
//   Initialize a texture state
// ----------------------------------------------------------------------------
  : type(GL_TEXTURE_2D),
    id(id), width(0), height(0),
    minFilt(GL_NEAREST_MIPMAP_LINEAR),
    magFilt(GL_LINEAR),
    active(false),
    wrapS(false), wrapT(false), wrapR(false),
    mipmap(false),
    unit(0), mode(GL_MODULATE)
{}


void TextureState::Sync(TextureState &ts)
// ----------------------------------------------------------------------------
//   Sync a texture state with a new value
// ----------------------------------------------------------------------------
{
    bool force = (id == 0);

    bool traceErrors = XLTRACE(glerrors);

#define SYNC_TEXTURE(name, Code)                \
    do                                          \
    {                                           \
        if (force || name != ts.name)           \
        {                                       \
            name = ts.name;                     \
            Code;                               \
            if (traceErrors)                    \
                OpenGLState::ShowErrors(#name); \
        }                                       \
    } while(0)

    type = ts.type;
    id = ts.id;

    SYNC_TEXTURE(active,
                 if (active) glEnable(type); else glDisable(type));
    SYNC_TEXTURE(minFilt,
                 glTexParameteri (type, GL_TEXTURE_MIN_FILTER, minFilt));
    SYNC_TEXTURE(magFilt,
                 glTexParameteri (type, GL_TEXTURE_MAG_FILTER, magFilt));
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


void TexturesState::Sync(TexturesState &nt, TextureUnitState &at)
// ----------------------------------------------------------------------------
//   Sync with a new texture state. nt contains only the changed textures.
// ----------------------------------------------------------------------------
{
    GLuint tex = at.texture;

    // Loop over all dirty textures in the new set
    texture_set::iterator it;
    texture_set &ndirty = nt.dirty;
    for (it = ndirty.begin(); it != ndirty.end(); it++)
    {
        GLuint id = *it;
        if (id)
        {
            TextureState &nts = nt.textures[id];
            TextureState &ts = textures[id];
            if (tex != id)
            {
                glBindTexture(nts.type, id);
                tex = id;
            }
            ts.Sync(nts);
        }
    }
    nt.dirty.clear();
    dirty.clear();

    // Restore originally bound texture
    if (tex != at.texture)
        glBindTexture(at.target, at.texture);
}

TAO_END


void debugglerrors()
// ----------------------------------------------------------------------------
//   Show GL errors
// ----------------------------------------------------------------------------
{
    Tao::OpenGLState::ShowErrors("From debugger");
}
