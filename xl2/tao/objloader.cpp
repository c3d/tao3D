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
#include "tao_utf8.h"
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <fstream>
#include <map>
#include <GL/glew.h>
#include <QFileInfo>

TAO_BEGIN

Object3D::Object3D(kstring name)
// ----------------------------------------------------------------------------
//   Initialize an object. If a name is given, load the file
// ----------------------------------------------------------------------------
    : listID(0)
{
    if (name)
        Load(name);
}


Object3D::~Object3D()
// ----------------------------------------------------------------------------
//   No specific destruction here
// ----------------------------------------------------------------------------
{
    if (listID)
        glDeleteLists(listID, 1);
}


void Object3D::Load(kstring name)
// ----------------------------------------------------------------------------
//    Load a Wavefront object file from disk
// ----------------------------------------------------------------------------
{
    std::ifstream input(name);
    uint          materialId = 0;
    uint          line       = 0;
    text          token;
    coord         x, y, z;
    char          buffer[256];
    char *        ptr;
    char *        end;
    bool          texture, normal;
    Face *        face;
    long          vIdx, tIdx, nIdx;
    text          material;
    int           c;

    while (input.good())
    {
        // Scan a new line of text
        ptr = buffer;
        end = buffer + sizeof(buffer) - 1;
        line++;

        do
        {
            c = input.get();
            if (c && ptr < end)
                *ptr++ = c;
        } while (input.good() && c != '\n' && c != '\r');
        *ptr++ = 0;

        // Skip spaces and blank lines
        ptr = buffer;
        while(isspace(*ptr))
            ptr++;
        if (!*ptr)
            continue;

        // Analyze input
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
                if (sscanf(++ptr, "%lf %lf %lf", &x, &y, &z) < 2)
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
                if (sscanf(ptr, "%lf %lf %lf", &x, &y, &z) != 3)
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


    // Now create a display list with the object data
    Faces::iterator             f;
    std::vector<uint>::iterator v, t, n;
    uint nv = vertices.size();
    uint nn = normals.size();
    uint nt = texCoords.size();

    listID = glGenLists(1);
    glNewList(listID, GL_COMPILE);

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
    glEndList();
}


void Object3D::Draw(Layout *where,
                    coord x, coord y, coord z,
                    scale w, scale h, scale d)
// ----------------------------------------------------------------------------
//   Draw the 3D object
// ----------------------------------------------------------------------------
{
    Vector3 offset = where->Offset();
    coord ox = x - bounds.Center().x;
    coord oy = y - bounds.Center().y;
    coord oz = z - bounds.Center().z;
    scale sx = w / bounds.Width();
    scale sy = h / bounds.Height();
    scale sz = d / bounds.Depth();

    glPushMatrix();
    glTranslatef(ox, oy, oz);
    glScalef(sx, sy, sz);
    glCallList(listID);
    glPopMatrix();
}


Box3 Object3D::Bounds(Layout *where)
// ----------------------------------------------------------------------------
//   Return the bounds of the loaded object
// ----------------------------------------------------------------------------
{
    return bounds + where->Offset();
}


Object3D *Object3D::Object(text name)
// ----------------------------------------------------------------------------
//   Maintain a list of object files currently in use
// ----------------------------------------------------------------------------
{
    typedef std::map<text, Object3D *> file_map;
    static file_map loaded;
    
    file_map::iterator found = loaded.find(name);
    if (found == loaded.end())
    {
        QString file = +name;
        QFileInfo fi(file);
        if (!fi.isReadable())
            fi.setFile("object:" + file);
        if (fi.isReadable())
        {
            text path = +fi.canonicalFilePath();
            found = loaded.find(path);
            if (found == loaded.end())
            {
                Object3D *object = new Object3D(path.c_str());
                loaded[name] = object;
                loaded[path] = object;
            }
        }
        else
        {
            // Record that we don't know how to load this guy
            loaded[name] = NULL;
        }

        // We should have a valid entry now
        found = loaded.find(name);
    }

    return (*found).second;
}


void Object3DDrawing::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Draw a 3D object using current texture and color
// ----------------------------------------------------------------------------
{
    setTexture(where);
    if (setFillColor(where))
        object->Draw(where, x,y,z, w,h,d);
}

TAO_END
