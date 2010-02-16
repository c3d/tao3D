// ****************************************************************************
//  widget.cpp                                                     Tao project
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
//  (C) 2010 Lionel Schaffhauser <lionel@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include <QtGui/QImage>
#include <math.h>
#include "widget.h"
#include "tao.h"
#include "main.h"
#include "runtime.h"
#include "gl_keepers.h"
#include <GL.h>


#include <iostream>


TAO_BEGIN

// ============================================================================
// 
//   Widget life management
// 
// ============================================================================

Widget::Widget(QWidget *parent, XL::SourceFile *sf)
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


Widget::~Widget()
// ----------------------------------------------------------------------------
//   Destroy the widget
// ----------------------------------------------------------------------------
{}



// ============================================================================
// 
//   Widget basic events (painting, mause, ...)
// 
// ============================================================================

void Widget::paintEvent(QPaintEvent *)
// ----------------------------------------------------------------------------
//    Repaint the widget
// ----------------------------------------------------------------------------
{
    draw();
}



void Widget::draw()
// ----------------------------------------------------------------------------
//    Redraw the widget
// ----------------------------------------------------------------------------
{
    QPainter p(this); // used for text overlay

    // Run the XL program associated with this widget
    if (xlProgram)
    {
        // Save the GL state set for QPainter
        GLStateKeeper save;

        // Clear the background
        glClearColor (1.0, 1.0, 1.0, 1.0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Setup the projection matrix
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        double w = width(), h = height();
        double zNear = 1000.0, zFar = 40000.0;
        double eyeX = 0.0, eyeY = 0.0, eyeZ = 1000.0;
        double centerX = 0.0, centerY = 0.0, centerZ = 0.0;
        double upX = 0.0, upY = 1.0, upZ = 0.0;
        glFrustum (-w/2, w/2, -h/2, h/2, zNear, zFar);
        gluLookAt(eyeX, eyeY, eyeZ, centerX, centerY, centerZ, upX, upY, upZ);

        // Setup the model view matrix so that 1.0 unit = 1px
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glViewport(0, 0, width(), height());
        glTranslatef(0.0, 0.0, -zNear);
        glScalef(2.0, 2.0, 2.0);

        // Setup other 
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDepthFunc(GL_LESS);
        glEnable(GL_DEPTH_TEST);
        glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

        // Run the XL program associated with this widget
        current = this;
        if (xlProgram)
        {
            try
            {
                xl_evaluate(xlProgram->tree.tree);
            }
            catch (XL::Error &e)
            {
                xlProgram = NULL;
                QMessageBox::warning(this, tr("Runtime error"),
                                     tr("Error executing the program:\n%1")
                                     .arg(QString::fromStdString(e.Message())));
            }
        }
    }

    // Draw the overlayed text using QPainter
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

    // Once we are done, do a garbage collection
    XL::Context::context->CollectGarbage();
}


void Widget::mousePressEvent(QMouseEvent *e)
// ----------------------------------------------------------------------------
//   Mouse button click
// ----------------------------------------------------------------------------
{
}


void Widget::mouseMoveEvent(QMouseEvent *e)
// ----------------------------------------------------------------------------
//    Mouse move
// ----------------------------------------------------------------------------
{
    draw();
}


void Widget::wheelEvent(QWheelEvent *e)
// ----------------------------------------------------------------------------
//   Mouse wheel
// ----------------------------------------------------------------------------
{
}


void Widget::mouseDoubleClickEvent(QMouseEvent *)
// ----------------------------------------------------------------------------
//   Mouse double click
// ----------------------------------------------------------------------------
{
}


void Widget::timerEvent(QTimerEvent *)
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

Widget *Widget::current = NULL;

typedef XL::Tree Tree;


Tree *Widget::caption(Tree *self, text caption)
// ----------------------------------------------------------------------------
//   Set the caption in the title
// ----------------------------------------------------------------------------
{
    caption_text = caption;
    return NULL;
}


struct SheetInfo : XL::Info
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
        double xs = 100.0;
        double ys = 100.0;
        double zs = 100.0;

        glTexCoord2f(0.0f, 0.0f); glVertex3f(-xs, -ys,  zs);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( xs, -ys,  zs);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( xs,  ys,  zs);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-xs,  ys,  zs);

        glTexCoord2f(1.0f, 0.0f); glVertex3f(-xs, -ys, -zs);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-xs,  ys, -zs);
        glTexCoord2f(0.0f, 1.0f); glVertex3f( xs,  ys, -zs);
        glTexCoord2f(0.0f, 0.0f); glVertex3f( xs, -ys, -zs);

        glTexCoord2f(0.0f, 1.0f); glVertex3f(-xs,  ys, -zs);
        glTexCoord2f(0.0f, 0.0f); glVertex3f(-xs,  ys,  zs);
        glTexCoord2f(1.0f, 0.0f); glVertex3f( xs,  ys,  zs);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( xs,  ys, -zs);

        glTexCoord2f(1.0f, 1.0f); glVertex3f(-xs, -ys, -zs);
        glTexCoord2f(0.0f, 1.0f); glVertex3f( xs, -ys, -zs);
        glTexCoord2f(0.0f, 0.0f); glVertex3f( xs, -ys,  zs);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-xs, -ys,  zs);

        glTexCoord2f(1.0f, 0.0f); glVertex3f( xs, -ys, -zs);
        glTexCoord2f(1.0f, 1.0f); glVertex3f( xs,  ys, -zs);
        glTexCoord2f(0.0f, 1.0f); glVertex3f( xs,  ys,  zs);
        glTexCoord2f(0.0f, 0.0f); glVertex3f( xs, -ys,  zs);

        glTexCoord2f(0.0f, 0.0f); glVertex3f(-xs, -ys, -zs);
        glTexCoord2f(1.0f, 0.0f); glVertex3f(-xs, -ys,  zs);
        glTexCoord2f(1.0f, 1.0f); glVertex3f(-xs,  ys,  zs);
        glTexCoord2f(0.0f, 1.0f); glVertex3f(-xs,  ys, -zs);
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
    SheetPainter(SheetInfo *info);
    ~SheetPainter();
    SheetInfo *info;
    GLStateKeeper save;
};


SheetPainter::SheetPainter(SheetInfo *info)
// ----------------------------------------------------------------------------
//   Begin drawing in the current context
// ----------------------------------------------------------------------------
    : QPainter(), info(info), save()
{
    // Draw without any transformation (reset the coordinates system)
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
    if (info->render_fbo != info->texture_fbo)
    {
        QRect rect(0, 0, info->render_fbo->width(), info->render_fbo->height());
        QGLFramebufferObject::blitFramebuffer(info->texture_fbo, rect,
                                              info->render_fbo, rect);
    }
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


Tree *Widget::drawSvg(Tree *self, text img)
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
        SheetPainter painter(rinfo);
        r->render(&painter);
    }

    {
        // Bind to the texture
        GLStateKeeper save;
        glBindTexture(GL_TEXTURE_2D, rinfo->texture_fbo->texture());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_MULTISAMPLE);
        glEnable(GL_CULL_FACE);

        // Draw a tile
        glCallList(rinfo->tile_list);
    }

    return NULL;
}


Tree *Widget::rotateX(Tree *self, double rx)
// ----------------------------------------------------------------------------
//   Rotation along the X axis
// ----------------------------------------------------------------------------
{
    glRotatef(rx, 1.0, 0.0, 0.0);
    return NULL;
}


Tree *Widget::rotateY(Tree *self, double ry)
// ----------------------------------------------------------------------------
//    Rotation along the Y axis
// ----------------------------------------------------------------------------
{
    glRotatef(ry, 0.0, 1.0, 0.0);
    return NULL;
}


Tree *Widget::rotateZ(Tree *self, double rz)
// ----------------------------------------------------------------------------
//    Rotation along the Z axis
// ----------------------------------------------------------------------------
{
    glRotatef(rz, 0.0, 0.0, 1.0);
    return NULL;
}


Tree *Widget::rotate(Tree *self, double rx, double ry, double rz, double ra)
// ----------------------------------------------------------------------------
//    Rotation along an arbitrary axis
// ----------------------------------------------------------------------------
{
    glRotatef(rx, ry, rz, ra);
    return NULL;
}


Tree *Widget::translateX(Tree *self, double rx)
// ----------------------------------------------------------------------------
//    Translation along the X axis
// ----------------------------------------------------------------------------
{
    glTranslatef(rx, 0.0, 0.0);
    return NULL;
}


Tree *Widget::translateY(Tree *self, double ry)
// ----------------------------------------------------------------------------
//     Translation along the Y axis
// ----------------------------------------------------------------------------
{
    glTranslatef(0.0, ry, 0.0);
    return NULL;
}


Tree *Widget::translateZ(Tree *self, double rz)
// ----------------------------------------------------------------------------
//     Translation along the Z axis
// ----------------------------------------------------------------------------
{
    glTranslatef(0.0, 0.0, rz);
    return NULL;
}


Tree *Widget::translate(Tree *self, double rx, double ry, double rz)
// ----------------------------------------------------------------------------
//     Translation along three axes
// ----------------------------------------------------------------------------
{
    glTranslatef(rx, ry, rz);
    return NULL;
}


Tree *Widget::scaleX(Tree *self, double sx)
// ----------------------------------------------------------------------------
//    Scaling along the X axis
// ----------------------------------------------------------------------------
{
    glScalef(sx, 1.0, 1.0);
    return NULL;
}


Tree *Widget::scaleY(Tree *self, double sy)
// ----------------------------------------------------------------------------
//     Scaling along the Y axis
// ----------------------------------------------------------------------------
{
    glScalef(1.0, sy, 1.0);
    return NULL;
}


Tree *Widget::scaleZ(Tree *self, double sz)
// ----------------------------------------------------------------------------
//     Scaling along the Z axis
// ----------------------------------------------------------------------------
{
    glScalef(1.0, 1.0, sz);
    return NULL;
}


Tree *Widget::scale(Tree *self, double sx, double sy, double sz)
// ----------------------------------------------------------------------------
//     Scaling along three axes
// ----------------------------------------------------------------------------
{
    glScalef(sx, sy, sz);
    return NULL;
}


Tree *Widget::color(Tree *self, double r, double g, double b, double a)
// ----------------------------------------------------------------------------
//    Set the RGBA color
// ----------------------------------------------------------------------------
{
    glColor4f(r,g,b,a);
    return NULL;
}


Tree *Widget::refresh(Tree *self, double delay)
// ----------------------------------------------------------------------------
//    Refresh after the given number of seconds
// ----------------------------------------------------------------------------
{
    QTimer::singleShot(1000 * delay, this, SLOT(draw()));
    return NULL;
}


Tree *Widget::locally(Tree *self, Tree *child)
// ----------------------------------------------------------------------------
//   Evaluate the child tree while preserving the OpenGL context
// ----------------------------------------------------------------------------
{
    GLStateKeeper save;
    Tree *result = xl_evaluate(child);
    return result;
}


Tree *Widget::time(Tree *self)
// ----------------------------------------------------------------------------
//   Return a fractional time, including milliseconds
// ----------------------------------------------------------------------------
{
    QTime t = QTime::currentTime();
    double d = (3600.0   * t.hour()
                + 60.0   * t.minute()
                +          t.second()
                +  0.001 * t.msec());
    return new XL::Real(d);
}


Tree *Widget::polygon(Tree *self, Tree *child)
// ----------------------------------------------------------------------------
//   Evaluate the child tree within a polygon 
// ----------------------------------------------------------------------------
{
    GLStateKeeper save;
    glBegin(GL_POLYGON);
    xl_evaluate(child);
    glEnd();
    return NULL;
}


Tree *Widget::vertex(Tree *self, double x, double y, double z)
// ----------------------------------------------------------------------------
//     GL vertex
// ----------------------------------------------------------------------------
{
    glVertex3f(x, y, z);
    return NULL;
}


Tree *Widget::sphere(Tree *self, double x, double y, double z, double r)
// ----------------------------------------------------------------------------
//     GL sphere
// ----------------------------------------------------------------------------
{
    GLUquadric *q = gluNewQuadric();
    gluQuadricTexture (q, true);
    glPushMatrix();
    glTranslatef(x,y,z);
    gluSphere(q, r, 15, 15);
    glPopMatrix();
    return NULL;
}


Tree *Widget::texture(Tree *self, text n, Tree *body)
// ----------------------------------------------------------------------------
//     GL sphere
// ----------------------------------------------------------------------------
{
    QImage source (QString::fromStdString(n));
    QImage glTex = convertToGLFormat(source);
    GLuint textureId;
    glGenTextures(1, &textureId);
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexImage2D(GL_TEXTURE_2D, 0, 3,
                 glTex.width(), glTex.height(), 0, GL_RGBA,
                  GL_UNSIGNED_BYTE, glTex.bits());
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_MULTISAMPLE);

    xl_evaluate(body);

    glDisable(GL_TEXTURE_2D);
    glDeleteTextures(1, &textureId);
    
    return NULL;
}


TAO_END
