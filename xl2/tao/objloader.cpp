// ****************************************************************************
//  objloader.cpp                                                   Tao project
// ****************************************************************************
// 
//   File Description:
// 
// 
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

#include "objloader.h"
#include "layout.h"
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <fstream>
#include <GL/glew.h>

TAO_BEGIN

Object3D::Object3D(kstring name)
// ----------------------------------------------------------------------------
//   Initialize an object. If a name is given, load the file
// ----------------------------------------------------------------------------
{
    if (name)
        Load(name);
}


Object3D::~Object3D()
// ----------------------------------------------------------------------------
//   No specific destruction here
// ----------------------------------------------------------------------------
{}


void Object3D::Load(kstring name)
// ----------------------------------------------------------------------------
//    Load a Wavefront object file from disk
// ----------------------------------------------------------------------------
{
    std::ifstream input(name);
    text          token;
    coord         x, y, z;
    char          buffer[256];
    char *        ptr;
    char *        end;
    bool          texture, normal;
    uint          line = 0;
    Face *        face;
    long          vIdx, tIdx, nIdx;
    text          material;
    uint          materialId = 0;

    while (input.good())
    {
        input.getline(buffer, sizeof(buffer));
        ptr = buffer;
        line++;

        // Skip spaces and blank lines
        while(isspace(*ptr))
            ptr++;
        if (!*ptr)
            continue;

        char key = *ptr++;
        switch(key)
        {
        case '#':
            // Comment: Simply skip until end of line
            break;
        case 'v':
            x = y = z = 0;
            switch(*ptr)
            {
            case 't':
                // Texture coordinate
                if (sscanf(++ptr, "%lf %lf %lf", &x, &y, &z) != 3)
                    std::cerr << name << ":" << line << ":"
                              << "Invalid texture coordinate: " << ptr << '\n';
                texCoords.push_back(Point3(x,y,z));
                break;
            case 'n':
                // Normal coordinate
                if (sscanf(++ptr, "%lf %lf %lf", &x, &y, &z) != 3)
                    std::cerr << name << ":" << line << ":"
                              << "Invalid normal coordinate: " << ptr << '\n';
                normals.push_back(Point3(x,y,z));
                break;
            default:
                if (sscanf(ptr, "%lf %lf %lf", &x, &y, &z) < 2)
                    std::cerr << name << ":" << line << ":"
                              << "Invalid vertex coordinate: " << ptr << '\n';
                vertices.push_back(Point3(x,y,z));
                bounds |= Point3(x, y, z);
                break;
            } // Switch secondary 'v' key
            break;

        case 'f':
        case 'l':
            // Facet: f v1[/vt1][/vn1] v2[/vt2][/vn2]
            faces.push_back(Face());
            face = &faces.back();
            face->materialId = materialId;
            texture = false;
            normal = false;

            // The 'fo' variant was seen in some object files
            if (*ptr == 'o')
                *ptr++;

            do
            {
                vIdx = strtol(ptr, &end, 10);
                if (end == ptr)
                    break;

                if (vIdx < 0)
                    vIdx = vertices.size() + vIdx;

                face->vertex.push_back(vIdx);
                if (*end == '/')
                {
                    char *endTx;
                    tIdx = strtol(end+1, &endTx, 10);
                    if (endTx != end+1)
                    {
                        if (tIdx < 0)
                            tIdx = texCoords.size() + vIdx;
                        face->texCoord.push_back(tIdx);
                        if (*endTx == '/')
                        {
                            nIdx = strtol(endTx+1, &end, 10);
                            if (nIdx < 0)
                                nIdx = normals.size() + nIdx;
                            if (end != endTx+1)
                                face->normal.push_back(nIdx);
                        }
                    }
                }

                uint sz = face->vertex.size();
                uint tsz = face->texCoord.size();
                uint nsz = face->normal.size();
                if ((tsz && tsz != sz) || (nsz && nsz != sz))
                {
                    std::cerr << name << ":" << line << ":"
                              << "Inconsistent facet: " << ptr << '\n';
                    break;
                }

                // Start over with next line
                ptr = end;
            } while (*ptr && *ptr != '\n');

            if (face->vertex.size() == 0)
                std::cerr << name << ":" << line << ":"
                          << "Invalid facet: " << ptr << '\n';

            break;

        case 'g':
            // Group
            break;

        case 'u':
            // Use material
            if (*ptr++ == 's' && *ptr++ == 'e' &&
                *ptr++ == 'm' && *ptr++ == 't' && *ptr++ == 'l')
            {
                
            }
            else
            {
                std::cerr << name << ":" << line << ":"
                          << "Invalid usemtl: " << ptr << '\n';
            }
            break;

        } // Switch key
    } // While loop
}


void Object3D::Draw(Layout *)
// ----------------------------------------------------------------------------
//   Draw the 3D object
// ----------------------------------------------------------------------------
{
    Faces::iterator             f;
    std::vector<uint>::iterator v, t, n;
    uint nv = vertices.size();
    uint nn = normals.size();
    uint nt = texCoords.size();

    for (f = faces.begin(); f != faces.end(); f++)
    {
        Face &face = *f;
        
        v = face.vertex.begin();
        t = face.texCoord.begin();
        n = face.normal.begin();

        GLuint mode = GL_POLYGON;
        switch (face.vertex.size())
        {
        case 1: mode = GL_POINTS; break;
        case 2: mode = GL_LINES; break;
        case 3: mode = GL_TRIANGLES; break;
        case 4: mode = GL_QUADS; break;
        }

        glBegin(mode);

        while(v != face.vertex.end())
        {
            if (t != face.texCoord.end())
                if (uint it = *t++)
                    if (it <= nt)
                        glTexCoord3dv(&texCoords[it-1].x);
            if (n != face.normal.end())
                if (uint in = *n++)
                    if (in <= nn)
                        glNormal3dv(&normals[in-1].x);
            if (uint iv = *v++)
                if (iv <= nv)
                    glVertex3dv(&vertices[iv-1].x);
        }

        glEnd();
    }
}


Box3 Object3D::Bounds(Layout *where)
// ----------------------------------------------------------------------------
//   Return the bounds of the loaded object
// ----------------------------------------------------------------------------
{
    return bounds + where->Offset();
}

TAO_END
