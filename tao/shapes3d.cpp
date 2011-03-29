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
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Lionel Schaffhauser <lionel@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "shapes3d.h"
#include "layout.h"
#include "widget.h"
#include "tao_gl.h"

TAO_BEGIN

void Shape3::DrawSelection(Layout *layout)
// ----------------------------------------------------------------------------
//   Draw a selection for 3-dimensional objects
// ----------------------------------------------------------------------------
{
    Widget *widget = layout->Display();
    uint sel = widget->selected(layout);
    if (sel)
    {
        Box3 bounds = Bounds(layout);
        XL::Save<Point3> zeroOffset(layout->offset, Point3(0,0,0));
        widget->drawSelection(layout, bounds, "3D_selection", 0);
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
        scale v = where->visibility * color.alpha;
        if (v > 0.0)
        {
            if (!where->hasMaterial)
                glColor4f(color.red, color.green, color.blue, v);
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
        scale v = where->visibility * color.alpha;
        if (v > 0.0 && width > 0.0)
        {
            if (!where->hasMaterial)
                glColor4f(color.red, color.green, color.blue, v);
            return true;
        }
    }
    return false;
}

Vector3& Shape3::calculateNormal(const Point3& v1,const Point3& v2,const Point3& v3)
// ----------------------------------------------------------------------------
//    Compute normal of specified triangle
// ----------------------------------------------------------------------------
{
    Point3 a;
    Point3 b;
    Vector3 normal;

    a.x = v1.x - v2.x;
    a.y = v1.y - v2.y;
    a.z = v1.z - v2.z;

    b.x = v2.x - v3.x;
    b.y = v2.y - v3.y;
    b.z = v2.z - v3.z;

    normal.x = (a.y * b.z) - (a.z * b.y);
    normal.y = (a.z * b.x) - (a.x * b.z);
    normal.z = (a.x * b.y) - (a.y * b.x);

    return normal.Normalize();
}

Box3 Cube::Bounds(Layout *where)
// ----------------------------------------------------------------------------
//   Return the bounding box for a 3D shape
// ----------------------------------------------------------------------------
{
    return bounds + where->offset;
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

    static GLfloat normals[][3] =
    {
        { 0,  0, -1}, { 0,  0, -1}, { 0,  0, -1}, { 0,  0, -1},
        { 0,  0,  1}, { 0,  0,  1}, { 0,  0,  1}, { 0,  0,  1},
        { 0, -1,  0}, { 0, -1,  0}, { 0, -1,  0}, { 0, -1,  0},
        { 0,  1,  0}, { 0,  1,  0}, { 0,  1,  0}, { 0,  1,  0},
        {-1,  0,  0}, {-1,  0,  0}, {-1,  0,  0}, {-1,  0,  0},
        { 1,  0,  0}, { 1,  0,  0}, { 1,  0,  0}, { 1,  0,  0},
    };

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glVertexPointer(3, GL_DOUBLE, 0, vertices);
    glNormalPointer(GL_FLOAT, 0, normals);
    std::map<uint, TextureState>::iterator it;
    for(it = where->fillTextures.begin(); it != where->fillTextures.end(); it++)
    {
        if(((*it).second).texId)
        {
            glClientActiveTexture( GL_TEXTURE0 + (*it).first );
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer(2, GL_INT, 0, textures);
        }
    }

    setTexture(where);
    if (setFillColor(where))
        glDrawArrays(GL_QUADS, 0, 24);
    if (setLineColor(where))
        for (uint face = 0; face < 6; face++)
            glDrawArrays(GL_LINE_LOOP, 4*face, 4);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);
    for(it = where->fillTextures.begin(); it != where->fillTextures.end(); it++)
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
    gluQuadricNormals (q, GLU_SMOOTH);
    glPushMatrix();
    glPushAttrib(GL_ENABLE_BIT);
    glEnable(GL_NORMALIZE);
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
    glPopAttrib();
    glPopMatrix();
    gluDeleteQuadric(q);
}


void Cone::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Draw the cone
// ----------------------------------------------------------------------------
{
    Point3 p = bounds.Center() + where->Offset();
    scale w = bounds.Width();
    scale h = bounds.Height();
    scale d = bounds.Depth();
    std::vector<Point3> tex;
    std::vector<Point3> geom;
    std::vector<Vector3> norm;

    for (double a = 0; a <= 2 * M_PI; a += M_PI / 10)
    {
        double ca = cos(a);
        double sa = sin(a);
        tex.push_back(Point3(0.5 + 0.5 * ca, 0.5 + 0.5 * sa, 0));
        geom.push_back(Point3(p.x + w/2* ca, p.y + h/2 * sa, p.z - d/2));

        tex.push_back(Point3(0.5 + 0.5 * ca, 0.5 + 0.5 * sa, 1));
        geom.push_back(Point3(p.x + w/2* ca * ratio, p.y + h/2 * sa * ratio, p.z + d/2));
    }

    // Compute normal of each vertex according to those calculate for neighbouring faces
    // NOTE: First and last normals are the same because of QUAD_STRIP
    Vector3 previousFaceNorm, nextFaceNorm;
    previousFaceNorm = calculateNormal(geom[geom.size() - 2], geom[geom.size() - 1], geom[0]);
    nextFaceNorm = calculateNormal(geom[0], geom[1], geom[2]);
    norm.push_back(((previousFaceNorm + nextFaceNorm)/2));
    norm.push_back(((previousFaceNorm + nextFaceNorm)/2));
    for(unsigned int i = 2; i < geom.size() - 2; i +=2)
    {
        previousFaceNorm = nextFaceNorm;

        if(i < geom.size() - 2)
         nextFaceNorm = calculateNormal(geom[i], geom[i + 1], geom[i + 2]);
        else
         nextFaceNorm = calculateNormal(geom[geom.size() - 2], geom[geom.size() - 1], geom[0]);

        norm.push_back(((previousFaceNorm + nextFaceNorm)/2));
        norm.push_back(((previousFaceNorm + nextFaceNorm)/2));
    }
    norm.push_back(norm[0]);
    norm.push_back(norm[0]);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    glVertexPointer(3, GL_DOUBLE, 0, &geom[0].x);
    glNormalPointer(GL_DOUBLE, 0, &norm[0].x);
    std::map<uint, TextureState>::iterator it;
    for(it = where->fillTextures.begin(); it != where->fillTextures.end(); it++)
    {
        if(((*it).second).texId)
        {
            glClientActiveTexture( GL_TEXTURE0 + (*it).first );
            glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            glTexCoordPointer(3, GL_DOUBLE, 0, &tex[0].x);
        }
    }
    setTexture(where);
    if (setFillColor(where))
        glDrawArrays(GL_QUAD_STRIP, 0, geom.size());
    if (setLineColor(where))
        // REVISIT: Inefficient and incorrect with alpha
        for (uint i = 3; i <= geom.size(); i++)
            glDrawArrays(GL_LINE_LOOP, 0, i);
            
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    for(it = where->fillTextures.begin(); it != where->fillTextures.end(); it++)
        glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}

TAO_END
