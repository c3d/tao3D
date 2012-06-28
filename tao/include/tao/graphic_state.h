#ifndef GRAPHIC_STATE_H
#define GRAPHIC_STATE_H
// ****************************************************************************
//  graphic_state.h                                                 Tao project
// ****************************************************************************
//
//   File Description:
//
//     Manage graphic state
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

#include "coords3d.h"
#include "matrix.h"
#include "tao_gl.h"
#include <stack>

TAO_BEGIN

struct MatrixState;
struct GraphicState;

//  Shortcut
#define GL  (*Tao::GraphicState::State())

struct MatrixState
// ----------------------------------------------------------------------------
//   The state of a matrix we want to preserve
// ----------------------------------------------------------------------------
{
    MatrixState() : needUpdate(false) {}

    bool    needUpdate;
    Matrix4 matrix;
};

struct GraphicState
// ----------------------------------------------------------------------------
//   Class to manage graphic states
// ----------------------------------------------------------------------------
{
    GraphicState();

    // Matrix management
    virtual void PushMatrix();
    virtual void PopMatrix();
    virtual void MatrixMode(GLenum mode);
    virtual void LoadMatrix();
    virtual void LoadIdentity();
    virtual void PrintMatrix(GLuint model);

    // Transformations
    virtual void Translate(double x, double y, double z);
    virtual void Rotate(double a, double x, double y, double z);
    virtual void Scale(double x, double y, double z);

    // Camera management
    virtual void PickMatrix(float x, float y, float width, float height,
                    int viewport[4]);
    virtual void Frustum(float left, float right, float bottom, float top,
                 float nearZ, float farZ);
    virtual void Perspective(float fovy, float aspect, float nearZ, float farZ);
    virtual void Ortho(float left, float right, float bottom,
               float top, float nearZ, float farZ);
    virtual void Ortho2D(float left, float right, float bottom, float top);
    virtual void LookAt(float eyeX, float eyeY, float eyeZ,
                float centerX, float centerY, float centerZ,
                float upX, float upY, float upZ);
    virtual void LookAt(Vector3 eye, Vector3 center, Vector3 up);
    virtual void Viewport(int x, int y, int w, int h);

    // Draw management
    virtual void Color(float r, float g, float b, float a);
    virtual void ClearColor(float r, float g, float b, float a);
    virtual void Clear(GLuint mask);
    virtual void LineWidth(float width);
    virtual void LineStipple(GLint factor, GLushort pattern);
    virtual void DepthMask(GLboolean flag);
    virtual void DepthFunc(GLenum func);

    // Misc
    virtual void Enable(GLenum cap);
    virtual void Disable(GLenum cap);
    virtual void ShadeModel(GLenum mode);

    static inline GraphicState* State() { Q_ASSERT(current); return current; }

    std::ostream & debug();

public:
    enum Vendor {
        ATI = 0,
        NVIDIA = 1,
        INTEL = 2,
        LAST = 3
    };

public:
    Vendor       vendorID;
    GLuint       maxTextureCoords;
    GLuint       maxTextureUnits;
    text         vendor;
    text         renderer;
    text         version;
    text         extensionsAvailable;

    GLenum       matrixMode;
    MatrixState* currentMatrix;
    MatrixState  projMatrix;
    MatrixState  mvMatrix;

    GLint        viewport[4];

    GLfloat      color[4];
    GLfloat      clearColor[4];

    GLenum       shadeMode;
    GLuint       lineWidth;
    GLint        stippleFactor;
    GLushort     stipplePattern;

    GLboolean    depthMask;
    GLenum       depthFunc;

    std::stack<MatrixState> projStack; // the stack for projection matrices
    std::stack<MatrixState> mvStack;   // the stack for modelview matrices

public:
    static GraphicState* current;
    static text          vendorsList[LAST];

};

TAO_END

#endif // STATES_H
