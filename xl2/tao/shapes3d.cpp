// ****************************************************************************
//  shapes3d.cpp                                                    Tao project
// ****************************************************************************
// 
//   File Description:
// 
//     Drawing of elementary 3D geometry shapes
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

#include "shapes3d.h"
#include "layout.h"
#include "widget.h"
#include <QtOpenGL>

TAO_BEGIN

void Shape3::DrawSelection(Layout *layout)
// ----------------------------------------------------------------------------
//   Draw a selection for 3-dimensional objects
// ----------------------------------------------------------------------------
{
    Widget *widget = layout->Display();
    if (widget->selected(layout))
    {
        Color line(1.0, 0.0, 0.0, 0.5);
        Color fill(0.0, 0.7, 1.0, 0.1);
        XL::LocalSave<Color> saveLine(layout->lineColor, line);
        XL::LocalSave<Color> saveFill(layout->fillColor, fill);
        widget->drawSelection(layout, Bounds(layout) + layout->Offset(),
                              "3D_selection", 0);
    }
}

    
bool Shape3::setFillColor(Layout *where)
// ----------------------------------------------------------------------------
//    Set the fill color and texture according to the layout attributes
// ----------------------------------------------------------------------------
//    This is like in Shape, except that we don't increment polygon offset
{
    // Check if we have a non-transparent fill color
    if (where)
    {
        Color &color = where->fillColor;
        if (color.alpha > 0.0)
        {
            glColor4f(color.red, color.green, color.blue, color.alpha);
            return true;
        }
    }
    return false;
}


bool Shape3::setLineColor(Layout *where)
// ----------------------------------------------------------------------------
//    Set the outline color according to the layout attributes
// ----------------------------------------------------------------------------
//    This is like in Shape, except that we don't increment polygon offset
{
    // Check if we have a non-transparent outline color
    if (where)
    {
        Color &color = where->lineColor;
        scale width = where->lineWidth;
        if (color.alpha > 0.0 && width > 0.0)
        {
            glColor4f(color.red, color.green, color.blue, color.alpha);
            return true;
        }
    }
    return false;
}


Box3 Cube::Bounds(Layout *)
// ----------------------------------------------------------------------------
//   Return the bounding box for a 3D shape
// ----------------------------------------------------------------------------
{
    return bounds;
}


void Cube::Draw(Layout *where)
// ----------------------------------------------------------------------------
//    Draw the cube within the bounding box
// ----------------------------------------------------------------------------
{
    Box3 b = bounds + where->Offset();
    coord xl = bounds.lower.x;
    coord yl = bounds.lower.y;
    coord zl = bounds.lower.z;
    coord xu = bounds.upper.x;
    coord yu = bounds.upper.y;
    coord zu = bounds.upper.z;

    coord vertices[][3] =
    {
        {xl, yl, zl}, {xl, yu, zl}, {xu, yu, zl}, {xu, yl, zl},
        {xl, yl, zu}, {xu, yl, zu}, {xu, yu, zu}, {xl, yu, zu},
        {xl, yl, zl}, {xu, yl, zl}, {xu, yl, zu}, {xl, yl, zu},
        {xl, yu, zl}, {xl, yu, zu}, {xu, yu, zu}, {xu, yu, zl},
        {xl, yu, zl}, {xl, yl, zl}, {xl, yl, zu}, {xl, yu, zu},
        {xu, yl, zl}, {xu, yu, zl}, {xu, yu, zu}, {xu, yl, zu}
    };

    static GLint textures[][2] = {
        {1, 0}, {1, 1}, {0, 1}, {0, 0},
        {0, 0}, {1, 0}, {1, 1}, {0, 1},
        {0, 0}, {1, 0}, {1, 1}, {0, 1},
        {0, 1}, {0, 0}, {1, 0}, {1, 1},
        {0, 1}, {0, 0}, {1, 0}, {1, 1},
        {1, 0}, {1, 1}, {0, 1}, {0, 0}
    };

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(3, GL_DOUBLE, 0, vertices);
    glTexCoordPointer(2, GL_INT, 0, textures);
    setTexture(where);
    if (setFillColor(where))
        glDrawArrays(GL_QUADS, 0, 24);
    if (setLineColor(where))
        for (uint face = 0; face < 6; face++)
            glDrawArrays(GL_LINE_LOOP, 4*face, 4);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}


void Sphere::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Draw the sphere
// ----------------------------------------------------------------------------
{
    Point3 p = bounds.Center() + where->Offset();
    double radius = 0.5;

    GLUquadric *q = gluNewQuadric();
    gluQuadricTexture (q, true);
    glPushMatrix();
    glTranslatef(p.x, p.y, p.z);
    glRotatef(-90.0, 1.0, 0.0, 0.0);
    glScalef(bounds.Width(), bounds.Height(), bounds.Depth());

    setTexture(where);
    if (setFillColor(where))
    {
        gluQuadricDrawStyle(q, GLU_FILL);
        gluSphere(q, radius, slices, stacks);
    }
    if (setLineColor(where))
    {
        gluQuadricDrawStyle(q, GLU_LINE);
        gluSphere(q, radius, slices, stacks);
    }
    glPopMatrix();
    gluDeleteQuadric(q);
}


void Cone::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Draw the cone
// ----------------------------------------------------------------------------
{
    Point3 tip = bounds.Center() + where->Offset();
    scale w = bounds.Width();
    scale h = bounds.Height();
    scale d = bounds.Depth();
    std::vector<Point3> tex;
    std::vector<Point3> geom;

    tip.z -= d/2;
    tex.push_back(Point3(0.5, 0.5, 0));
    geom.push_back(Point3(tip.x, tip.y, tip.z));
    tip.z += d;
    for (double a = 0; a <= 2 * M_PI; a += M_PI / 10)
    {
        tex.push_back(Point3(0.5 + 0.5 * cos(a), 0.5 + 0.5 * sin(a), 1));
        geom.push_back(Point3(tip.x + w/2*cos(a), tip.y + h/2*sin(a), tip.z));
    }

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(3, GL_DOUBLE, 0, &geom[0].x);
    glTexCoordPointer(3, GL_DOUBLE, 0, &tex[0].x);

    setTexture(where);
    if (setFillColor(where))
        glDrawArrays(GL_TRIANGLE_FAN, 0, geom.size());
    if (setLineColor(where))
        // REVISIT: Inefficient and incorrect with alpha
        for (uint i = 3; i <= geom.size(); i++)
            glDrawArrays(GL_LINE_LOOP, 0, i);
            
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

TAO_END
