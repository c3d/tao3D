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
#include "application.h"
#include "gl_keepers.h"


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
{
    where->ClearPolygonOffset();
    return Shape::setFillColor(where);
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

    static GLdouble textures[][2] = {
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

    GL.EnableClientState(GL_VERTEX_ARRAY);
    GL.VertexPointer(3, GL_DOUBLE, 0, vertices);

    // Set normals only if we have lights or shaders
    if(where->currentLights || where->programId)
    {
        GL.Sync(STATE_lights);
        GL.EnableClientState(GL_NORMAL_ARRAY);
        GL.NormalPointer(GL_FLOAT, 0, normals);
    }

    //Active texture coordinates for all used units
    enableTexCoord(&textures[0][0], ~0ULL);
    setTexture(where);
    GL.LoadMatrix();

    // Draw filled faces
    if (setFillColor(where))
        GL.DrawArrays(GL_QUADS, 0, 24);

    // Draw wireframe
    if (setLineColor(where))
        for (uint face = 0; face < 6; face++)
            GL.DrawArrays(GL_LINE_LOOP, 4*face, 4);
    
    // Disable texture coordinates after drawing
    disableTexCoord(~0ULL);

    if(where->currentLights || where->programId)
        GL.DisableClientState(GL_NORMAL_ARRAY);

    GL.DisableClientState(GL_VERTEX_ARRAY);
}



// ============================================================================
//
//    Mesh
//
// ============================================================================

void MeshBased::Draw(Mesh *mesh, Layout *where)
// ----------------------------------------------------------------------------
//    Draw the mesh within the bounding box
// ----------------------------------------------------------------------------
{
    Point3 p = bounds.Center() + where->Offset();

    GLAllStateKeeper save;

    // Set Vertices
    GL.EnableClientState(GL_VERTEX_ARRAY);
    GL.VertexPointer(3, GL_DOUBLE, 0, &mesh->vertices[0].x);

    // Set normals only if we have lights or shaders
    if(where->currentLights || where->programId)
    {
        GL.Sync(STATE_lights);
        GL.Enable(GL_NORMALIZE);
        GL.EnableClientState(GL_NORMAL_ARRAY);
        GL.NormalPointer(GL_DOUBLE, 0, &mesh->normals[0].x);
    }

    //Active texture coordinates for all used units
    enableTexCoord(&mesh->textures[0].x, ~0ULL);
    GL.Translate(p.x, p.y, p.z);
    GL.Scale(bounds.Width(), bounds.Height(), bounds.Depth());
    GL.LoadMatrix();

    // Apply textures
    setTexture(where);

    scale v = where->visibility * where->fillColor.alpha;
    bool drawBackFaces = where->programId || !culling || v != 1.0;
    // Optimize drawing of convex
    // shapes in case of no shaders thanks to
    // backface culling (doesn't need to draw back faces)
    GL.Enable(GL_CULL_FACE);
    if (drawBackFaces)
    {
        // Use painter algorithm to apply correctly
        // transparency on shapes
        // This was made necessary by Bug #1403.
        GL.CullFace(GL_FRONT);

        // Read Only mode of depth buffer
        if (v != 1.0)
            GL.DepthMask(false);

        if (setFillColor(where))
            GL.DrawArrays(GL_QUAD_STRIP, 0, mesh->textures.size());
        if (setLineColor(where))
            GL.DrawArrays(GL_LINE_LOOP, 0, mesh->textures.size());
    }

    // Draw the stuff in the front
    GL.CullFace(GL_BACK);
    GL.DepthMask(true);

    if (setFillColor(where))
        GL.DrawArrays(GL_QUAD_STRIP, 0, mesh->textures.size());
    if (setLineColor(where))
        GL.DrawArrays(GL_LINE_LOOP, 0, mesh->textures.size());

    GL.Disable(GL_CULL_FACE);

    // Disable texture coordinates
    disableTexCoord(~0ULL);

    // Disable normals
    if (where->currentLights || where->programId)
    {
        GL.Disable(GL_NORMALIZE);
        GL.DisableClientState(GL_NORMAL_ARRAY);
    }

    GL.DisableClientState(GL_VERTEX_ARRAY);
}



// ============================================================================
//
//    Sphere shape
//
// ============================================================================

SphereMesh::SphereMesh(uint slices, uint stacks)
// ----------------------------------------------------------------------------
//    Construct a unit sphere with given number of slices and stacks
// ----------------------------------------------------------------------------
{
    double radius = 0.5;
    for (uint j = 0; j < stacks; j++)
    {
        GLfloat phi      = M_PI * j / stacks;
        GLfloat incr_phi = M_PI * (j + 1) / stacks;

        // Compute phi components
        float sinPhi     =  sin(phi);
        float cosPhi     =  cos(phi);
        float sinIncrPhi =  sin(incr_phi);
        float cosIncrPhi =  cos(incr_phi);

        for (uint i = 0; i <= slices; i++)
        {
            GLfloat theta  = 2 * M_PI * i / slices;
            // Compute teta components (add an offset to be adaptated to the
            // old version)
            float sinTheta = sin(theta - M_PI/2);
            float cosTheta = cos(theta - M_PI/2) ;

            // First vertex
            textures.push_back(Point(1 - (double) i / slices,
                                     1 - (double) (j+1) / stacks));
            normals.push_back(Vector3(cosTheta * sinIncrPhi,
                                      cosIncrPhi,
                                      sinTheta * sinIncrPhi).Normalize());
            vertices.push_back(Point3(radius * cosTheta * sinIncrPhi,
                                      radius * cosIncrPhi,
                                      radius * sinTheta * sinIncrPhi));

            // Second vertex
            textures.push_back(Point(1 - (double) i / slices,
                                     1 - (double) j / stacks));
            normals.push_back(Vector3(cosTheta * sinPhi,
                                      cosPhi,
                                      sinTheta * sinPhi).Normalize());
            vertices.push_back(Point3(radius * cosTheta * sinPhi,
                                      radius * cosPhi,
                                      radius * sinTheta * sinPhi));
        }
    }
}


Sphere::SphereCache Sphere::cache;

void Sphere::Draw(Layout *where)
// ----------------------------------------------------------------------------
//    Draw the sphere within the bounding box
// ----------------------------------------------------------------------------
{
    Mesh * mesh = NULL;
    Key key(slices, stacks);
    SphereCache::iterator found = cache.find(key);
    if (found == cache.end())
    {
        // Prune the map if it gets too big
        while (cache.size() > MAX_SPHERES)
        {
            SphereCache::iterator first = cache.begin();
            delete (*first).second;
            cache.erase(first);
        }
        mesh = new SphereMesh(slices, stacks);
        cache[key] = mesh;
    }
    else
    {
        mesh = (*found).second;
    }

    MeshBased::Draw(mesh, where);
}



// ============================================================================
//
//    Torus shape
//
// ============================================================================

TorusMesh::TorusMesh(uint slices, uint stacks, double ratio)
// ----------------------------------------------------------------------------
//    Construct a unit torus with given number of slices, stacks and ratio
// ----------------------------------------------------------------------------
{
    double minRadius = ratio * 0.25;
    double majRadius = 0.25;
    double thickness = 0.25;

    for (uint j = 0; j < stacks; j++) {
        GLfloat phi      = 2 * M_PI * j / stacks;
        GLfloat incr_phi = 2 * M_PI * (j + 1) / stacks;

        // Compute phi components
        float sinPhi     =  sin(phi);
        float cosPhi     =  cos(phi);
        float sinIncrPhi =  sin(incr_phi);
        float cosIncrPhi =  cos(incr_phi);

        for (uint i = 0; i <= slices; i++)
        {
            GLfloat theta  = 2 * M_PI * i / slices;
            // Compute teta components
            float sinTheta = sin(theta);
            float cosTheta = cos(theta);
            float radius1 = majRadius + minRadius * cosIncrPhi;
            float radius2 = majRadius + minRadius * cosPhi;

            // First vertex
            textures.push_back(Vector((double) i / slices,
                                      (double) (j+1) / stacks));
            normals.push_back(Vector3( cosTheta * cosIncrPhi,
                                       sinTheta * cosIncrPhi,
                                       sinIncrPhi).Normalize());
            vertices.push_back(Vector3(radius1 * cosTheta,
                                       radius1 * sinTheta,
                                       (thickness * sinIncrPhi)));

            // Second vertex
            textures.push_back(Vector((double) i / slices,
                                      (double) j / stacks));
            normals.push_back(Vector3(cosTheta * cosPhi,
                                      sinTheta * cosPhi,
                                      sinPhi).Normalize());
            vertices.push_back(Vector3(radius2 * cosTheta,
                                       radius2 * sinTheta,
                                       (thickness * sinPhi)));
        }
    }
}


Torus::TorusCache Torus::cache;

void Torus::Draw(Layout *where)
// ----------------------------------------------------------------------------
//    Draw the torus within the bounding box
// ----------------------------------------------------------------------------
{
    Mesh * mesh = NULL;
    Key key(slices, stacks, ratio);
    TorusCache::iterator found = cache.find(key);
    if (found == cache.end())
    {
        // Prune the map if it gets too big
        while (cache.size() > MAX_TORUS)
        {
            TorusCache::iterator first = cache.begin();
            delete (*first).second;
            cache.erase(first);
        }
        mesh = new TorusMesh(slices, stacks, ratio);
        cache[key] = mesh;
    }
    else
    {
        mesh = (*found).second;
    }

    MeshBased::Draw(mesh, where);
}



// ============================================================================
//
//    Cone shape
//
// ============================================================================

ConeMesh::ConeMesh(double ratio)
// ----------------------------------------------------------------------------
//    Construct a (possibly truncated) unit cone - Limit case is a cylinder
// ----------------------------------------------------------------------------
{
    for (double a = 0; a <= 2 * M_PI; a += M_PI / 30)
    {
        double ca = cos(a);
        double sa = sin(a);

        double s = a / (2 * M_PI);
        textures.push_back(Point(s, 1));
        // normals.push_back(Vector3(ca, sa, 0));
        vertices.push_back(Point3(ca * ratio, sa * ratio, 0.5));

        textures.push_back(Point(s, 0));
        // normals.push_back(Vector3(ca, sa, 0));
        vertices.push_back(Point3(ca, sa, -0.5));
    }

    // Compute normal of each vertex according to those calculate for
    // neighbouring faces
    // NOTE: First and last normals are the same because of QUAD_STRIP
    Vector3 previousFaceNorm, nextFaceNorm;
    Triangle lastTriangle(vertices[vertices.size() - 3],
                          vertices[vertices.size() - 1],
                          vertices[0]);
    Triangle firstTriangle(vertices[0],
                           vertices[1],
                           vertices[3]);

    previousFaceNorm = lastTriangle.computeNormal();
    nextFaceNorm     = firstTriangle.computeNormal();
    normals.push_back(((previousFaceNorm + nextFaceNorm)/2));
    normals.push_back(((previousFaceNorm + nextFaceNorm)/2));

    for(unsigned int i = 2; i < vertices.size() - 2; i +=2)
    {
        previousFaceNorm = nextFaceNorm;
        Triangle triangle(vertices[i],
                          vertices[i + 1],
                          vertices[i + 3]);
        nextFaceNorm = triangle.computeNormal();

        normals.push_back(((previousFaceNorm + nextFaceNorm)/2));
        normals.push_back(((previousFaceNorm + nextFaceNorm)/2));
    }
    normals.push_back(normals[0]);
    normals.push_back(normals[0]);
}


Cone::ConeCache Cone::cache;

void Cone::Draw(Layout *where)
// ----------------------------------------------------------------------------
//    Draw the cone within the bounding box
// ----------------------------------------------------------------------------
{
    Mesh * mesh = NULL;
    Key key(ratio);
    ConeCache::iterator found = cache.find(key);
    if (found == cache.end())
    {
        // Prune the map if it gets too big
        while (cache.size() > MAX_CONES)
        {
            ConeCache::iterator first = cache.begin();
            delete (*first).second;
            cache.erase(first);
        }
        mesh = new ConeMesh(ratio);
        cache[key] = mesh;
    }
    else
    {
        mesh = (*found).second;
    }

    MeshBased::Draw(mesh, where);
}

TAO_END



// ****************************************************************************
// 
//    Code generation from shapes3d.tbl
// 
// ****************************************************************************

#include "graphics.h"
#include "opcodes.h"
#include "options.h"
#include "widget.h"
#include "types.h"
#include "drawing.h"
#include "layout.h"
#include "module_manager.h"
#include <iostream>


// ============================================================================
//
//    Top-level operation
//
// ============================================================================

#include "widget.h"

using namespace XL;

#include "opcodes_declare.h"
#include "shapes3d.tbl"

namespace Tao
{

#include "shapes3d.tbl"


void EnterShapes3D()
// ----------------------------------------------------------------------------
//   Enter all the basic operations defined in attributes.tbl
// ----------------------------------------------------------------------------
{
    XL::Context *context = MAIN->context;
#include "opcodes_define.h"
#include "shapes3d.tbl"
}


void DeleteShapes3D()
// ----------------------------------------------------------------------------
//   Delete all the global operations defined in attributes.tbl
// ----------------------------------------------------------------------------
{
#include "opcodes_delete.h"
#include "shapes3d.tbl"
}

}
