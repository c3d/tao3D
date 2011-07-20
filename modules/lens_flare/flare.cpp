// ****************************************************************************
//  flare.cpp                                                       Tao project
// ****************************************************************************
//
//   File Description:
//
//    Lens flare implementation.
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
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "flare.h"

const ModuleApi *LensFlare::tao = NULL;

// ============================================================================
//
//    Lens Flare
//
// ============================================================================

LensFlare::LensFlare() : depth_test(true), target(0, 0, 0), source(0, 0, 0)
// ----------------------------------------------------------------------------
//   Construction
// ----------------------------------------------------------------------------
{
}

LensFlare::~LensFlare()
// ----------------------------------------------------------------------------
//   Destruction
// ----------------------------------------------------------------------------
{
}

void LensFlare::render_callback(void *arg)
// ----------------------------------------------------------------------------
//   Rendering callback: call the render function for the object
// ----------------------------------------------------------------------------
{
    ((LensFlare *)arg)->Draw();
}

void LensFlare::delete_callback(void *arg)
// ----------------------------------------------------------------------------
//   Delete callback: destroy object
// ----------------------------------------------------------------------------
{
    delete (LensFlare *)arg;
}

void LensFlare::setTarget(Vector3 position)
// ----------------------------------------------------------------------------
//   Define position of the lens flare target
// ----------------------------------------------------------------------------
{
    target = position;
}

void LensFlare::setSource(Vector3 position)
// ----------------------------------------------------------------------------
//   Define position of the lens flare source light
// ----------------------------------------------------------------------------
{
    source = position;
}

void LensFlare::enableDephTest(bool enable)
// ----------------------------------------------------------------------------
//   Enable or disable manual depth test for the lens flare
// ----------------------------------------------------------------------------
{
    depth_test = enable;
}

void LensFlare::addFlare(GLuint id, float location, float scale, GLfloat r, GLfloat g, GLfloat b, GLfloat a)
// ----------------------------------------------------------------------------
//   Create a new flare and add it to the others.
// ----------------------------------------------------------------------------
{
    Flare flare;

    // Define flare paramaters
    flare.id = id;
    flare.loc = location;
    flare.scale = scale;
    flare.color[0] = r;
    flare.color[1] = g;
    flare.color[2] = b;
    flare.color[3] = a;

    // Add to the flares list for drawing
    lens_flare.push_back(flare);
}

void LensFlare::Draw()
// ----------------------------------------------------------------------------
//   Draw a lens flare centred at the source position
//   and heading toward the defined target.
// ----------------------------------------------------------------------------
{
    // Disable current depth_test to avoid
    // display problem with flares
    glDisable(GL_DEPTH_TEST);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    // Compute lens direction
    Vector3 lens_dir = target  - source;

    // Determine manually if the source is occluded by a previous object.
    // If it is, we draw no one of the flares.
    if(! isOccluded(source))
    {
        glDepthMask(GL_FALSE);
        for(uint i = 0; i < lens_flare.size(); i++)
        {
            // Interpolate position of the current flare
            Vector3 pos = source + lens_flare[i].loc * lens_dir;
            DrawFlare(lens_flare[i], pos);
        }
        glDepthMask(GL_TRUE);
    }

    // Restore previous bend settings.
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);

    // Restore OpenGL depth test
    glEnable(GL_DEPTH_TEST);
}

void LensFlare::DrawFlare(Flare flare, Vector3 pos)
// ----------------------------------------------------------------------------
//   Draw a flare at the specified position.
// ----------------------------------------------------------------------------
{
    float scale = flare.scale;
    // General scaling rate of flares textures of size 256 * 256
    scale *= 128;

    glBindTexture(GL_TEXTURE_2D, flare.id);
    glColor4f(flare.color[0],
              flare.color[1],
              flare.color[2],
              flare.color[3]);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(pos.x - scale, pos.y - scale, pos.z);

    glTexCoord2f(1.0, 0.0);
    glVertex3f(pos.x + scale, pos.y - scale, pos.z);

    glTexCoord2f(1.0, 1.0);
    glVertex3f(pos.x + scale, pos.y  + scale, pos.z);

    glTexCoord2f(0.0, 1.0);
    glVertex3f(pos.x - scale, pos.y + scale, pos.z);

    glEnd();
}

bool LensFlare::isOccluded(Vector3 p)
// ----------------------------------------------------------------------------
//   Define a manual depth test in order to determine if
//   a lens flare source is occluded by an other object in the scene
// ----------------------------------------------------------------------------
{
    if(depth_test)
    {
        GLint viewport[4];							// Space for viewport data
        GLdouble mvmatrix[16], projmatrix[16];					// Space for transform matrix
        GLdouble winx, winy, winz;						// Space for returned projected coords
        GLdouble flareZ;							// Store the transformed flare Z
        GLfloat bufferZ;							// Store the read Z from the buffer

        glGetIntegerv (GL_VIEWPORT, viewport);					// Get actual viewport
        glGetDoublev (GL_MODELVIEW_MATRIX, mvmatrix);				// Get actual model view matrix
        glGetDoublev (GL_PROJECTION_MATRIX, projmatrix);			// Get actual projection matrix

        // This asks OGL to guess the 2D position of a 3D point inside the viewport
        gluProject(p.x, p.y, p.z, mvmatrix, projmatrix, viewport, &winx, &winy, &winz);
        flareZ = winz;

        // Read back one pixel from the depth buffer (exactly where source should be drawn)
        glReadPixels(winx, winy, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &bufferZ);

        // If the buffer Z is lower than the flare guessed Z then don't draw
        // This means there is something in front of our source.
        if (bufferZ < flareZ)
            return true;
    }

    return false;
}
