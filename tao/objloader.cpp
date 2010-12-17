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
#include "tao_gl.h"
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include <fstream>
#include <map>
#include <QFileInfo>

TAO_BEGIN

Object3D::Object3D(kstring name)
// ----------------------------------------------------------------------------
//   Initialize an object. If a name is given, load the file
// ----------------------------------------------------------------------------
    : listID(0), useCallList(false)
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
    uint          materialId = (uint)-1;
    uint          line       = 0;
    coord         x, y, z;
#  define         BUFSIZE      1024
    char *        buffer     = (char *)malloc(BUFSIZE * sizeof(char));
    char *        ptr;
    char *        end;
    bool          texture, normal;
    Face *        face;
    long          vIdx, tIdx, nIdx;
    int           c;
    char *        oldlocale = setlocale(LC_NUMERIC, "C");
    char          prev_eol  = '\0';

    IFTRACE(objloader)
        std::cerr << "Loading " << name << "\n";

    while (input.good())
    {
        // Scan a new line of text
        ptr = buffer;
        end = buffer + BUFSIZE - 1;
#       undef BUFSIZE
        line++;

        bool too_long = false;
        do
        {
            c = input.get();
            if (c)
            {
                if (ptr < end)
                    *ptr++ = c;
                else
                    too_long = true;
            }
        } while (input.good() && c != '\n' && c != '\r');
        *ptr++ = 0;
        if (too_long)
            std::cerr << name << ":" << line << ":"
                      << "Line too long\n";

        // Do not count CR+LF as 2 lines
        ptr = buffer;
        if ((*ptr == '\n' && prev_eol == '\r'))
              line--;
        prev_eol = '\0';
        if (c == '\n' || c == '\r')
            prev_eol = c;

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
                ptr++;

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
                text mtl(++ptr);
                size_t eol = mtl.find('\r');
                if (eol == mtl.npos)
                    eol = mtl.find('\n');
                if (eol != mtl.npos)
                    mtl.resize(eol);
                materialId = MaterialId(mtl);
            }
            else
            {
                std::cerr << name << ":" << line << ":"
                          << "Invalid usemtl: " << ptr << '\n';
            }
            break;

        case 'm':
            // Material library
            if (*ptr++ == 't' && *ptr++ == 'l' &&
                *ptr++ == 'l' && *ptr++ == 'i' && *ptr++ == 'b')
            {
                text mtl(++ptr);
                size_t eol = mtl.find('\r');
                if (eol == mtl.npos)
                    eol = mtl.find('\n');
                if (eol != mtl.npos)
                    mtl.resize(eol);
                if (mtl[0] != '/')
                {
                    text dir(name);
                    size_t pos = dir.rfind("/");
                    if (pos != dir.npos)
                    {
                        dir.resize(pos+1);
                        dir.append(mtl);
                        mtl = dir;
                    }
                }
                LoadMtl(mtl);
            }
            break;

        } // Switch key
    } // While loop


    if (useCallList)
    {
        // Now create a display list with the object data
        listID = glGenLists(1);
        glNewList(listID, GL_COMPILE);

        DrawDirect();

        glEndList();
    }

    setlocale(LC_NUMERIC, oldlocale);
    free(buffer);

    IFTRACE(objloader)
        std::cerr << name << ": " << vertices.size() << " vertices "
                  << faces.size() << " faces\n";
}


void Object3D::LoadMtl(text name)
// ----------------------------------------------------------------------------
//    Load a material library file (.mtl) from disk
// ----------------------------------------------------------------------------
{
    std::ifstream input(name.c_str());
    uint          line       = 0;
    coord         x, y, z, d;
#  define         BUFSIZE      1024
    char *        buffer     = (char *)malloc(BUFSIZE * sizeof(char));
    char *        ptr;
    char *        end;
    int           c;
    char *        oldlocale = setlocale(LC_NUMERIC, "C");
    char          prev_eol  = '\0';
    Material      material;

    while (input.good())
    {
        // Scan a new line of text
        ptr = buffer;
        end = buffer + BUFSIZE - 1;
#       undef BUFSIZE
        line++;

        bool too_long = false;
        do
        {
            c = input.get();
            if (c)
            {
                if (ptr < end)
                    *ptr++ = c;
                else
                    too_long = true;
            }
        } while (input.good() && c != '\n' && c != '\r');
        *ptr++ = 0;
        if (too_long)
            std::cerr << name << ":" << line << ":"
                      << "Line too long\n";

        // Do not count CR+LF as 2 lines
        ptr = buffer;
        if ((*ptr == '\n' && prev_eol == '\r'))
              line--;
        prev_eol = '\0';
        if (c == '\n' || c == '\r')
            prev_eol = c;

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

        case 'n':
            // New material
            if (*ptr++ == 'e' && *ptr++ == 'w' &&
                *ptr++ == 'm' && *ptr++ == 't' && *ptr++ == 'l')
            {
                if (!material.name.empty())
                    materials.push_back(material);
                while(isspace(*ptr))
                    ptr++;
                if (!*ptr)
                    break;
                text name(ptr);
                size_t eol = name.find('\r');
                if (eol == name.npos)
                    eol = name.find('\n');
                if (eol != name.npos)
                    name.resize(eol);
                material = Material(name.c_str());
            }
            break;

        case 'K':
            // Ambiante/diffuse/specular reflectance
            x = y = z = 0;
            switch(*ptr)
            {
            case 'a':
                // Ambient coordinate
                if (sscanf(++ptr, "%lf %lf %lf", &x, &y, &z) < 2)
                    std::cerr << name << ":" << line << ":"
                              << "Invalid ambient coordinate: " << ptr << '\n';
                material.ambient = Reflectance(x, y, z);
                break;
            case 'd':
                // Diffuse coordinate
                if (sscanf(++ptr, "%lf %lf %lf", &x, &y, &z) < 2)
                    std::cerr << name << ":" << line << ":"
                              << "Invalid ambient coordinate: " << ptr << '\n';
                material.diffuse = Reflectance(x, y, z);
                break;
            case 's':
                // Specular coordinate
                if (sscanf(++ptr, "%lf %lf %lf", &x, &y, &z) < 2)
                    std::cerr << name << ":" << line << ":"
                              << "Invalid ambient coordinate: " << ptr << '\n';
                material.specular = Reflectance(x, y, z);
                break;
            } // Switch secondary 'K' key
            break;

        case 'd':
            // Dissolve factor
            d = 1;
            if (sscanf(++ptr, "%lf", &d) < 1)
                std::cerr << name << ":" << line << ":"
                          << "Invalid dissolve factor: " << ptr << '\n';
            material.alpha = d;
            break;

        case 'N':
            // Specular exponent
            if (*ptr++ == 's')
            {
                coord Ns = 0;
                if (sscanf(++ptr, "%lf", &Ns) < 1)
                    std::cerr << name << ":" << line << ":"
                            << "Invalid specular exponent: " << ptr << '\n';
                material.shininess = Ns;
            }
            break;

        } // Switch key
    } // While loop

    if (!material.name.empty())
        materials.push_back(material);

    setlocale(LC_NUMERIC, oldlocale);
    free(buffer);
}


uint Object3D::MaterialId(text name)
// ----------------------------------------------------------------------------
//   Find the index of a material
// ----------------------------------------------------------------------------
{
    for (size_t i = 0; i < materials.size(); i++)
        if (materials[i].name == name)
            return i;
    return (uint)-1;
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
    if (useCallList)
        glCallList(listID);
    else
        DrawDirect();
    glPopMatrix();
}


void Object3D::DrawDirect()
// ----------------------------------------------------------------------------
//   Draw object, direct GL calls
// ----------------------------------------------------------------------------
{
    Faces::iterator f;
    Faces::iterator fe = faces.end();
    uint nv = vertices.size();
    uint nn = normals.size();
    uint nt = texCoords.size();
    uint m_id, prev_id = (uint)-1;

    listID = glGenLists(1);
    glNewList(listID, GL_COMPILE);

    for (f = faces.begin(); f != fe; f++)
    {
        Face &face = *f;

        std::vector<uint>::iterator v = face.vertex.begin();
        std::vector<uint>::iterator t = face.texCoord.begin();
        std::vector<uint>::iterator n = face.normal.begin();

        std::vector<uint>::iterator ve = face.vertex.end();
        std::vector<uint>::iterator te = face.texCoord.end();
        std::vector<uint>::iterator ne = face.normal.end();

        v = face.vertex.begin();
        t = face.texCoord.begin();
        n = face.normal.begin();
        m_id = face.materialId;

        GLuint mode = GL_POLYGON;
        switch (face.vertex.size())
        {
        case 1: mode = GL_POINTS; break;
        case 2: mode = GL_LINES; break;
        case 3: mode = GL_TRIANGLES; break;
        case 4: mode = GL_QUADS; break;
        }

        glBegin(mode);

        if (m_id != (uint)-1 && m_id != prev_id)
        {
            prev_id = m_id;
            Material & m = materials[m_id];

            GLfloat amb[4] = { m.ambient.x, m.ambient.y, m.ambient.z, m.alpha };
            glMaterialfv(GL_FRONT, GL_AMBIENT, &amb[0]);

            GLfloat diff[4] = { m.diffuse.x,m.diffuse.y,m.diffuse.z,m.alpha };
            glMaterialfv(GL_FRONT, GL_DIFFUSE, &diff[0]);

            GLfloat spec[4] = { m.specular.x,m.specular.y,m.specular.z,m.alpha};
            glMaterialfv(GL_FRONT, GL_SPECULAR, &spec[0]);

            if (m.shininess != 0.0)
                glMaterialf(GL_FRONT, GL_SHININESS, m.shininess/100.0);
        }

        while(v != ve)
        {
            if (t != te)
                if (uint it = *t++)
                    if (it <= nt)
                        glTexCoord3dv(&texCoords[it-1].x);
            if (n != ne)
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
        QString qname(+name);
        QFileInfo file(qname);
        if (!file.isReadable())
        {
            QFile qualified ("object:" + qname);
            file = QFileInfo(qualified);
        }
        text path = +file.canonicalFilePath();
        file.setFile(+path);
        if (file.isReadable())
        {
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
