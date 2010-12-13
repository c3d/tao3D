#ifndef OBJLOADER_H
#define OBJLOADER_H
// ****************************************************************************
//  objloader.h                                                     Tao project
// ****************************************************************************
//
//   File Description:
//
//    A quick and dirty 3D object loaders (.obj format)
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
//  (C) 2010 Jérôme Forissier <jerome@taodyne.com>
//  (C) 2010 Catherine Burvelle <cathy@taodyne.com>
//  (C) 2010 Lionel Schaffhauser <lionel@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "tao_tree.h"
#include "shapes3d.h"
#include "coords3d.h"
#include "coords.h"

TAO_BEGIN

struct Object3D
// ----------------------------------------------------------------------------
//   Representation of a complete 3D object
// ----------------------------------------------------------------------------
{
    // Constructor and destructor
    Object3D(kstring name = NULL);
    ~Object3D();

    // Loading an objet file
    void Load(kstring name);

    // Draw interface (called from Object3DDrawing)
    void Draw(Layout *l, coord x, coord y, coord z, scale w, scale h, scale d);
    Box3 Bounds(Layout *w);

    static Object3D *Object(text name);

protected:
    // Load a material library
    void LoadMtl(text name);
    uint MaterialId(text name);

public:
    // Types
    typedef Point3  Vertex;
    typedef Vector3 Normal, Reflectance;
    typedef Point3  TexCoord;
    struct  Face
    {
        std::vector<uint> vertex, texCoord, normal;
        uint materialId;
    };
    struct Material
    {
        Material(kstring name = "")
            : name(name), ambient(0.0, 0.0, 0.0), diffuse(0.0, 0.0, 0.0),
              specular(0.0, 0.0, 0.0), alpha(1.0), shininess(0.0) {}

        text         name;
        Reflectance  ambient, diffuse, specular;
        coord        alpha;
        coord        shininess;
    };

    typedef std::vector<Vertex>         Vertices;
    typedef std::vector<Normal>         Normals;
    typedef std::vector<TexCoord>       TexCoords;
    typedef std::vector<Face>           Faces;
    typedef std::vector<Material>       Materials;

public:
    // Representation of an object
    Vertices    vertices;
    Normals     normals;
    TexCoords   texCoords;
    Faces       faces;
    Materials   materials;
    Box3        bounds;
    uint        listID;
};


struct Object3DDrawing : Shape3
// ----------------------------------------------------------------------------
//   We draw a 3D object by reference
// ----------------------------------------------------------------------------
{
    Object3DDrawing (Object3D *obj,
                     coord x, coord y, coord z,
                     scale w, scale h, scale d)
        : object(obj), x(x), y(y), z(z), w(w), h(h), d(d) {}
    ~Object3DDrawing() {}

    virtual void        Draw(Layout *l);
    virtual void        DrawSelection(Layout *l){ Draw(l); }
    virtual void        Identify(Layout *l)     { Draw(l); }
    virtual Box3        Bounds(Layout *l)       { return object->Bounds(l); }
    virtual Box3        Space(Layout *l)        { return object->Bounds(l); }

public:
    Object3D *          object;
    coord               x, y, z;
    scale               w, h, d;
};

TAO_END

#endif // OBJLOADER_H

