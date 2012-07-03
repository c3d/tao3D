#ifndef OPENGL_STATE_H
#define OPENGL_STATE_H
// ****************************************************************************
//  opengl_state.h                                                 Tao project
// ****************************************************************************
//
//   File Description:
//
//     Manage OpenGL state
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
//  (C) 2012 Baptiste Soulisse <soulisse.baptiste@taodyne.com>
//  (C) 2012 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2012 Taodyne SAS
// ****************************************************************************

#include "coords3d.h"
#include "color.h"
#include "matrix.h"
#include "graphic_state.h"
#include <stack>

TAO_BEGIN

struct OpenGLSave;              // Structure used to save/restore state

struct ViewportState
// ----------------------------------------------------------------------------
//    Represent the viewport information in a more readable way
// ----------------------------------------------------------------------------
{
    ViewportState(int x, int y, int w, int h): x(x), y(y), w(w), h(h) {}
    bool operator==(const ViewportState &o)
    {
        return x==o.x && y==o.y && w==o.w && h==o.h;
    }
    bool operator!=(const ViewportState &o) { return !operator==(o); }
    int x, y, w, h;
};


struct LineStippleState
// ----------------------------------------------------------------------------
//    Represent line stipple information
// ----------------------------------------------------------------------------
{
    LineStippleState(GLint f, GLushort p): factor(f), pattern(p) {}
    bool operator==(const LineStippleState &o)
    {
        return factor == o.factor && pattern == o.pattern;
    }
    bool operator!=(const LineStippleState &o) { return !operator==(o); }
    GLint factor;
    GLushort pattern;
};


struct OpenGLState : GraphicState
// ----------------------------------------------------------------------------
//   Class to manage graphic states
// ----------------------------------------------------------------------------
{
    OpenGLState();

    // Saving and restoring state
    virtual GraphicSave *       Save();
    virtual void                Restore(GraphicSave *saved);

    // Return attributes of state
    virtual uint   MaxTextureCoords()           { return maxTextureCoords; }
    virtual uint   MaxTextureUnits()            { return maxTextureUnits; }
    virtual text   Vendor()                     { return vendor; }
    virtual text   Renderer()                   { return renderer; }
    virtual text   Version()                    { return version; }
    virtual uint   VendorID()                   { return vendorID; }
    virtual bool   IsATIOpenGL()                { return vendorID == ATI; }


    // Matrix management
    virtual coord* ModelViewMatrix();
    virtual coord* ProjectionMatrix();
    virtual void   MatrixMode(GLenum mode);
    virtual void   LoadMatrix();
    virtual void   LoadIdentity();
    virtual void   MultMatrices(const coord *m1,const coord *m2,coord *result);
    virtual void   MultMatrixVec(const coord matrix[16], const coord in[4],
                                 coord out[4]);
    virtual bool   InvertMatrix(const GLdouble m[16], GLdouble invOut[16]);
    virtual void   PrintMatrix(GLuint model);

    // Transformations
    virtual void Translate(double x, double y, double z);
    virtual void Rotate(double a, double x, double y, double z);
    virtual void Scale(double x, double y, double z);

    // Camera management
    virtual bool Project(coord objx, coord objy, coord objz,
                         const coord* mv, const coord* proj,
                         const int* viewport,
                         coord *winx, coord *winy, coord *winz);
    virtual bool UnProject(coord winx, coord winy, coord winz,
                           const coord* mv, const coord* proj,
                           const int* viewport,
                           coord *objx, coord *objy, coord *objz);
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
    virtual int *Viewport()           { return (int *) &viewport.x; }

    // Attributes management
    virtual void Color(float r, float g, float b, float a);
    virtual void ClearColor(float r, float g, float b, float a);
    virtual void Clear(GLuint mask);
    virtual void LineWidth(float width);
    virtual void LineStipple(GLint factor, GLushort pattern);
    virtual void DepthMask(GLboolean flag);
    virtual void DepthFunc(GLenum func);
    virtual void ShadeModel(GLenum mode);
    virtual void Hint(GLenum target, GLenum mode);
    virtual void Enable(GLenum cap);
    virtual void Disable(GLenum cap);

    // Blend
    virtual void BlendFunc(GLenum sfactor, GLenum dfactor);
    virtual void BlendFuncSeparate(GLenum sRgb, GLenum dRgb,
                                   GLenum sAlpha, GLenum dAlpha);
    virtual void BlendEquation(GLenum mode);

    // Alpha
    virtual void AlphaFunc(GLenum func, float ref);

    std::ostream & debug();

public:
#define GS(type, name)                          \
    bool name##_isDirty : 1;
#include "opengl_state.tbl"

public:
    enum VendorID
    {
        UNKNOWN = 0,
        ATI,
        NVIDIA,
        INTEL,
        LAST_VENDOR
    };

public:
    enum VendorID vendorID;
    GLuint       maxTextureCoords;
    GLuint       maxTextureUnits;
    text         vendor;
    text         renderer;
    text         version;
    text         extensionsAvailable;

#define GS(type, name)                          \
    type name;
#define GFLAG(enum, name)                       \
    bool name:1;
#include "opengl_state.tbl"

public:
    static text          vendorsList[LAST_VENDOR];

private:
    // Structure used to push/pop state
    friend class OpenGLSave;
    OpenGLSave *save;
};

TAO_END

#endif // GRAPHIC_STATE_H
