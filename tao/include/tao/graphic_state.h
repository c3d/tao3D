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
#include "tao_gl.h"

TAO_BEGIN

struct ModelState;
struct GraphicState;

//  Shortcut
#define GL              Tao::GraphicState::State()

struct ModelState
// ----------------------------------------------------------------------------
//   The state of the model we want to preserve
// ----------------------------------------------------------------------------
{
    ModelState(): tx(0), ty(0), tz(0),
                  sx(1), sy(1), sz(1),
                  rotation(1, 0, 0, 0) {}

    float tx, ty, tz;     // Translate parameters
    float sx, sy, sz;     // Scaling parameters
    Quaternion rotation;  // Rotation parameters
};


struct GraphicState
// ----------------------------------------------------------------------------
//   Class to manage graphic states
// ----------------------------------------------------------------------------
{
    GraphicState();

    // Matrix management
    void projectionMatrix();
    void modelViewMatrix();
    void textureMatrix();

    // Draw management
    void setColor(float r, float g, float b, float a);

    // Transform
    void translate(float x, float y, float z);
    void rotate(float a, float x, float y, float z);
    void scale(float x, float y, float z);

    static GraphicState* State() { Q_ASSERT(current); return current; }

public:
    GLenum     matrixMode;
    GLfloat    color[4];

    static GraphicState* current;
};

TAO_END

#endif // STATES_H
