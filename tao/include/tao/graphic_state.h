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
    void pushMatrix();
    void popMatrix();
    void setMatrixMode(GLenum mode);
    void loadMatrix();
    void loadIdentity();
    void printMatrix(GLuint model);

    // Transformations
    void translate(double x, double y, double z);
    void rotate(double a, double x, double y, double z);
    void scale(double x, double y, double z);

    // Camera management
    void pickMatrix(float x, float y, float width, float height,
                    int viewport[4]);
    void setFrustum(float left, float right, float bottom, float top,
                    float nearZ, float farZ);
    void setPerspective(float fovy, float aspect, float nearZ, float farZ);
    void setOrtho(float left, float right, float bottom,
                  float top, float nearZ, float farZ);
    void setOrtho2D(float left, float right, float bottom, float top);
    void setLookAt(float eyeX, float eyeY, float eyeZ,
                   float centerX, float centerY, float centerZ,
                   float upX, float upY, float upZ);
    void setLookAt(Vector3 eye, Vector3 center, Vector3 up);
    void setViewport(int x, int y, int w, int h);

    // Scene management
    void setColor(float r, float g, float b, float a);
    void setClearColor(float r, float g, float b, float a);
    void setClear(GLenum mask);

    void setShadeModel(GLenum mode);

    void setDepthFunc(GLenum func);
    void setDepthMask(GLboolean flag);

    void setLineWidth(float width);
    void setLineStipple(GLint factor, GLushort pattern);

    // Misc
    void enable(GLenum cap);
    void disable(GLenum cap);

    static inline GraphicState* State() { Q_ASSERT(current); return current; }

    std::ostream & debug();

public:
    enum Vendor {
        ATI = 0,
        NVIDIA = 1,
        INTEL = 2,
        LAST = 3
    };

    typedef std::stack<MatrixState> matrixState;

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
    GLenum       clearMask;

    GLenum       shadeMode;

    GLboolean    depthMask;
    GLenum       depthFunc;

    GLuint       lineWidth;
    GLint        stippleFactor;
    GLushort     stipplePattern;

    matrixState projStack; // the stack for projection matrices
    matrixState mvStack;   // the stack for modelview matrices

public:
    static GraphicState* current;
    static text          vendorsList[LAST];

};

TAO_END

#endif // STATES_H
