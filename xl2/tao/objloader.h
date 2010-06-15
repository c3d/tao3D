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
#include "drawing.h"
#include "coords3d.h"
#include "coords.h"

TAO_BEGIN

struct Object3D : XL::Info
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
    void Draw(Layout *);
    Box3 Bounds(Layout *w);

public:
    // Types
    typedef Point3  Vertex;
    typedef Vector3 Normal;
    typedef Point3  TexCoord;
    struct  Face
    {
        std::vector<uint> vertex, texCoord, normal;
        uint materialId;
    };

    typedef std::vector<Vertex>         Vertices;
    typedef std::vector<Normal>         Normals;
    typedef std::vector<TexCoord>       TexCoords;
    typedef std::vector<Face>           Faces;

public:
    // Representation of an object
    Vertices    vertices;
    Normals     normals;
    TexCoords   texCoords;
    Faces       faces;
    Box3        bounds;
};


struct Object3DDrawing : Drawing
// ----------------------------------------------------------------------------
//   We draw a 3D object by reference
// ----------------------------------------------------------------------------
{
    Object3DDrawing (Object3D *obj): object(obj) {}
    ~Object3DDrawing() {}

    virtual void        Draw(Layout *l)         { object->Draw(l); }
    virtual void        DrawSelection(Layout *l){ object->Draw(l); }
    virtual void        Identify(Layout *l)     { object->Draw(l); }
    virtual Box3        Bounds(Layout *l)       { return object->Bounds(l); }
    virtual Box3        Space(Layout *l)        { return object->Bounds(l); }

public:
    Object3D *          object;
};

TAO_END

#endif // OBJLOADER_H

