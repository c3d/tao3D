#ifndef FLARE_H
#define FLARE_H
// ****************************************************************************
//  flare.h                                                         Tao project
// ****************************************************************************
//
//   File Description:
//
//      Draw a lens flare at a specific location and heading toward a
//      defined target.
//
//
//
//
//
// ****************************************************************************
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************


#include "tao/coords3d.h"
#include "tao/module_api.h"
#include "tao/tao_gl.h"
#include <QObject>
#include <vector>

using namespace std;
using namespace Tao;

struct Flare
// ----------------------------------------------------------------------------
//   Representation of a single flare
// ----------------------------------------------------------------------------
{
  GLuint id;            // Flare texture index
  float scale;          // Scaling rate
  float loc;            // Position on axis
  float color[4];       // Mixing color
};

struct LensFlare : public QObject
// ----------------------------------------------------------------------------
//   Define a complete lens flare
// ----------------------------------------------------------------------------
{
    // Constructor and destructor
    LensFlare();
    ~LensFlare();

    // Specify lens flare parameters
    void            setTarget(Vector3 target);
    void            setSource(Vector3 source);
    void            enableDephTest(bool enable);

    // Add a flare to the list
    void            addFlare(GLuint id, float location, float scale, GLfloat r, GLfloat g, GLfloat b, GLfloat a);

    // Draw lens flare
    void            Draw();

    static void     render_callback(void *arg);
    static void     delete_callback(void *arg);

private:
    // Draw single flare
    void            DrawFlare(Flare flare, Vector3 position);
    // Depth test for the lens flare
    bool            isOccluded(Vector3 p);

public:
    // Pointer to Tao functions
    static const Tao::ModuleApi *tao;

private:
    // List of attached flares
    vector<Flare> lens_flare;
    // Depth test status
    bool          depth_test;
    // Define source and target of the lens flare
    Vector3       target, source;
};

#endif
