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

struct TextureState;
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
    void setMatrixMode(GLenum mode);

    void printMatrix(GLuint model);

    // Draw management
    void setColor(float r, float g, float b, float a);

    // Transformations
    void translate(double x, double y, double z);
    void rotate(double a, double x, double y, double z);
    void scale(double x, double y, double z);

    // Misc
    void enable(GLenum cap);
    void disable(GLenum cap);

    static GraphicState* State() { Q_ASSERT(current); return current; }

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
    GLfloat      color[4];

    static GraphicState* current;
    static text          vendorsList[LAST];

};

TAO_END

#endif // STATES_H
