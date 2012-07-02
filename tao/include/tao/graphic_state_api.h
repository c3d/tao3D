#ifndef GRAPHIC_STATE_API_H
#define GRAPHIC_STATE_API_H
// ****************************************************************************
//  graphic_state_api.h                                             Tao project
// ****************************************************************************
//
//   File Description:
//
//    Graphic state onterface for native modules
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
//  (C) 2012 Taodyne SAS
// ****************************************************************************

#include "coords3d.h"
#include "matrix.h"
#include "tao_gl.h"
#include <stack>

TAO_BEGIN

struct GraphicStateApi
// ----------------------------------------------------------------------------
//   Interface to manage graphic states
// ----------------------------------------------------------------------------
{
    GraphicStateApi() {}

    // Matrix management
    virtual coord* ModelViewMatrix() = 0;
    virtual coord* ProjectionMatrix() = 0;
    virtual void   PushMatrix() = 0;
    virtual void   PopMatrix() = 0;
    virtual void   MatrixMode(GLenum mode) = 0;
    virtual void   LoadMatrix() = 0;
    virtual void   LoadIdentity() = 0;

    // Transformations
    virtual void Translate(double x, double y, double z) = 0;
    virtual void Rotate(double a, double x, double y, double z)  = 0;
    virtual void Scale(double x, double y, double z) = 0;

    // Camera management
    virtual void PickMatrix(float x, float y, float width, float height,
                            int viewport[4]) = 0;
    virtual void Frustum(float left, float right, float bottom, float top,
                         float nearZ, float farZ) = 0;
    virtual void Perspective(float fovy, float aspect, float nearZ, float farZ) = 0;
    virtual void Ortho(float left, float right, float bottom,
                       float top, float nearZ, float farZ) = 0;
    virtual void Ortho2D(float left, float right, float bottom, float top) = 0;
    virtual void LookAt(float eyeX, float eyeY, float eyeZ,
                        float centerX, float centerY, float centerZ,
                        float upX, float upY, float upZ) = 0;
    virtual void LookAt(Vector3 eye, Vector3 center, Vector3 up) = 0;
    virtual void Viewport(int x, int y, int w, int h) = 0;

    // Draw management
    virtual void Color(float r, float g, float b, float a) = 0;
    virtual void ClearColor(float r, float g, float b, float a) = 0;
    virtual void Clear(GLuint mask) = 0;
    virtual void LineWidth(float width) = 0;
    virtual void LineStipple(GLint factor, GLushort pattern) = 0;
    virtual void DepthMask(GLboolean flag) = 0;
    virtual void DepthFunc(GLenum func) = 0;

    // Misc
    virtual void Enable(GLenum cap) = 0;
    virtual void Disable(GLenum cap) = 0;
    virtual void ShadeModel(GLenum mode) = 0;
};

TAO_END

#endif // GRAPHIC_STATE_API_H
