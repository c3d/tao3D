#ifndef SHAPES3D_H
#define SHAPES3D_H
// *****************************************************************************
// shapes3d.h                                                      Tao3D project
// *****************************************************************************
//
// File description:
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
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2010, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010-2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2012, Jérôme Forissier <jerome@taodyne.com>
// (C) 2010, Lionel Schaffhauser <lionel@taodyne.com>
// (C) 2011, Soulisse Baptiste <soulisse@polytech.unice.fr>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can r redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Tao3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Tao3D, in a file named COPYING.
// If not, see <https://www.gnu.org/licenses/>.
// *****************************************************************************

#include "shapes.h"

TAO_BEGIN

struct Shape3 : Shape
// ----------------------------------------------------------------------------
//   Base class for all 3D shapes, just in case
// ----------------------------------------------------------------------------
{
    Shape3(): Shape() {}
    virtual void        Identify(Layout * l) { Draw(l); }
    virtual void        DrawSelection(Layout *layout);

    static bool         setFillColor(Layout *where);
    static void         enableCulling(Layout *where);
};


struct Cube : Shape3
// ----------------------------------------------------------------------------
//   Draw a cube (OK, not really a cube, but the name is short)
// ----------------------------------------------------------------------------
{
    Cube(const Box3 &bounds): Shape3(), bounds(bounds) {}
    virtual void        Draw(Layout *where);
    virtual Box3        Bounds(Layout *);

    Box3 bounds;
};


struct Mesh
// ----------------------------------------------------------------------------
//   Generic mesh data
// ----------------------------------------------------------------------------
{
    std::vector<Point3>  vertices;
    std::vector<Vector3> normals;
    std::vector<Point>   textures;
};


struct SphereMesh : Mesh
// ----------------------------------------------------------------------------
//   A unit-radius sphere, represented as a mesh
// ----------------------------------------------------------------------------
{
    SphereMesh(uint slices, uint stacks);
};


struct TorusMesh : Mesh
// ----------------------------------------------------------------------------
//   A unit-radius sphere, represented as a mesh
// ----------------------------------------------------------------------------
{
    TorusMesh(uint slices, uint stacks, double r);
};


struct ConeMesh : Mesh
// ----------------------------------------------------------------------------
//   A unit-radius sphere, represented as a mesh
// ----------------------------------------------------------------------------
{
    ConeMesh(double r);
};


struct MeshBased : Cube
// ----------------------------------------------------------------------------
//   Common drawing code for mesh-based shapes
// ----------------------------------------------------------------------------
{
    MeshBased(const Box3 &bounds, bool culling)
        : Cube(bounds), culling(culling) {}
    void Draw(Layout *where) { Cube::Draw(where); }
    void Draw(Mesh *mesh, Layout *where);

    // Define if mesh needs backface culling
    bool culling;
};


struct Sphere : MeshBased
// ----------------------------------------------------------------------------
//   Draw a sphere or ellipsoid
// ----------------------------------------------------------------------------
{
    Sphere(Box3 bounds, uint sl, uint st)
        : MeshBased(bounds, true), slices(sl), stacks(st) {}
    virtual void        Draw(Layout *where);

private:
    uint    slices, stacks;

private:
    // Cache of unit radius spheres
    // (they differ by the number of subdivisions onhy)

    enum { MAX_SPHERES = 10 };
    typedef std::pair<uint, uint> Key;
    typedef std::map<Key, Mesh *> SphereCache;

    static SphereCache cache;
};


struct Torus : MeshBased
// ----------------------------------------------------------------------------
//   Draw a torus
// ----------------------------------------------------------------------------
{
    Torus(Box3 bounds, uint sl, uint st, double r)
        : MeshBased(bounds, true), slices(sl), stacks(st), ratio(r) {}
    virtual void        Draw(Layout *where);

private:
    uint    slices, stacks;
    double  ratio;

private:
    // Cache of unit radius torus
    // (they differ by the number of subdivisions and their ratio)

    struct Key
    {
        Key(uint slices, uint stacks, double ratio)
            : slices(slices), stacks(stacks), ratio(ratio) {}
        uint slices, stacks;
        double ratio;

        bool operator==(const Key &o) const
        {
            return slices == o.slices && stacks == o.stacks && ratio == o.ratio;
        }

        bool operator<(const Key &o) const
        {
            return slices < o.slices || stacks < o.stacks || ratio < o.ratio;
        }
    };

    typedef std::map<Key, Mesh *> TorusCache;

    enum { MAX_TORUS = 10 };
    static TorusCache cache;
};


struct Cone : MeshBased
// ----------------------------------------------------------------------------
//   Draw a (possibly truncated) cone - Limit case is a cylinder
// ----------------------------------------------------------------------------
{
    Cone(Box3 bounds, double tipRatio = 0.0)
        : MeshBased(bounds, false), ratio(tipRatio) {}
    virtual void        Draw(Layout *where);

private:
    double  ratio;

private:
    // Cache of unit radius cones
    // (they differ by their ratio onhy)

    enum { MAX_CONES = 10 };
    typedef double Key;
    typedef std::map<Key, Mesh *> ConeCache;

    static ConeCache cache;
};


// ============================================================================
//
//   Entering shapes in the symbols table
//
// ============================================================================

extern void EnterShapes3D();
extern void DeleteShapes3D();

TAO_END

#endif // SHAPES3D_H
