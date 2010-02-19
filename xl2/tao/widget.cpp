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
#include <math.h>
#include "widget.h"
#include "tao.h"
#include "page.h"
#include "main.h"
#include "runtime.h"
#include "opcodes.h"
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

    // Initial state
    state.polygonMode = GL_POLYGON;
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
	glDepthFunc(GL_LEQUAL);
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
    GLAndWidgetKeeper save(this);
    Tree *result = xl_evaluate(child);
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
    return NULL;
}


Tree *Widget::hollow(Tree *self)
// ----------------------------------------------------------------------------
//   Select hollow polygon mode
// ----------------------------------------------------------------------------
{
    state.polygonMode = GL_LINE_LOOP;
    return NULL;
}


Tree *Widget::disconnected(Tree *self)
// ----------------------------------------------------------------------------
//   Select a polygon mode that creates disconnected lines
// ----------------------------------------------------------------------------
{
    state.polygonMode = GL_LINE_STRIP;
    return NULL;
}


Tree *Widget::linewidth(Tree *self, double lw)
// ----------------------------------------------------------------------------
//    Select the line width for OpenGL
// ----------------------------------------------------------------------------
{
    glLineWidth(lw);
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


Tree *Widget::polygon(Tree *self, Tree *child)
// ----------------------------------------------------------------------------
//   Evaluate the child tree within a polygon
// ----------------------------------------------------------------------------
{
    GLAndWidgetKeeper save(this);
    glBegin(state.polygonMode);
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
        return NULL;
    }

    ImageTextureInfo *rinfo = self->GetInfo<ImageTextureInfo>();
    if (!rinfo)
    {
        rinfo = new ImageTextureInfo();
        self->SetInfo<ImageTextureInfo>(rinfo);
    }
    rinfo->bind(img);
    return NULL;
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
    return NULL;
}


Tree *Widget::texCoord(Tree *self, double x, double y)
// ----------------------------------------------------------------------------
//     GL texture coordinate
// ----------------------------------------------------------------------------
{
    glTexCoord2f(x, y);
    return NULL;
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
    return NULL;
}


static inline void circVertex(double cx, double cy, double r,
                                double x, double y)
// ----------------------------------------------------------------------------
//   A circular vertex, including texture coordinate
// ----------------------------------------------------------------------------
//   x range between -1 and 1, y between -1 and 1
//   cx and cy are the center of the circle, r its radius
{
    glTexCoord2f((x + 1.0) / 2.0, (y + 1.0) / 2.0);
    glVertex2f(cx + r * x, cy + r * y);
}


static inline void circTriangle1(double cx, double cy, double r,
                                 double x1, double y1,
                                 double x2, double y2)
// ----------------------------------------------------------------------------
//   Draw 1 circular triangle
// ----------------------------------------------------------------------------
{
    // Triangles need to be drawn counter-clockwise
    circVertex(cx, cy, r, 0, 0);
    circVertex(cx, cy, r, x1, y1);
    circVertex(cx, cy, r, x2, y2);
}


static inline void circTriangleN(double cx, double cy, double r,
                           double x1, double y1,
                           double x2, double y2, int sa, int n)
// ----------------------------------------------------------------------------
//   Draw n circular triangles (n from 1 to 8, taking into account symmetries)
// ----------------------------------------------------------------------------
{
    // Triangles need to be drawn counter-clockwise
    for (int i = 0; i < n; i++)
    { 
        switch ((sa + i) % 8)
        {
            case 7: 
                circTriangle1(cx, cy, r,  x2, -y2,  x1, -y1);
                break;
            case 6: 
                circTriangle1(cx, cy, r,  y1, -x1,  y2, -x2);
                break;
            case 5: 
                circTriangle1(cx, cy, r, -y2, -x2, -y1, -x1);
                break;
            case 4: 
                circTriangle1(cx, cy, r, -x1, -y1, -x2, -y2);
                break;
            case 3: 
                circTriangle1(cx, cy, r, -x2,  y2, -x1,  y1);
                break;
            case 2: 
                circTriangle1(cx, cy, r, -y1,  x1, -y2,  x2);
                break;
            case 1: 
                circTriangle1(cx, cy, r,  y2,  x2,  y1,  x1);
                break;
            case 0: 
                circTriangle1(cx, cy, r,  x1,  y1,  x2,  y2);
                break;
        }
    }
}


static inline void circSectorN(double cx, double cy, double r, int sa, int n)
// ----------------------------------------------------------------------------
//     Draw a circular sector of N/8th of a circle
// ----------------------------------------------------------------------------
//   We use a reduced Bresenham-like algorithm for circles (midpoint circle)
//
//   For now the sector is limited to multiples of 1/8th of circle. For
//   example, an angle of 280 will draw 3/4 of a circle.
{
    // The two first values configure how precise the circle is
    int step = 10;              // Triangles generated every <step> points
    double grid = 1 / 500.0;    // Tolerance for points on the circle 

    double end = M_SQRT2 / 2;   // sqrt(1/2) for a perfect finish
    double error = -1.0;
    double x1 = 1.0, x2 = 1.0;
    double y1 = 0, y2 = 0;
    int i = 0;

    while (x1 > y1)
    {
        error += y2;
        y2 += grid;
        error += y2;

        if (error >= 0)
        {
            x2 -= grid;
            error -= x2;
            error -= x2;
        }

        if (x2 <= y2)
        {
            x2 = end;
            y2 = end;
            i = step;
        }

        if (++i >= step)
        {
            // drawing n triangles at a time
            circTriangleN(cx, cy, r, x1, y1, x2, y2, sa, n);
            i = 0;
            x1 = x2;
            y1 = y2;
        }
    }
}


Tree *Widget::circle(Tree *self, double cx, double cy, double r)
// ----------------------------------------------------------------------------
//     GL circle centered around (cx,cy), radius r
// ----------------------------------------------------------------------------
{
    glBegin(state.polygonMode);
    circSectorN(cx, cy, r, 0, 8);
    glEnd();

    return NULL;
}


Tree *Widget::circsector(Tree *self, double cx, double cy, double r, 
                         double a, double b)
// ----------------------------------------------------------------------------
//     GL circular sector centered around (cx,cy), radius r and two angles a, b
// ----------------------------------------------------------------------------
{
    while (b < a)
    {
        b += 360;
    }
    int n = int((b-a) / 45);    // Number of 1/8th of circle sectors to draw
    if (n > 8)
    {
        n = 8;
    }

    while (a < 0)
    {
        a += 360;
    }
    int sa = (int(a / 45) % 8); // Starting sector

    glBegin(state.polygonMode);
    circSectorN(cx, cy, r, sa, n);
    glEnd();

    return NULL;
 }



Tree *Widget::roundrect(Tree *self, double cx, double cy, 
                        double w, double h, double r)
// ----------------------------------------------------------------------------
//     GL rounded rectangle centered around (cx,cy), width w, height h and 
//     radius r for the corners
// ----------------------------------------------------------------------------
{
    if (r <= 0) return rectangle(self, cx, cy, w, h);
    if (r > w / 2) r = w / 2;
    if (r > h / 2) r = h / 2;

    glBegin(state.polygonMode);

    circSectorN(cx + w / 2.0 - r, cy + h / 2.0 - r, r, 0, 2);

    glVertex2f(cx - w / 2.0 + r, cy + h / 2.0 - r);
    glVertex2f(cx + w / 2.0 - r, cy + h / 2.0 - r);
    glVertex2f(cx + w / 2.0 - r, cy + h / 2.0);
    
    glVertex2f(cx + w / 2.0 - r, cy + h / 2.0);
    glVertex2f(cx - w / 2.0 + r, cy + h / 2.0);
    glVertex2f(cx - w / 2.0 + r, cy + h / 2.0 - r);
    
    circSectorN(cx - w / 2.0 + r, cy + h / 2.0 - r, r, 2, 2);

    glVertex2f(cx - w / 2.0, cy - h / 2.0 + r);
    glVertex2f(cx + w / 2.0, cy - h / 2.0 + r);
    glVertex2f(cx + w / 2.0, cy + h / 2.0 - r);
    
    glVertex2f(cx + w / 2.0, cy + h / 2.0 - r);
    glVertex2f(cx - w / 2.0, cy + h / 2.0 - r);
    glVertex2f(cx - w / 2.0, cy - h / 2.0 + r);
    
    circSectorN(cx - w / 2.0 + r, cy - h / 2.0 + r, r, 4, 2);

    glVertex2f(cx - w / 2.0 + r, cy - h / 2.0);
    glVertex2f(cx + w / 2.0 - r, cy - h / 2.0);
    glVertex2f(cx + w / 2.0 - r, cy - h / 2.0 + r);
    
    glVertex2f(cx + w / 2.0 - r, cy - h / 2.0 + r);
    glVertex2f(cx - w / 2.0 + r, cy - h / 2.0 + r);
    glVertex2f(cx - w / 2.0 + r, cy - h / 2.0);
    
    circSectorN(cx + w / 2.0 - r, cy - h / 2.0 + r, r, 6, 2);
   
    glEnd();

    return NULL;
}


Tree *Widget::rectangle(Tree *self, double cx, double cy, 
                        double w, double h)
// ----------------------------------------------------------------------------
//     GL rectangle centered around (cx,cy), width w, height h
// ----------------------------------------------------------------------------
{
    return NULL;
}


Tree *Widget::fromCm(Tree *self, double cm)
// ----------------------------------------------------------------------------
//   Convert from cm to pixels
// ----------------------------------------------------------------------------
{
    RREAL(cm * logicalDpiX() * (1.0 / 2.54));
}


Tree *Widget::fromMm(Tree *self, double mm)
// ----------------------------------------------------------------------------
//   Convert from mm to pixels
// ----------------------------------------------------------------------------
{
    RREAL(mm * logicalDpiX() * (0.1 / 2.54));
}


Tree *Widget::fromIn(Tree *self, double in)
// ----------------------------------------------------------------------------
//   Convert from inch to pixels
// ----------------------------------------------------------------------------
{
    RREAL(in * logicalDpiX());
}


Tree *Widget::fromPt(Tree *self, double pt)
// ----------------------------------------------------------------------------
//   Convert from pt to pixels
// ----------------------------------------------------------------------------
{
    RREAL(pt * logicalDpiX() * (1.0 / 72.0));
}


Tree *Widget::fromPx(Tree *self, double px)
// ----------------------------------------------------------------------------
//   Convert from pixel to pixels (trivial)
// ----------------------------------------------------------------------------
{
    RREAL(px);
}

TAO_END
