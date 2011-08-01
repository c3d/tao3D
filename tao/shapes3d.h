#ifndef SHAPES3D_H
#define SHAPES3D_H
// ****************************************************************************
//  shapes3d.h                                                      Tao project
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
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "shapes.h"

TAO_BEGIN

struct Shape3 : Shape
// ----------------------------------------------------------------------------
//   Base class for all 3D shapes, just in case
// ----------------------------------------------------------------------------
{
    Shape3(): Shape() {}
    virtual void        DrawSelection(Layout *layout);

protected:
    bool                setFillColor(Layout *where);
    bool                setLineColor(Layout *where);
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
    std::vector<Point3> vertices;
    std::vector<Point3> normals;
    std::vector<Point>  textures;

protected:
    Vector3&            calculateNormal(const Point3& v1,const Point3& v2, const Point3& v3);
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
    MeshBased(const Box3 &bounds) : Cube(bounds) {}
    void Draw(Mesh *mesh, Layout *where);
};

struct Sphere : MeshBased
// ----------------------------------------------------------------------------
//   Draw a sphere or ellipsoid
// ----------------------------------------------------------------------------
{
    Sphere(Box3 bounds, uint sl, uint st) : MeshBased(bounds),
                                            slices(sl), stacks(st) {}
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
    Torus(Box3 bounds, uint sl, uint st, double r) : MeshBased(bounds),
                                                     slices(sl), stacks(st), ratio(r) {}
    virtual void        Draw(Layout *where);

private:
    uint    slices, stacks;
    double  ratio;

private:
    // Cache of unit radius torus
    // (they differ by the number of subdivisions and their ratio)

#define EPSILON 0.001
    struct Key
    {
        Key(uint slices, uint stacks, double ratio): slices(slices), stacks(stacks), ratio(ratio) {}
        uint slices, stacks;
        double ratio;

        bool operator==(const Key &o) const
        {
            return ((slices == o.slices) && (stacks == o.stacks) && (fabs(ratio - o.ratio) < EPSILON));
        }

        bool operator<(const Key &o) const
        {
            if((*this) == o)
                return false;

            return (((slices < o.slices) || (stacks < o.stacks)) || (ratio < o.ratio));
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
    Cone(Box3 bounds, double tipRatio = 0.0) : MeshBased(bounds), ratio(tipRatio) {}
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

TAO_END

#endif // SHAPES3D_H
