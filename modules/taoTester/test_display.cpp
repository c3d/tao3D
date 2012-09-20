// ****************************************************************************
//  test_display.cpp                                                Tao project
// ****************************************************************************
//
//   File Description:
//
//    Tao display module to test presentation in offline mode.
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
//  (C) 2012 Catherine BURVELLE <Catherine@taodyne.com>
//  (C) 2012 Taodyne SAS
// ****************************************************************************

#include "test_display.h"
#include <tao/tao_gl.h>
#include "widgettests.h"
#include "../tao_synchro/event_capture.h"

test_display::test_display(int w, int h)
    : w(w), h(h), shot(false)
// ----------------------------------------------------------------------------
//   Create a display context
// ----------------------------------------------------------------------------
{
    frame = synchroBasic::tao->newFrameBufferObject(w, h);
}


test_display::~test_display()
// ----------------------------------------------------------------------------
//   Delete display context
// ----------------------------------------------------------------------------
{
    IFTRACE(displaymode)
            debug() << "Deleting framebuffer\n";
    synchroBasic::tao->deleteFrameBufferObject(frame);
}


void test_display::display(void *obj)
// ----------------------------------------------------------------------------
//   Rendering in 2D+Depth mode
// ----------------------------------------------------------------------------
{
    IFTRACE(displaymode)
            debug() << "->test_display::display \n";
    test_display * instance = (test_display *)obj;

    // Save current framebuffer, if any
    GLint fbname = 0;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &fbname);

    // Read output resolution
    int w = synchroBasic::base->winSize.width();
    int h = synchroBasic::base->winSize.height();

    // Make sure output buffer has the right size (resolution may have changed)
    // and prepare to draw into it
    instance->resize(w, h);
    synchroBasic::tao->bindFrameBufferObject(instance->frame);

    // (1) Normal rendering into the FBO.

    // Setup viewport: rendering to full FBO size.
    glViewport(0, 0, w, h);

    synchroBasic::tao->doMouseTracking(true);
    synchroBasic::tao->setMouseTrackingViewport(0, 0, w, h);

    // Clear draw buffer
    synchroBasic::tao->setGlClearColor();
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Setup projection and modelview matrices
    synchroBasic::tao->setProjectionMatrix(w, h, 1, 1);
    synchroBasic::tao->setModelViewMatrix(1, 1);

    // Set suitable GL parameters for drawing
    synchroBasic::tao->setupGl();
    glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA,
                        GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

    // Draw scene, selection and activities
    synchroBasic::tao->drawScene();
    synchroBasic::tao->drawSelection();
    synchroBasic::tao->drawActivities();
    synchroBasic::tao->releaseFrameBufferObject(instance->frame);

    WidgetTests * player = dynamic_cast<WidgetTests *>
            (synchroBasic::base->tao_event_handler);
    if (player && player->shotImage)
    {
        IFTRACE(displaymode)
                debug() << "..test_display::display IMAGE SHOT\n";
        *(player->shotImage) = (QImage *)
                synchroBasic::tao->imageFromFrameBufferObject(instance->frame);
        player->shotImage = NULL;
    }

    // (2) Render a full-screen quad

    synchroBasic::tao->frameBufferObjectToTexture(instance->frame);


    // Select draw buffer
    if (fbname)
        glBindFramebuffer(GL_FRAMEBUFFER, fbname);
    else
        glDrawBuffer(GL_BACK);

    // (2) Render a full-screen quad

    // Set viewport for full screen
    glViewport(0, 0, w, h);

    // Clear draw buffer
    glClearColor(0, 0, 0, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDisable(GL_BLEND);

    // CHECK
    // Not sure why, but without this I often have a blank screen
    glDisable(GL_POLYGON_OFFSET_FILL);
    glDisable(GL_POLYGON_OFFSET_LINE);
    glDisable(GL_POLYGON_OFFSET_POINT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glBegin(GL_QUADS);
    glTexCoord2i( 0 , 0);
    glVertex2i  (-1, -1);
    glTexCoord2i( 1 , 0);
    glVertex2i  ( 1, -1);
    glTexCoord2i( 1,  1);
    glVertex2i  ( 1,  1);
    glTexCoord2i( 0,  1);
    glVertex2i  (-1,  1);
    glEnd();

    IFTRACE(displaymode)
            debug() << "<-test_display::display \n";

}


void * test_display::use()
// ----------------------------------------------------------------------------
//   Display mode is about to be used: create instance of display object
// ----------------------------------------------------------------------------
{
    glewInit();

    QString msg = QObject::tr("TestDisplay");
    synchroBasic::tao->setWatermarkText(text(msg.toUtf8().constData()), 400, 200);

    int w = synchroBasic::base->winSize.width();
    int h = synchroBasic::base->winSize.height();
    test_display * instance = new  test_display(w, h);
    IFTRACE(displaymode)
        debug() << "test_display::use\n";

    return instance;
}


void test_display::resize(int w, int h)
// ----------------------------------------------------------------------------
//   Resize the frame buffer if needed
// ----------------------------------------------------------------------------
{
    if (this->w == 0 && this->h == 0)
    {
        IFTRACE(displaymode)
            debug() << "Allocating framebuffer: "
                    << w << "x" << h << "\n";
        frame = synchroBasic::tao->newFrameBufferObject(w, h);
    }
    else
    {
        if (w != this->w || h != this->h)
        {
            IFTRACE(displaymode)
                debug() << "Resizing framebuffer: "
                        << w << "x" << h << "\n";
            synchroBasic::tao->resizeFrameBufferObject(frame, w, h);
        }
    }
    this->w = w;
    this->h = h;
}


std::ostream & test_display::debug()
// ----------------------------------------------------------------------------
//   Convenience method to log with a common prefix
// ----------------------------------------------------------------------------
{
    std::cerr << "[test_display] ";
    return std::cerr;
}


void test_display::unuse(void *obj)
// ----------------------------------------------------------------------------
//   Delete data allocated in use function
// ----------------------------------------------------------------------------
{
    test_display * o = (test_display *)obj;
    if (o)
        delete o;
}




