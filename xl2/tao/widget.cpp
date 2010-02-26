// ****************************************************************************
//  widget.cpp							   Tao project
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
#include <cmath>
#include "widget.h"
#include "tao.h"
#include "main.h"
#include "runtime.h"
#include "opcodes.h"
#include "gl_keepers.h"
#include "frame.h"
#include "texture.h"
#include "svg.h"
#include "window.h"
#include <QtOpenGL>
#include <QFont>
#include <iostream>


TAO_BEGIN

// ============================================================================
//
//   Widget life management
//
// ============================================================================

Widget::Widget(Window *parent, XL::SourceFile *sf)
// ----------------------------------------------------------------------------
//    Create the GL widget
// ----------------------------------------------------------------------------
    : QGLWidget(QGLFormat(QGL::SampleBuffers|QGL::AlphaChannel), parent),
      xlProgram(sf), timer(this)
{
    // Make this the current context for OpenGL
    makeCurrent();

    // Prepare the timer
    connect(&timer, SIGNAL(timeout()), this, SLOT(updateGL()));

    // No bounding box to start with
    boundingBox = NULL;
}


Widget::~Widget()
// ----------------------------------------------------------------------------
//   Destroy the widget
// ----------------------------------------------------------------------------
{
}



// ============================================================================
//
//   Widget basic events (painting, mause, ...)
//
// ============================================================================

void Widget::initializeGL()
// ----------------------------------------------------------------------------
//    Called once per rendering to setup the GL environment
// ----------------------------------------------------------------------------
{
}


void Widget::resizeGL(int width, int height)
// ----------------------------------------------------------------------------
//   Called when the size changes
// ----------------------------------------------------------------------------
{
}


void Widget::paintGL()
// ----------------------------------------------------------------------------
//    Repaint the contents of the window
// ----------------------------------------------------------------------------
{
    draw();
    glShowErrors();
}


void Widget::setup(double w, double h)
// ----------------------------------------------------------------------------
//   Setup an initial environment for drawing
// ----------------------------------------------------------------------------
{
    // Setup the projection matrix
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    double zNear = 1000.0, zFar = 40000.0;
    double eyeX = 0.0, eyeY = 0.0, eyeZ = 1000.0;
    double centerX = 0.0, centerY = 0.0, centerZ = 0.0;
    double upX = 0.0, upY = 1.0, upZ = 0.0;
    glFrustum (-w/2, w/2, -h/2, h/2, zNear, zFar);
    gluLookAt(eyeX, eyeY, eyeZ, centerX, centerY, centerZ, upX, upY, upZ);

    // Setup the model view matrix so that 1.0 unit = 1px
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glViewport(0, 0, w, h);
    glTranslatef(0.0, 0.0, -zNear);
    glScalef(2.0, 2.0, 2.0);

    // Setup other
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEPTH_TEST);
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    // Initial state
    state.polygonMode = GL_POLYGON;
    state.frameWidth = w;
    state.frameHeight = h;
    state.charFormat = QTextCharFormat();
    state.paintDevice = this;
    QTextOption tmp(Qt::AlignCenter);
    state.textOptions = &tmp;

    // Initial bounding box
    delete boundingBox;
    boundingBox = NULL;
}


void Widget::draw()
// ----------------------------------------------------------------------------
//    Redraw the widget
// ----------------------------------------------------------------------------
{
    // Clear the background
    glClearColor (1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // If there is a program, we need to run it
    if (xlProgram)
    {
        // Setup the initial drawing environment
        setup(width(), height());

	// Run the XL program associated with this widget
	current = this;
        TextFlow mainFlow(*state.textOptions);
        state.flow = &mainFlow;
        state.textOptions = & state.flow->paragraphOption;
        state.charFormat.setTextOutline(QPen(Qt::black));
        state.charFormat.setForeground(QBrush(Qt::black));
        state.charFormat.setBackground(QBrush(QColor(255,255,255,0)));


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
        catch(...)
        {
            xlProgram = NULL;
            QMessageBox::warning(this, tr("Runtime error"),
                                 tr("Unknown error executing the program"));
        }

        // Once we are done, do a garbage collection
        state.flow = NULL;
        XL::Context::context->CollectGarbage();
    }
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


Tree *Widget::status(Tree *self, text caption)
// ----------------------------------------------------------------------------
//   Set the status line of the window
// ----------------------------------------------------------------------------
{
    Window *window = (Window *) parentWidget();
    window->statusBar()->showMessage(QString::fromStdString(caption));
    return XL::xl_true;
}


Tree *Widget::rotateX(Tree *self, double rx)
// ----------------------------------------------------------------------------
//   Rotation along the X axis
// ----------------------------------------------------------------------------
{
    glRotatef(rx, 1.0, 0.0, 0.0);
    return XL::xl_true;
}


Tree *Widget::rotateY(Tree *self, double ry)
// ----------------------------------------------------------------------------
//    Rotation along the Y axis
// ----------------------------------------------------------------------------
{
    glRotatef(ry, 0.0, 1.0, 0.0);
    return XL::xl_true;
}


Tree *Widget::rotateZ(Tree *self, double rz)
// ----------------------------------------------------------------------------
//    Rotation along the Z axis
// ----------------------------------------------------------------------------
{
    glRotatef(rz, 0.0, 0.0, 1.0);
    return XL::xl_true;
}


Tree *Widget::rotate(Tree *self, double rx, double ry, double rz, double ra)
// ----------------------------------------------------------------------------
//    Rotation along an arbitrary axis
// ----------------------------------------------------------------------------
{
    glRotatef(rx, ry, rz, ra);
    return XL::xl_true;
}


Tree *Widget::translateX(Tree *self, double rx)
// ----------------------------------------------------------------------------
//    Translation along the X axis
// ----------------------------------------------------------------------------
{
    glTranslatef(rx, 0.0, 0.0);
    return XL::xl_true;
}


Tree *Widget::translateY(Tree *self, double ry)
// ----------------------------------------------------------------------------
//     Translation along the Y axis
// ----------------------------------------------------------------------------
{
    glTranslatef(0.0, ry, 0.0);
    return XL::xl_true;
}


Tree *Widget::translateZ(Tree *self, double rz)
// ----------------------------------------------------------------------------
//     Translation along the Z axis
// ----------------------------------------------------------------------------
{
    glTranslatef(0.0, 0.0, rz);
    return XL::xl_true;
}


Tree *Widget::translate(Tree *self, double rx, double ry, double rz)
// ----------------------------------------------------------------------------
//     Translation along three axes
// ----------------------------------------------------------------------------
{
    glTranslatef(rx, ry, rz);
    return XL::xl_true;
}


Tree *Widget::scaleX(Tree *self, double sx)
// ----------------------------------------------------------------------------
//    Scaling along the X axis
// ----------------------------------------------------------------------------
{
    glScalef(sx, 1.0, 1.0);
    return XL::xl_true;
}


Tree *Widget::scaleY(Tree *self, double sy)
// ----------------------------------------------------------------------------
//     Scaling along the Y axis
// ----------------------------------------------------------------------------
{
    glScalef(1.0, sy, 1.0);
    return XL::xl_true;
}


Tree *Widget::scaleZ(Tree *self, double sz)
// ----------------------------------------------------------------------------
//     Scaling along the Z axis
// ----------------------------------------------------------------------------
{
    glScalef(1.0, 1.0, sz);
    return XL::xl_true;
}


Tree *Widget::scale(Tree *self, double sx, double sy, double sz)
// ----------------------------------------------------------------------------
//     Scaling along three axes
// ----------------------------------------------------------------------------
{
    glScalef(sx, sy, sz);
    return XL::xl_true;
}


Tree *Widget::refresh(Tree *self, double delay)
// ----------------------------------------------------------------------------
//    Refresh after the given number of seconds
// ----------------------------------------------------------------------------
{
    timer.setSingleShot(true);
    timer.start(1000 * delay);
    return XL::xl_true;
}


Tree *Widget::locally(Tree *self, Tree *child)
// ----------------------------------------------------------------------------
//   Evaluate the child tree while preserving the OpenGL context
// ----------------------------------------------------------------------------
{
    GLAndWidgetKeeper save(this);
    Tree *result = xl_evaluate(child);
    return result;
}


Tree *Widget::pagesize(Tree *self, uint w, uint h)
// ----------------------------------------------------------------------------
//    Set the bit size for the page textures
// ----------------------------------------------------------------------------
{
    // Little practical point in ever creating textures bigger than viewport
    if (w > width())    w = width();
    if (h > height())   h = height();
    state.frameWidth = w;
    state.frameHeight = h;
    return XL::xl_true;
}


Tree *Widget::page(Tree *self, Tree *p)
// ----------------------------------------------------------------------------
//  Evaluate the tree in a frame with the given size
// ----------------------------------------------------------------------------
{
    uint w = state.frameWidth, h = state.frameHeight;
    FrameInfo *frame = self->GetInfo<FrameInfo>();
    if (!frame)
    {
        frame = new FrameInfo(w,h);
        self->SetInfo<FrameInfo> (frame);
    }
    Tree *result = NULL;

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();

    frame->resize(w,h);

    frame->begin();
    {
        // Clear the background and setup initial state
        setup(w, h);
        XL::LocalSave<QPaintDevice *> sv(state.paintDevice, frame->render_fbo);
        result = xl_evaluate(p);
    }
    frame->end();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glPopAttrib();

    frame->bind();

    return result;
}


Tree *Widget::time(Tree *self)
// ----------------------------------------------------------------------------
//   Return a fractional time, including milliseconds
// ----------------------------------------------------------------------------
{
    QTime t = QTime::currentTime();
    double d = (3600.0	 * t.hour()
		+ 60.0	 * t.minute()
		+	   t.second()
		+  0.001 * t.msec());
    return new XL::Real(d);
}


Tree *Widget::filled(Tree *self)
// ----------------------------------------------------------------------------
//   Select filled polygon mode
// ----------------------------------------------------------------------------
{
    state.polygonMode = GL_POLYGON;
    return XL::xl_true;
}


Tree *Widget::hollow(Tree *self)
// ----------------------------------------------------------------------------
//   Select hollow polygon mode
// ----------------------------------------------------------------------------
{
    state.polygonMode = GL_LINE_LOOP;
    return XL::xl_true;
}


Tree *Widget::linewidth(Tree *self, double lw)
// ----------------------------------------------------------------------------
//    Select the line width for OpenGL
// ----------------------------------------------------------------------------
{
    glLineWidth(lw);
    return XL::xl_true;
}


Tree *Widget::color(Tree *self, double r, double g, double b, double a)
// ----------------------------------------------------------------------------
//    Set the RGBA color
// ----------------------------------------------------------------------------
{
    glColor4f(r,g,b,a);

    // Set color for text layout
    const double amp=255.9;
    QColor qcolor(floor(amp*r),floor(amp*g),floor(amp*b),floor(amp*a));
    state.charFormat.setTextOutline(QPen(qcolor));
    state.charFormat.setForeground(QBrush(qcolor));
    state.charFormat.setBackground(QBrush(QColor(255,255,255,0)));
 //   state.charFormat.setTextOutline(QPen(Qt::red));

    return XL::xl_true;
}


Tree *Widget::polygon(Tree *self, Tree *child)
// ----------------------------------------------------------------------------
//   Evaluate the child tree within a polygon
// ----------------------------------------------------------------------------
{
    GLAndWidgetKeeper save(this);
    glBegin(state.polygonMode);
    xl_evaluate(child);
    glEnd();
    return XL::xl_true;
}


Tree *Widget::vertex(Tree *self, double x, double y, double z)
// ----------------------------------------------------------------------------
//     GL vertex
// ----------------------------------------------------------------------------
{
    glVertex3f(x, y, z);

    Box dot(x, y, 0, 0); // FIXME: Temporary projection to the plan z=0
    if (boundingBox == NULL)
    {
        boundingBox = new Box();
        *boundingBox = dot;
    }
    else
    {
        *boundingBox |= dot;
    }

    return XL::xl_true;
}



// ============================================================================
//
//    Texture management
//
// ============================================================================

Tree *Widget::texture(Tree *self, text img)
// ----------------------------------------------------------------------------
//     Build a GL texture out of an image file
// ----------------------------------------------------------------------------
{
    if (img == "")
    {
        glDisable(GL_TEXTURE_2D);
        return XL::xl_true;
    }

    ImageTextureInfo *rinfo = self->GetInfo<ImageTextureInfo>();
    if (!rinfo)
    {
        rinfo = new ImageTextureInfo();
        self->SetInfo<ImageTextureInfo>(rinfo);
    }
    rinfo->bind(img);
    return XL::xl_true;
}


Tree *Widget::svg(Tree *self, text img)
// ----------------------------------------------------------------------------
//    Draw an image in SVG format
// ----------------------------------------------------------------------------
//    The image may be animated, in which case we will get repaintNeeded()
//    signals that we send to our 'draw()' so that we redraw as needed.
{
    SvgRendererInfo *rinfo = self->GetInfo<SvgRendererInfo>();
    if (!rinfo)
    {
        rinfo = new SvgRendererInfo(this);
        self->SetInfo<SvgRendererInfo>(rinfo);
    }
    rinfo->bind(img);
    return XL::xl_true;
}


Tree *Widget::texCoord(Tree *self, double x, double y)
// ----------------------------------------------------------------------------
//     GL texture coordinate
// ----------------------------------------------------------------------------
{
    glTexCoord2f(x, y);
    return XL::xl_true;
}


Tree *Widget::sphere(Tree *self,
                     double x, double y, double z,
                     double r, int nslices, int nstacks)
// ----------------------------------------------------------------------------
//     GL sphere
// ----------------------------------------------------------------------------
{
    GLUquadric *q = gluNewQuadric();
    gluQuadricTexture (q, true);
    glPushMatrix();
    glTranslatef(x,y,z);
    glRotatef(-90.0, 1.0, 0.0, 0.0);
    gluSphere(q, r, nslices, nstacks);
    glPopMatrix();
    return XL::xl_true;
}


void Widget::widgetVertex(double x, double y, double tx, double ty)
// ----------------------------------------------------------------------------
//   A vertex, including texture coordinate and bounding box
// ----------------------------------------------------------------------------
{
    texCoord(NULL, tx, ty);
    vertex(NULL, x, y, 0.0);   
}


void Widget::circularVertex(double cx, double cy, double r,
                            double x, double y,
                            double tx0, double ty0, double tx1, double ty1)
// ----------------------------------------------------------------------------
//   A circular vertex, including texture coordinate
// ----------------------------------------------------------------------------
//   x range between -1 and 1, y between -1 and 1
//   cx and cy are the center of the circle, r its radius
{
    widgetVertex(cx + r * x, 
                 cy + r * y,
                 tx0 + ((x + 1.0) / 2.0 * (tx1 - tx0)),
                 ty0 + ((y + 1.0) / 2.0 * (ty1 - ty0)));
}


void Widget::circularSectorN(double cx, double cy, double r,
                            double tx0, double ty0, double tx1, double ty1,
                            int sq, int nq)
// ----------------------------------------------------------------------------
//     Draw a circular sector of N/4th of a circle
// ----------------------------------------------------------------------------
//   We use a reduced Bresenham-like algorithm for circles (midpoint circle)
//
//   For now the sector is limited to multiples of 1/4th of circle. For
//   example, an angle of 280 will draw 3/4 of a circle.
{
    // The two first values configure how precise the circle is
    int step = 10;                // Triangles generated every <step> points
    double grid = 1.0 / 500.0;    // Tolerance for points on the circle 
    double error, x, y, s;


    for (int q = 0; q < nq; q++)
    { 
    error = -1.0;
    x = 1.0;
    y = 0;
    s = step;

    while (x > 0)
    {
        if (++s >= step)
        {
        s = 0;
        switch ((sq + q) % 4)
        {
            case 3: 
                circularVertex(cx, cy, r,  y, -x, tx0, ty0, tx1, ty1);
                break;
            case 2: 
                circularVertex(cx, cy, r, -x, -y, tx0, ty0, tx1, ty1);
                break;
            case 1: 
                circularVertex(cx, cy, r, -y,  x, tx0, ty0, tx1, ty1);
                break;
            case 0: 
                circularVertex(cx, cy, r,  x,  y, tx0, ty0, tx1, ty1);
                break;
        }
        }
        if (error >= 0)
        {
            x -= grid;
            error -= x + x;
        }
        else 
        {
            y += grid;
            error += y + y;
        }
    }
    }
    switch ((sq + nq) % 4)
    {
        case 3: 
            circularVertex(cx, cy, r,  0, -1, tx0, ty0, tx1, ty1);
            break;
        case 2: 
            circularVertex(cx, cy, r, -1,  0, tx0, ty0, tx1, ty1);
            break;
        case 1: 
            circularVertex(cx, cy, r,  0,  1, tx0, ty0, tx1, ty1);
            break;
        case 0: 
            circularVertex(cx, cy, r,  1,  0, tx0, ty0, tx1, ty1);
            break;
    }
}


void Widget::debugBoundingBox()
{
    if (boundingBox == NULL) return;

    // DEBUG: draw the bounding box
    GLAndWidgetKeeper save(this);
    //glColor4f(1.0f, 0.0f, 0.0f, 0.8f);
    glBegin(GL_LINE_LOOP);
    {
        glVertex2f(boundingBox->lower.x, boundingBox->lower.y);
        glVertex2f(boundingBox->upper.x, boundingBox->lower.y);
        glVertex2f(boundingBox->upper.x, boundingBox->upper.y);
        glVertex2f(boundingBox->lower.x, boundingBox->upper.y);
    }
    glEnd();
}


Tree *Widget::circle(Tree *self, double cx, double cy, double r)
// ----------------------------------------------------------------------------
//     GL circle centered around (cx,cy), radius r
// ----------------------------------------------------------------------------
{
    glBegin(state.polygonMode);
    circularSectorN(cx, cy, r, 0, 0, 1, 1, 0, 4);
    glEnd();

    // DEBUG: draw the bounding box
    debugBoundingBox();

    return XL::xl_true;
}


Tree *Widget::circularSector(Tree *self,
                         double cx, double cy, double r, 
                         double a, double b)
// ----------------------------------------------------------------------------
//     GL circular sector centered around (cx,cy), radius r and two angles a, b
// ----------------------------------------------------------------------------
{
    while (b < a)
    {
        b += 360;
    }
    int nq = int((b-a) / 90);                   // Number of quadrants to draw
    if (nq > 4)
    {
        nq = 4;
    }

    while (a < 0)
    {
        a += 360;
    }
    int sq = (int(a / 90) % 4);                 // Starting quadrant

    glBegin(state.polygonMode);
    circularVertex(cx, cy, r, 0, 0, 0, 0, 1, 1);    // The center
    circularSectorN(cx, cy, r, 0, 0, 1, 1, sq, nq);
    glEnd();

    // DEBUG: draw the bounding box
    debugBoundingBox();

    return XL::xl_true;
 }



Tree *Widget::roundedRectangle(Tree *self,
                        double cx, double cy, 
                        double w, double h, double r)
// ----------------------------------------------------------------------------
//     GL rounded rectangle with radius r for the rounded corners
// ----------------------------------------------------------------------------
{
    if (r <= 0) return rectangle(self, cx, cy, w, h);
    if (r > w/2) r = w/2;
    if (r > h/2) r = h/2;

    double x0  = cx-w/2;
    double x0r = x0+r;
    double x1  = cx+w/2;
    double x1r = x1-r;

    double y0  = cy-h/2;
    double y0r = y0+r;
    double y1  = cy+h/2;
    double y1r = y1-r;

    double tx0  = 0;
    double tx0r = 0+r/w;
    double tx0d = 0+2*r/w;
    double tx1  = 1;
    double tx1r = 1-r/w;
    double tx1d = 1-2*r/w;

    double ty0  = 0;
    double ty0r = 0+r/h;
    double ty0d = 0+2*r/h;
    double ty1  = 1;
    double ty1r = 1-r/h;
    double ty1d = 1-2*r/h;

    glBegin(state.polygonMode);
    {
        widgetVertex(x1, y1r, tx1, ty1r);

        circularSectorN(x1r, y1r, r, tx1d, ty1d, tx1, ty1, 0, 1);

        widgetVertex(x1r, y1, tx1r, ty1);
        widgetVertex(x0r, y1, tx0r, ty1);
    
        circularSectorN(x0r, y1r, r, tx0, ty1d, tx0d, ty1, 1, 1);

        widgetVertex(x0, y1r, tx0, ty1r);
        widgetVertex(x0, y0r, tx0, ty0r);
    
        circularSectorN(x0r, y0r, r, tx0, ty0, tx0d, ty0d, 2, 1);

        widgetVertex(x0r, y0, tx0r, ty0);
        widgetVertex(x1r, y0, tx1r, ty0);
    
        circularSectorN(x1r, y0r, r, tx1d, ty0, tx1, ty0d, 3, 1);
   
        widgetVertex(x1, y0r, tx1, ty0r);
    }
    glEnd();

    // DEBUG: draw the bounding box
    debugBoundingBox();

    return XL::xl_true;
}


Tree *Widget::rectangle(Tree *self, double cx, double cy, double w, double h)
// ----------------------------------------------------------------------------
//     GL rectangle centered around (cx,cy), width w, height h
// ----------------------------------------------------------------------------
{
    glBegin(state.polygonMode);
    {
        widgetVertex(cx-w/2, cy-h/2, 0, 0);
        widgetVertex(cx+w/2, cy-h/2, 1, 0);
        widgetVertex(cx+w/2, cy+h/2, 1, 1);
        widgetVertex(cx-w/2, cy+h/2, 0, 1);
    }
    glEnd();

    // DEBUG: draw the bounding box
    debugBoundingBox();

    return XL::xl_true;
}


Tree *Widget::regularStarPolygon(Tree *self, double cx, double cy, double r, 
                        int p, int q)
// ----------------------------------------------------------------------------
//     GL regular p-side star polygon {p/q} centered around (cx,cy), radius r
// ----------------------------------------------------------------------------
{
    if (p < 2 || q < 1 || q > (p-1)/2)
        return XL::xl_false;

    double R_r = cos( q*M_PI/p ) / cos( (q-1)*M_PI/p );
    double R = r * R_r;

    GLuint mode = state.polygonMode;
    if (mode == GL_POLYGON) 
    {
        mode = GL_TRIANGLE_FAN; // GL_POLYGON does not work here
    }
    
    glBegin(mode);
    {
        if (mode == GL_TRIANGLE_FAN)
        {
            circularVertex(cx, cy, r, 0, 0, 0, 0, 1, 1);    // The center
        }

        for (int i = 0; i < p; i++)
        {
            circularVertex(cx, cy, r, 
                    cos( i * 2*M_PI/p + M_PI_2), 
                    sin( i * 2*M_PI/p + M_PI_2),
                    0, 0, 1, 1);

            circularVertex(cx, cy, R, 
                    cos( i * 2*M_PI/p + M_PI_2 + M_PI/p), 
                    sin( i * 2*M_PI/p + M_PI_2 + M_PI/p),
                    (1-R_r)/2, (1-R_r)/2, (1+R_r)/2, (1+R_r)/2);
        }

        if (mode == GL_TRIANGLE_FAN)
        {
            circularVertex(cx, cy, r, 0, 1, 0, 0, 1, 1);    // Closing the star
        }
    }
    glEnd();

    // DEBUG: draw the bounding box
    debugBoundingBox();

    return XL::xl_true;
}


XL::Real *Widget::fromCm(Tree *self, double cm)
// ----------------------------------------------------------------------------
//   Convert from cm to pixels
// ----------------------------------------------------------------------------
{
    XL_RREAL(cm * logicalDpiX() * (1.0 / 2.54));
}


XL::Real *Widget::fromMm(Tree *self, double mm)
// ----------------------------------------------------------------------------
//   Convert from mm to pixels
// ----------------------------------------------------------------------------
{
    XL_RREAL(mm * logicalDpiX() * (0.1 / 2.54));
}


XL::Real *Widget::fromIn(Tree *self, double in)
// ----------------------------------------------------------------------------
//   Convert from inch to pixels
// ----------------------------------------------------------------------------
{
    XL_RREAL(in * logicalDpiX());
}


XL::Real *Widget::fromPt(Tree *self, double pt)
// ----------------------------------------------------------------------------
//   Convert from pt to pixels
// ----------------------------------------------------------------------------
{
    XL_RREAL(pt * logicalDpiX() * (1.0 / 72.0));
}


XL::Real *Widget::fromPx(Tree *self, double px)
// ----------------------------------------------------------------------------
//   Convert from pixel to pixels (trivial)
// ----------------------------------------------------------------------------
{
    XL_RREAL(px);
}


Tree *Widget::font(Tree *self, text description)
// ----------------------------------------------------------------------------
//   Select a font family
// ----------------------------------------------------------------------------
{
    QFont font = state.charFormat.font();
    font.fromString((QString::fromStdString(description)));
    state.charFormat.setFont(font);
    return XL::xl_true;
}


Tree *Widget::fontSize(Tree *self, double size)
// ----------------------------------------------------------------------------
//   Select a font size
// ----------------------------------------------------------------------------
{
    state.charFormat.setFontPointSize(size);
    return XL::xl_true;
}


Tree *Widget::fontPlain(Tree *self)
// ----------------------------------------------------------------------------
//   Select whether this is italic or not
// ----------------------------------------------------------------------------
{
    state.charFormat.setFontItalic(false);
    state.charFormat.setFontWeight(QFont::Normal);
    state.charFormat.setFontUnderline(false);
    state.charFormat.setFontOverline(false);
    state.charFormat.setFontStrikeOut(false);
    return XL::xl_true;
}


Tree *Widget::fontItalic(Tree *self, bool italic)
// ----------------------------------------------------------------------------
//   Select whether this is italic or not
// ----------------------------------------------------------------------------
{
    state.charFormat.setFontItalic(italic);
    return XL::xl_true;
}


Tree *Widget::fontBold(Tree *self, bool bold)
// ----------------------------------------------------------------------------
//   Select whether the font is bold or not
// ----------------------------------------------------------------------------
{
    state.charFormat.setFontWeight( bold ? QFont::Bold : QFont::Normal);
    return XL::xl_true;
}


Tree *Widget::fontUnderline(Tree *self, bool underline)
// ----------------------------------------------------------------------------
//    Select whether we underline a font
// ----------------------------------------------------------------------------
{
    state.charFormat.setFontUnderline(underline);
    return XL::xl_true;
}


Tree *Widget::fontOverline(Tree *self, bool overline)
// ----------------------------------------------------------------------------
//    Select whether we draw an overline 
// ----------------------------------------------------------------------------
{
    state.charFormat.setFontOverline(overline);
    return XL::xl_true;
}


Tree *Widget::fontStrikeout(Tree *self, bool strikeout)
// ----------------------------------------------------------------------------
//    Select whether we strikeout a font
// ----------------------------------------------------------------------------
{
    state.charFormat.setFontStrikeOut(strikeout);
    return XL::xl_true;
}


Tree *Widget::fontStretch(Tree *self, int stretch)
// ----------------------------------------------------------------------------
//    Set font streching factor
// ----------------------------------------------------------------------------
{
    //state.charFormat.setFontStretch(stretch);
    return XL::xl_true;
}


Tree *Widget::align(Tree *self, int align)
// ----------------------------------------------------------------------------
//   Set text alignment
// ----------------------------------------------------------------------------
{
    Qt::Alignment old = state.textOptions->alignment();
    if (align & Qt::AlignHorizontal_Mask)
        old &= ~Qt::AlignHorizontal_Mask;
    if (align & Qt::AlignVertical_Mask)
        old &= ~Qt::AlignVertical_Mask;
    align |= old;
    state.textOptions->setAlignment(Qt::Alignment(align));
    return XL::xl_true;
}


Tree *Widget::textSpan(Tree *self, text content)
// ----------------------------------------------------------------------------
//   Insert a block of text with the current definition of font, color, ...
// ----------------------------------------------------------------------------
{
    state.flow->addText(QString::fromStdString(content), state.charFormat);
    return XL::xl_true;
}


Tree *Widget::flow(Tree *self)
// ----------------------------------------------------------------------------
//   Create a new text flow
// ----------------------------------------------------------------------------
{
    TextFlow *thisFlow = self->GetInfo<TextFlow>();
    if (!thisFlow)
    {
        thisFlow = new TextFlow(*state.textOptions);
        self->SetInfo<TextFlow> (thisFlow);
    }
    state.flow = thisFlow;
    state.textOptions = &(state.flow->paragraphOption);
    return XL::xl_true;
}


Tree *Widget::frameTexture(Tree *self, double w, double h)
// ----------------------------------------------------------------------------
//   Make a texture out of the current text layout
// ----------------------------------------------------------------------------
{
    if (w < 16) w = 16;
    if (h < 16) h = 16;

    // Get or build the current frame if we don't have one
    FrameInfo *frame = self->GetInfo<FrameInfo>();
    if (!frame)
    {
        frame = new FrameInfo(w,h);
        self->SetInfo<FrameInfo> (frame);
    }

    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();

    frame->resize(w,h);

    frame->begin();
    {
        // Clear the background and setup initial state
        setup(w, h);
        XL::LocalSave<QPaintDevice *> sv(state.paintDevice, frame->render_fbo);

        TextFlow *flow = state.flow;
        qreal lineY = 0, lineHeight = 0, topY = flow->topLineY;

        flow->setText(flow->completeText);
        flow->setAdditionalFormats(flow->formats);
        flow->setTextOption(flow->paragraphOption);
        flow->beginLayout();

        while(true)
        {
            // Create a new line
            QTextLine line = flow->createLine();
            if (!line.isValid())
                break;
            if (lineHeight + lineY - topY >= h)
                break;
            line.setLineWidth(w);
            line.setPosition(QPoint(0, lineY - topY));
            lineHeight = line.height();
            lineY += lineHeight;
        }
        
        flow->topLineY = lineY;
        flow->endLayout();

        QPainter painter(state.paintDevice);
//       painter.setRenderHint(QPainter::Antialiasing);
        painter.setRenderHint(QPainter::Antialiasing, false);
        painter.setRenderHint(QPainter::HighQualityAntialiasing, false);
        painter.setRenderHint(QPainter::TextAntialiasing, false);
        flow->draw(&painter, QPoint(0,0));
        painter.end();

        flow->clear();
    }
    frame->end();

    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glPopAttrib();

    // Bind the resulting texture
    frame->bind();

    return XL::xl_true;
}


Tree *Widget::frame(Tree *self, double x, double y, double w, double h)
// ----------------------------------------------------------------------------
//   Draw a frame with the current text flow
// ----------------------------------------------------------------------------
{
    glPushAttrib(GL_TEXTURE_BIT);
    frameTexture(self, w, h);

    // Draw a rectangle with the resulting texture
    glBegin(state.polygonMode);
    {
        widgetVertex(x-w/2, y-h/2, 0, 0);
        widgetVertex(x+w/2, y-h/2, 1, 0);
        widgetVertex(x+w/2, y+h/2, 1, 1);
        widgetVertex(x-w/2, y+h/2, 0, 1);
    }
    glEnd();
    glPopAttrib();

    return XL::xl_true;
}

TAO_END
