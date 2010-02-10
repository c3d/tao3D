// ****************************************************************************
//  tao_widget.cpp                                                  XLR project
// ****************************************************************************
// 
//   File Description:
// 
//     The main widget used to display some Tao stuff
// 
// 
// 
// 
// 
// 
// 
// 
// ****************************************************************************
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include <QtGui/QImage>
#include <math.h>
#include "tao_widget.h"
#include "main.h"
#include "runtime.h"



// ============================================================================
// 
//   Widget life management
// 
// ============================================================================

TaoWidget::TaoWidget(QWidget *parent, XL::SourceFile *sf)
// ----------------------------------------------------------------------------
//    Create the GL widget
// ----------------------------------------------------------------------------
    : QGLWidget(QGLFormat(QGL::SampleBuffers|QGL::AlphaChannel), parent),
      render_fbo(NULL), texture_fbo(NULL),
      xlProgram(sf),
      caption_text("A simple OpenGL framebuffer object example.")
{
    // Make this the current context for OpenGL
    makeCurrent();

    // Select whether we draw directly in texture or blit to it
    // REVISIT ddd: why is this even remotely useful? Is it just a demo?
    if (QGLFramebufferObject::hasOpenGLFramebufferBlit())
    {
        QGLFramebufferObjectFormat format;
        format.setSamples(4);
        format.setAttachment(QGLFramebufferObject::CombinedDepthStencil);

        render_fbo = new QGLFramebufferObject(512, 512, format);
        texture_fbo = new QGLFramebufferObject(512, 512);
    }
    else
    {
        render_fbo = new QGLFramebufferObject(1024, 1024);
        texture_fbo = render_fbo;
    }

    // Generate a cube tile that we will use for drawing
    tile_list = glGenLists(1);
    glNewList(tile_list, GL_COMPILE);
    glBegin(GL_QUADS);
    {
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);

        glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);

        glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);

        glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);

        glTexCoord2f(1.0f, 0.0f); glVertex3f( 1.0f, -1.0f, -1.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( 1.0f,  1.0f, -1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f( 1.0f,  1.0f,  1.0f);
        glTexCoord2f(0.0f, 0.0f); glVertex3f( 1.0f, -1.0f,  1.0f);

        glTexCoord2f(0.0f, 0.0f); glVertex3f(-1.0f, -1.0f, -1.0f);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-1.0f, -1.0f,  1.0f);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-1.0f,  1.0f,  1.0f);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-1.0f,  1.0f, -1.0f);
    }
    glEnd();
    glEndList();
}


TaoWidget::~TaoWidget()
// ----------------------------------------------------------------------------
//   Destroy the widget
// ----------------------------------------------------------------------------
{
    glDeleteLists(tile_list, 1);
    delete texture_fbo;
    if (render_fbo != texture_fbo)
        delete render_fbo;
}



// ============================================================================
// 
//   Widget basic events (painting, mause, ...)
// 
// ============================================================================

void TaoWidget::paintEvent(QPaintEvent *)
// ----------------------------------------------------------------------------
//    Repaint the widget
// ----------------------------------------------------------------------------
{
    draw();
}


void TaoWidget::draw()
// ----------------------------------------------------------------------------
//    Redraw the widget
// ----------------------------------------------------------------------------
{
    QPainter p(this); // used for text overlay

    // Save the GL state set for QPainter
    saveGLState();

    // Setup the GL widget for execution of the XL program
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glFrustum(-1, 1, -1, 1, 10, 100);
    glTranslatef(0.0f, 0.0f, -15.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glViewport(0, 0, width(), height());
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthFunc(GL_LESS);
    glEnable(GL_DEPTH_TEST);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    // Run the XL program associated with this widget
    glPushMatrix();
    current = this;
    if (xlProgram)
        xl_evaluate(xlProgram->tree.tree);
    glPopMatrix();

    // restore the GL state that QPainter expects
    restoreGLState();

    // draw the overlayed text using QPainter
    p.setPen(QColor(197, 197, 197, 157));
    p.setBrush(QColor(197, 197, 197, 127));
    p.drawRect(QRect(0, 0, width(), 50));
    p.setPen(Qt::black);
    p.setBrush(Qt::NoBrush);
    const QString str1 = QString::fromStdString(caption_text);
    const QString str2(tr("Use the mouse wheel to zoom, press buttons and move mouse to rotate, double-click to flip."));
    QFontMetrics fm(p.font());
    p.drawText(width()/2 - fm.width(str1)/2, 20, str1);
    p.drawText(width()/2 - fm.width(str2)/2, 20 + fm.lineSpacing(), str2);
}


void TaoWidget::mousePressEvent(QMouseEvent *e)
// ----------------------------------------------------------------------------
//   Mouse button click
// ----------------------------------------------------------------------------
{
}


void TaoWidget::mouseMoveEvent(QMouseEvent *e)
// ----------------------------------------------------------------------------
//    Mouse move
// ----------------------------------------------------------------------------
{
    draw();
}


void TaoWidget::wheelEvent(QWheelEvent *e)
// ----------------------------------------------------------------------------
//   Mouse wheel
// ----------------------------------------------------------------------------
{
}


void TaoWidget::mouseDoubleClickEvent(QMouseEvent *)
// ----------------------------------------------------------------------------
//   Mouse double click
// ----------------------------------------------------------------------------
{
}


void TaoWidget::saveGLState()
// ----------------------------------------------------------------------------
//   Save the GL state to be able to restore what the QPainter wants
// ----------------------------------------------------------------------------
{
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
}


void TaoWidget::restoreGLState()
// ----------------------------------------------------------------------------
//    Restore the GL state to what the QPainter wants
// ----------------------------------------------------------------------------
{
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glPopAttrib();
}


void TaoWidget::timerEvent(QTimerEvent *)
// ----------------------------------------------------------------------------
//    Timer expired
// ----------------------------------------------------------------------------
{
}



// ============================================================================
// 
//   XLR runtime entry points
// 
// ============================================================================

TaoWidget *TaoWidget::current = NULL;


Tree *TaoWidget::caption(Tree *self, text caption)
// ----------------------------------------------------------------------------
//   Set the caption in the title
// ----------------------------------------------------------------------------
{
    caption_text = caption;
    return NULL;
}


struct SvgRendererInfo : Info
// ----------------------------------------------------------------------------
//    Hold information about the SVG renderer for a tree
// ----------------------------------------------------------------------------
{
    typedef QSvgRenderer *data_t;
    SvgRendererInfo(QSvgRenderer *r): renderer(r) {}
    ~SvgRendererInfo() { delete renderer; }
    operator data_t() { return renderer; }
    QSvgRenderer *renderer;
};


Tree *TaoWidget::drawSvg(Tree *self, text img)
// ----------------------------------------------------------------------------
//    Draw an image in SVG format
// ----------------------------------------------------------------------------
//    The image may be animated, in which case we will get repaintNeeded()
//    signals that we send to our 'draw()' so that we redraw as needed.
{
    QSvgRenderer *r = self->Get<SvgRendererInfo>();
    if (!r)
    {
        QString qs = QString::fromStdString(img);
        r = new QSvgRenderer(qs, this);
        connect(r, SIGNAL(repaintNeeded()), this, SLOT(draw()));
        self->Set<SvgRendererInfo>(r);
    }

    // Render the SVG in our frame buffer object
    QPainter fbo_painter(render_fbo);
    r->render(&fbo_painter);
    fbo_painter.end();

    // Blit the result in the texture if necessary
    if (render_fbo != texture_fbo) {
        QRect rect(0, 0, render_fbo->width(), render_fbo->height());
        QGLFramebufferObject::blitFramebuffer(texture_fbo, rect,
                                              render_fbo, rect);
    }

    // Bind to the texture
#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif
    glBindTexture(GL_TEXTURE_2D, texture_fbo->texture());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_CULL_FACE);

    // Draw a tile
    glCallList(tile_list);

    return NULL;
}


