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

#include <iostream>



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
      xlProgram(sf),
      caption_text("A simple OpenGL framebuffer object example.")
{
    // Make this the current context for OpenGL
    makeCurrent();
}


TaoWidget::~TaoWidget()
// ----------------------------------------------------------------------------
//   Destroy the widget
// ----------------------------------------------------------------------------
{}



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


struct SheetInfo : Info
// ----------------------------------------------------------------------------
//    Information about a given sheet being rendered in a dynamic texture
// ----------------------------------------------------------------------------
{
    typedef SheetInfo *data_t;

    SheetInfo();
    ~SheetInfo();

    void begin();
    void end();

    QGLFramebufferObject *render_fbo;
    QGLFramebufferObject *texture_fbo;
    GLuint                tile_list;
};


SheetInfo::SheetInfo()
// ----------------------------------------------------------------------------
//   Create the required frame buffer objects and tile
// ----------------------------------------------------------------------------
    : render_fbo(NULL), texture_fbo(NULL)
{
    // Select whether we draw directly in texture or blit to it
    // If we can blit, we first draw in a multisample buffer
    // with 4 samples per pixel. This cannot be used directly as texture.
    const int w = 512, h = 512;
    if (QGLFramebufferObject::hasOpenGLFramebufferBlit())
    {
        QGLFramebufferObjectFormat format;
#ifndef CONFIG_LINUX
        // Setting this crashes in the first framebuffer object ctor
        format.setSamples(4);
#endif
        format.setAttachment(QGLFramebufferObject::CombinedDepthStencil);

        render_fbo = new QGLFramebufferObject(w, h, format);
        texture_fbo = new QGLFramebufferObject(w, h);
    }
    else
    {
        render_fbo = new QGLFramebufferObject(w, h);
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


SheetInfo::~SheetInfo()
// ----------------------------------------------------------------------------
//   Delete the frame buffer object and GL tile
// ----------------------------------------------------------------------------
{
    glDeleteLists(tile_list, 1);
    delete texture_fbo;
    if (render_fbo != texture_fbo)
        delete render_fbo;
}


struct SheetPainter : QPainter
// ----------------------------------------------------------------------------
//   Paint on a given sheet, given a SheetInfo
// ----------------------------------------------------------------------------
{
    SheetPainter(TaoWidget *wid, SheetInfo *info);
    ~SheetPainter();
    TaoWidget *tao;
    SheetInfo *info;
};


SheetPainter::SheetPainter(TaoWidget *wid, SheetInfo *info)
// ----------------------------------------------------------------------------
//   Begin drawing in the current context
// ----------------------------------------------------------------------------
    : QPainter(), tao(wid), info(info)
{
    // Draw without any transformation (reset the coordinates system)
    tao->saveGLState();
    glLoadIdentity();

    // Clear the render FBO
    info->render_fbo->bind();
    glClearColor(0.0, 0.0, 0.3, 0.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    info->render_fbo->release();

    begin(info->render_fbo);
}


SheetPainter::~SheetPainter()
// ----------------------------------------------------------------------------
//   Finish the drawing on the current sheet
// ----------------------------------------------------------------------------
{
    end();

    // Blit the result in the texture if necessary
    if (info->render_fbo != info->texture_fbo) {
        QRect rect(0, 0, info->render_fbo->width(), info->render_fbo->height());
        QGLFramebufferObject::blitFramebuffer(info->texture_fbo, rect,
                                              info->render_fbo, rect);
    }

    tao->restoreGLState();
}


struct SvgRendererInfo : SheetInfo
// ----------------------------------------------------------------------------
//    Hold information about the SVG renderer for a tree
// ----------------------------------------------------------------------------
{
    typedef SvgRendererInfo *data_t;

    SvgRendererInfo(QSvgRenderer *r): SheetInfo(), renderer(r) {}
    ~SvgRendererInfo() { delete renderer; }
    operator data_t() { return this; }

    QSvgRenderer         *renderer;
};


Tree *TaoWidget::drawSvg(Tree *self, text img)
// ----------------------------------------------------------------------------
//    Draw an image in SVG format
// ----------------------------------------------------------------------------
//    The image may be animated, in which case we will get repaintNeeded()
//    signals that we send to our 'draw()' so that we redraw as needed.
{
    SvgRendererInfo *rinfo = self->GetInfo<SvgRendererInfo>();
    QSvgRenderer    *r     = NULL;

    if (rinfo)
    {
        r = rinfo->renderer;
    }
    else
    {
        QString qs = QString::fromStdString(img);
        r = new QSvgRenderer(qs, this);
        connect(r, SIGNAL(repaintNeeded()), this, SLOT(draw()));
        rinfo = new SvgRendererInfo(r);
        self->SetInfo<SvgRendererInfo>(rinfo);
    }

    {
        SheetPainter painter(this, rinfo);
        r->render(&painter);
    }

    // Bind to the texture
    saveGLState();
#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif
    glBindTexture(GL_TEXTURE_2D, rinfo->texture_fbo->texture());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_CULL_FACE);

    // Draw a tile
    glCallList(rinfo->tile_list);
    restoreGLState();

    return NULL;
}


Tree *TaoWidget::rotateX(Tree *self, double rx)
// ----------------------------------------------------------------------------
//   Rotation along the X axis
// ----------------------------------------------------------------------------
{
    glRotatef(rx, 1.0, 0.0, 0.0);
    return NULL;
}


Tree *TaoWidget::rotateY(Tree *self, double ry)
// ----------------------------------------------------------------------------
//    Rotation along the Y axis
// ----------------------------------------------------------------------------
{
    glRotatef(ry, 0.0, 1.0, 0.0);
    return NULL;
}


Tree *TaoWidget::rotateZ(Tree *self, double rz)
// ----------------------------------------------------------------------------
//    Rotation along the Z axis
// ----------------------------------------------------------------------------
{
    glRotatef(rz, 0.0, 0.0, 1.0);
    return NULL;
}


Tree *TaoWidget::rotate(Tree *self, double rx, double ry, double rz, double ra)
// ----------------------------------------------------------------------------
//    Rotation along an arbitrary axis
// ----------------------------------------------------------------------------
{
    glRotatef(rx, ry, rz, ra);
    return NULL;
}


Tree *TaoWidget::translateX(Tree *self, double rx)
// ----------------------------------------------------------------------------
//    Translation along the X axis
// ----------------------------------------------------------------------------
{
    glTranslatef(rx, 0.0, 0.0);
    return NULL;
}


Tree *TaoWidget::translateY(Tree *self, double ry)
// ----------------------------------------------------------------------------
//     Translation along the Y axis
// ----------------------------------------------------------------------------
{
    glTranslatef(0.0, ry, 0.0);
    return NULL;
}


Tree *TaoWidget::translateZ(Tree *self, double rz)
// ----------------------------------------------------------------------------
//     Translation along the Z axis
// ----------------------------------------------------------------------------
{
    glTranslatef(0.0, 0.0, rz);
    return NULL;
}


Tree *TaoWidget::translate(Tree *self, double rx, double ry, double rz)
// ----------------------------------------------------------------------------
//     Translation along three axes
// ----------------------------------------------------------------------------
{
    glTranslatef(rx, ry, rz);
    return NULL;
}


Tree *TaoWidget::scaleX(Tree *self, double sx)
// ----------------------------------------------------------------------------
//    Scaling along the X axis
// ----------------------------------------------------------------------------
{
    glScalef(sx, 1.0, 1.0);
    return NULL;
}


Tree *TaoWidget::scaleY(Tree *self, double sy)
// ----------------------------------------------------------------------------
//     Scaling along the Y axis
// ----------------------------------------------------------------------------
{
    glScalef(1.0, sy, 1.0);
    return NULL;
}


Tree *TaoWidget::scaleZ(Tree *self, double sz)
// ----------------------------------------------------------------------------
//     Scaling along the Z axis
// ----------------------------------------------------------------------------
{
    glScalef(1.0, 1.0, sz);
    return NULL;
}


Tree *TaoWidget::scale(Tree *self, double sx, double sy, double sz)
// ----------------------------------------------------------------------------
//     Scaling along three axes
// ----------------------------------------------------------------------------
{
    glScalef(sx, sy, sz);
    return NULL;
}


Tree *TaoWidget::color(Tree *self, double r, double g, double b, double a)
// ----------------------------------------------------------------------------
//    Set the RGBA color
// ----------------------------------------------------------------------------
{
    glColor4f(r,g,b,a);
    return NULL;
}


Tree *TaoWidget::refresh(Tree *self, double delay)
// ----------------------------------------------------------------------------
//    Refresh after the given number of seconds
// ----------------------------------------------------------------------------
{
    QTimer::singleShot(1000 * delay, this, SLOT(draw()));
    return NULL;
}


Tree *TaoWidget::locally(Tree *self, Tree *child)
// ----------------------------------------------------------------------------
//   Evaluate the child tree while preserving the OpenGL context
// ----------------------------------------------------------------------------
{
    saveGLState();
    Tree *result = xl_evaluate(child);
    restoreGLState();
    return result;
}


Tree *TaoWidget::time(Tree *self)
// ----------------------------------------------------------------------------
//   Return a fractional time, including milliseconds
// ----------------------------------------------------------------------------
{
    QTime t = QTime::currentTime();
    double d = (3600.0   * t.hour()
                + 60.0   * t.minute()
                +          t.second()
                +  0.001 * t.msec());
    return new Real(d);
}
