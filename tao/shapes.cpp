// ****************************************************************************
//  shapes.cpp                                                     Tao project
// ****************************************************************************
//
//   File Description:
//
//     Drawing of elementary geometry shapes on a 2D layout
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

#include "shapes.h"
#include "layout.h"
#include "attributes.h"
#include "path3d.h"
#include "gl_keepers.h"
#include "application.h"
#include "widget_surface.h"
#include <QPainterPath>
#include "lighting.h"
TAO_BEGIN

// ============================================================================
//
//    Common utilities for drawing shapes
//
// ============================================================================

bool Shape::setTexture(Layout *where)
// ----------------------------------------------------------------------------
//   Get the texture from the layout
// ----------------------------------------------------------------------------
{
    // Do not bother with textures if in Identify phase
    if (where->InIdentify())
        return !where->fillTextures.empty();

    for(uint i = 0; i < GL.maxTextureUnits; i++)
    {
        //Determine if there is a current and previous texture
        bool hasCurrent = where->fillTextures.count(i);
        bool hasPrevious = where->previousTextures.count(i);

        // If there is a previous texture and no current
        // then unbind this one.
        if(hasPrevious && (! hasCurrent))
        {
            // Unbind the previous texture
            unbindTexture(where->previousTextures[i]);
        }
        else if(hasCurrent && (where->textureUnits & (1 << i)))
        {
            // If there is a previous texture with a different type
            // of the current then unbind the previous before to bind the
            // current.
            if(hasPrevious)
            {
                if(where->fillTextures[i].type !=
                   where->previousTextures[i].type)
                {
                    // Unbind the previous texture
                    unbindTexture(where->previousTextures[i]);
                }
            }

            // Bind the current texture
            bindTexture(where->fillTextures[i], where->hasPixelBlur);
        }
    }

    // Active current shader
    setShader(where);

    //Update used texture units
    where->previousTextures = where->fillTextures;

    return !(where->fillTextures.empty());
}


void Shape::bindTexture(TextureState& texture, bool hasPixelBlur)
// ----------------------------------------------------------------------------
//    Bind the given texture
// ----------------------------------------------------------------------------
{
    glActiveTexture(GL_TEXTURE0 + texture.unit);
    GL.enable(texture.type);
    glBindTexture(texture.type, texture.id);
    GLint min, mag;
    if (texture.type == GL_TEXTURE_2D)
    {
        min = texture.minFilt;
        mag = texture.magFilt;
        if (!texture.mipmap)
        {
            if (min == GL_NEAREST_MIPMAP_NEAREST ||
                min == GL_LINEAR_MIPMAP_NEAREST  ||
                min == GL_NEAREST_MIPMAP_LINEAR  ||
                min == GL_LINEAR_MIPMAP_LINEAR)
                min = GL_LINEAR;
            if (mag == GL_NEAREST_MIPMAP_NEAREST ||
                mag == GL_LINEAR_MIPMAP_NEAREST  ||
                mag == GL_NEAREST_MIPMAP_LINEAR  ||
                mag == GL_LINEAR_MIPMAP_LINEAR)
                mag = GL_LINEAR;
        }
    }
    else
    {
        if (hasPixelBlur)
            min = mag = GL_LINEAR;
        else
            min = mag = GL_NEAREST;
    }
    glTexParameteri(texture.type, GL_TEXTURE_MAG_FILTER, mag);
    glTexParameteri(texture.type, GL_TEXTURE_MIN_FILTER, min);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, texture.mode);

    // Wrap if texture 2D
    if(texture.type == GL_TEXTURE_2D)
    {
        GLuint wrapS = texture.wrapS ? GL_REPEAT : GL_CLAMP_TO_EDGE;
        GLuint wrapT = texture.wrapT ? GL_REPEAT : GL_CLAMP_TO_EDGE;
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapS);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapT);
    }

    if (TaoApp->hasGLMultisample)
        GL.enable(GL_MULTISAMPLE);
}


void Shape::unbindTexture(TextureState& texture)
// ----------------------------------------------------------------------------
//    Unbind the given texture
// ----------------------------------------------------------------------------
{
    glActiveTexture(GL_TEXTURE0 + texture.unit);
    glBindTexture(texture.type, 0);
    GL.disable(texture.type);
}


void Shape::enableTexCoord(uint unit, void *texCoord)
// ----------------------------------------------------------------------------
//    Enable texture coordinates of the specified unit
// ----------------------------------------------------------------------------
{
    glClientActiveTexture( GL_TEXTURE0 + unit);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_DOUBLE, 0, texCoord);
}


void Shape::disableTexCoord(uint unit)
// ----------------------------------------------------------------------------
//    Disable texture coordinates of the specified unit
// ----------------------------------------------------------------------------
{
    glClientActiveTexture( GL_TEXTURE0 + unit);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}


bool Shape::setFillColor(Layout *where)
// ----------------------------------------------------------------------------
//    Set the fill color and texture according to the layout attributes
// ----------------------------------------------------------------------------
{
    // Check if we have a non-transparent fill color
    if (where)
    {
        Color &color = where->fillColor;
        scale v = where->visibility * color.alpha;
        if (v >= 0.01)
        {
            if (!where->hasMaterial)
                GL.setColor(color.red, color.green, color.blue, v);
            where->PolygonOffset();
            return true;
        }
    }
    return false;
}


bool Shape::setLineColor(Layout *where)
// ----------------------------------------------------------------------------
//    Set the outline color according to the layout attributes
// ----------------------------------------------------------------------------
{
    // Check if we have a non-transparent outline color
    if (where)
    {
        Color &color = where->lineColor;
        scale width = where->lineWidth;
        scale v = where->visibility * color.alpha;
        if (v >= 0.01 && width > 0.0)
        {
            if (!where->hasMaterial)
                GL.setColor(color.red, color.green, color.blue, v);
            where->PolygonOffset();
            return true;
        }
    }
    return false;
}


bool Shape::setShader(Layout *where)
// ----------------------------------------------------------------------------
//   Set the shader according to info in current layout
// ----------------------------------------------------------------------------
{
    if(where->InIdentify())
        return false;

    // Activate current shader
    if (where->globalProgramId)
        glUseProgram(where->globalProgramId);
    else
        glUseProgram(where->programId);

    // In order to improve performance of large and complex 3D models,
    // we use a shader based ligting (Feature #1508), which needs some
    // uniform values to have an efficient behaviour.
    if(where->perPixelLighting == where->programId)
    {
        if(where->programId)
        {
            GLint lights = glGetUniformLocation(where->programId, "lights");
            glUniform1i(lights, where->currentLights);

            GLint textures = glGetUniformLocation(where->programId, "textures");
            glUniform1i(textures, where->textureUnits);

            GLint vendor = glGetUniformLocation(where->programId, "vendor");
            glUniform1i(vendor, GL.vendorID);

            // Set texture units
            GLint tex0 = glGetUniformLocation(where->programId, "tex0");
            glUniform1i(tex0, 0);
            GLint tex1 = glGetUniformLocation(where->programId, "tex1");
            glUniform1i(tex1, 1);
            GLint tex2 = glGetUniformLocation(where->programId, "tex2");
            glUniform1i(tex2, 2);
            GLint tex3 = glGetUniformLocation(where->programId, "tex3");
            glUniform1i(tex3, 3);
        }
    }

    return true;
}

// ============================================================================
//
//   Shape 2D
//
// ============================================================================


void Shape2::Draw(GraphicPath &path)
// ----------------------------------------------------------------------------
//    Draw the shape in a path
// ----------------------------------------------------------------------------
{
    (void) path;
}


void Shape2::Draw(Layout *where)
// ----------------------------------------------------------------------------
//    Draw the shape using a path
// ----------------------------------------------------------------------------
{
    GraphicPath path;
    Draw(path);
    path.Draw(where);
}


void Shape2::Identify(Layout *where)
// ----------------------------------------------------------------------------
//   Draw a simplified version of the shape for selection purpose
// ----------------------------------------------------------------------------
{
    GraphicPath path;
    Draw(path);
    path.Draw(where, GL_SELECT);
}

// ============================================================================
//
//   Shape drawing routines
//
// ============================================================================

Box3 Rectangle::Bounds(Layout *where)
// ----------------------------------------------------------------------------
//   Return the bounding box for all rectangular objects
// ----------------------------------------------------------------------------
{
    return bounds + where->offset;
}


void Rectangle::Draw(GraphicPath &path)
// ----------------------------------------------------------------------------
//   Draw a rectangle
// ----------------------------------------------------------------------------
{
    path.moveTo(Point3(bounds.lower.x, bounds.lower.y, 0));
    path.lineTo(Point3(bounds.upper.x, bounds.lower.y, 0));
    path.lineTo(Point3(bounds.upper.x, bounds.upper.y, 0));
    path.lineTo(Point3(bounds.lower.x, bounds.upper.y, 0));
    path.close();
}


void PlaceholderRectangle::Draw(Layout *where)
// ----------------------------------------------------------------------------
//    Draw the shape using a path
// ----------------------------------------------------------------------------
{
    GLAttribKeeper save(GL_LINE_BIT | GL_CURRENT_BIT | GL_POLYGON_STIPPLE_BIT);
    GraphicPath path;
    Draw(path);

    GL.setColor(0.3, 0.7, 0.9, 0.7);
    GL.setLineWidth(1);
    GL.disable(GL_LINE_STIPPLE);

    where->PolygonOffset();
    path.Draw(where->Offset(), where->textureUnits, GL_LINE_STRIP, 0);
}


void PlaceholderRectangle::Draw(GraphicPath &path)
// ----------------------------------------------------------------------------
//   We draw a rectangle crossed out
// ----------------------------------------------------------------------------
{
    path.moveTo(Point3(bounds.lower.x, bounds.lower.y, 0));
    path.lineTo(Point3(bounds.upper.x, bounds.lower.y, 0));
    path.lineTo(Point3(bounds.upper.x, bounds.upper.y, 0));
    path.lineTo(Point3(bounds.lower.x, bounds.upper.y, 0));
    path.lineTo(Point3(bounds.lower.x, bounds.lower.y, 0));

    path.lineTo(Point3(bounds.upper.x, bounds.upper.y, 0));
    path.moveTo(Point3(bounds.upper.x, bounds.lower.y, 0));
    path.lineTo(Point3(bounds.lower.x, bounds.upper.y, 0));
}


void ClickThroughRectangle::DrawSelection (Layout *layout)
// ----------------------------------------------------------------------------
//   Pass clicks through to the widget
// ----------------------------------------------------------------------------
{
    Widget *widget = layout->Display();
    Point center = bounds.Center();
    if (surface->requestFocus(layout, center.x, center.y))
    {
        Box3 bounds = Bounds(layout);
        XL::Save<Point3> zeroOffset(layout->offset, Point3(0,0,0));
        widget->drawSelection(layout, bounds,
                              "widget_selection", layout->id);
    }
}


void IsoscelesTriangle::Draw(GraphicPath &path)
// ----------------------------------------------------------------------------
//   Draw an isosceles triangle
// ----------------------------------------------------------------------------
{
    path.moveTo(Point3(bounds.lower.x, bounds.lower.y, 0));
    path.lineTo(Point3(bounds.upper.x, bounds.lower.y, 0));
    path.lineTo(Point3((bounds.upper.x+bounds.lower.x)/2, bounds.upper.y, 0));
    path.close();
}


void RightTriangle::Draw(GraphicPath &path)
// ----------------------------------------------------------------------------
//   Draw a right triangle
// ----------------------------------------------------------------------------
{
    path.moveTo(Point3(bounds.lower.x, bounds.lower.y, 0));
    path.lineTo(Point3(bounds.upper.x, bounds.lower.y, 0));
    path.lineTo(Point3(bounds.lower.x, bounds.upper.y, 0));
    path.close();
}


void RoundedRectangle::Draw(GraphicPath &path)
// ----------------------------------------------------------------------------
//   Draw a rounded rectangle
// ----------------------------------------------------------------------------
{
    QPainterPath rect;
    coord w = bounds.Width();
    coord h = bounds.Height();
    int sw = w > 0? 1: -1;
    int sh = h > 0? 1: -1;
    double pw = sw*w;
    double ph = sh*h;

    if (pw < ph)
    {
        if (r > pw / 2)
            r = pw / 2;
    }
    else
    {
        if (r > ph / 2)
            r = ph / 2;
    }
    rect.addRoundedRect(bounds.lower.x, bounds.lower.y,
                        w, h, r, r);
    path.addQtPath(rect);
}


void EllipticalRectangle::Draw(Layout *where)
// ----------------------------------------------------------------------------
//    We need correct tesselation
// ----------------------------------------------------------------------------
{
    GraphicPath path;
    Draw(path);
    path.Draw(where, GLU_TESS_WINDING_POSITIVE);
}


void EllipticalRectangle::Draw(GraphicPath &path)
// ----------------------------------------------------------------------------
//   Draw an elliptical rectangle
// ----------------------------------------------------------------------------
{
    Point c = bounds.Center();
    coord w = bounds.Width();
    coord h = bounds.Height();
    int sw = w > 0? 1: -1;
    int sh = h > 0? 1: -1;
    double pw = sw*w;
    double ph = sh*h;
    if (r < 0.0)
    {
        QPainterPath ellipse;
        ellipse.addEllipse(bounds.lower.x, bounds.lower.y,
                           bounds.Width(), bounds.Height());
        path.addQtPath(ellipse);
    }
    else if (r > 1.0)
    {
        path.moveTo(Point3(bounds.lower.x, bounds.lower.y, 0));
        path.lineTo(Point3(bounds.upper.x, bounds.lower.y, 0));
        path.lineTo(Point3(bounds.upper.x, bounds.upper.y, 0));
        path.lineTo(Point3(bounds.lower.x, bounds.upper.y, 0));
        path.close();
    }
    else
    {
        double rr = M_SQRT1_2+(1-M_SQRT1_2)*r;
        double alpha_r = acos(rr);
        double alpha_d = alpha_r * 180 / M_PI;
        double ratio = (1-cos(alpha_r))/(1-sin(alpha_r));

        QPainterPath rect;
        rect.moveTo(c.x+rr*pw/2, c.y+rr*ph/2);
        rect.arcTo(c.x+pw/2-ratio*pw, c.y-ph/2,
                   ratio*pw, ph,
                   270+alpha_d, 180-2*alpha_d);
        rect.arcTo(c.x-pw/2, c.y-ph/2,
                   pw, ratio*ph,
                   alpha_d, 180-2*alpha_d);
        rect.arcTo(c.x-pw/2, c.y-ph/2,
                   ratio*pw, ph,
                   90+alpha_d, 180-2*alpha_d);
        rect.arcTo(c.x-pw/2, c.y+ph/2-ratio*ph,
                   pw, ratio*ph,
                   180+alpha_d, 180-2*alpha_d);
        path.addQtPath(rect);
    }
}


void Ellipse::Draw(GraphicPath &path)
// ----------------------------------------------------------------------------
//   Draw an ellipse inside the bounds
// ----------------------------------------------------------------------------
{
    QPainterPath qt;
    qt.addEllipse(bounds.lower.x, bounds.lower.y,
                  bounds.Width(), bounds.Height());
    path.addQtPath(qt);
}


void EllipseArc::Draw(GraphicPath &path)
// ----------------------------------------------------------------------------
//    Draw an arc of ellipse
// ----------------------------------------------------------------------------
{
    QPainterPath qt;
    Point center = bounds.Center();
    qt.moveTo(center.x, center.y);
    qt.arcTo(bounds.lower.x, bounds.lower.y,
             bounds.Width(), bounds.Height(),
             start, sweep);
    qt.closeSubpath();
    path.addQtPath(qt);
}


void Arrow::Draw(Layout *where)
// ----------------------------------------------------------------------------
//    We need correct tesselation
// ----------------------------------------------------------------------------
{
    GraphicPath path;
    Draw(path);
    path.Draw(where, GLU_TESS_WINDING_POSITIVE);
}


void Arrow::Draw(GraphicPath &path)
// ----------------------------------------------------------------------------
//   Draw an arrow
// ----------------------------------------------------------------------------
{
    coord aax, aay;
    int sw = bounds.Width() > 0? 1: -1;

    if (ax > sw*bounds.Width())
        aax = bounds.Width();
    else
        aax = sw*ax;

    if (ax < 0.0)
        aax = 0.0;

    if (ary > 1.0)
        aay = bounds.Height();
    else
        aay = ary*bounds.Height();

    if (ary < 0.0)
        aay = 0.0;

    coord x0 = bounds.Left();
    coord x1 = bounds.Right();
    coord xa1 = x1 - aax;

    coord y0 = bounds.Bottom();
    coord yc = (bounds.Bottom() + bounds.Top())/2;
    coord y1 = bounds.Top();
    coord ya0 = yc - aay/2;
    coord ya1 = yc + aay/2;

    path.moveTo(Point3(x0,  ya0, 0));
    path.lineTo(Point3(xa1, ya0, 0));
    path.lineTo(Point3(xa1, y0,  0));
    path.lineTo(Point3(x1,  yc,  0));
    path.lineTo(Point3(xa1, y1,  0));
    path.lineTo(Point3(xa1, ya1, 0));
    path.lineTo(Point3(x0,  ya1, 0));
    path.close();
}


void DoubleArrow::Draw(Layout *where)
// ----------------------------------------------------------------------------
//    We need correct tesselation
// ----------------------------------------------------------------------------
{
    GraphicPath path;
    Draw(path);
    path.Draw(where, GLU_TESS_WINDING_POSITIVE);
}


void DoubleArrow::Draw(GraphicPath &path)
// ----------------------------------------------------------------------------
//   Draw a double arrow
// ----------------------------------------------------------------------------
{
    coord aax, aay;
    int sw = bounds.Width() > 0? 1: -1;

    if (ax > sw*bounds.Width()/2)
        aax = bounds.Width()/2;
    else
        aax = sw*ax;

    if (ax < 0.0)
        aax = 0.0;

    if (ary > 1.0)
        aay = bounds.Height();
    else
        aay = ary*bounds.Height();

    if (ary < 0.0)
        aay = 0.0;

    coord x0 = bounds.lower.x;
    coord x1 = bounds.upper.x;
    coord xa0 = x0 + aax;
    coord xa1 = x1 - aax;

    coord y0 = bounds.lower.y;
    coord yc = (bounds.lower.y + bounds.upper.y)/2;
    coord y1 = bounds.upper.y;
    coord ya0 = yc - aay/2;
    coord ya1 = yc + aay/2;

    path.moveTo(Point3(x0,  yc,  0));
    path.lineTo(Point3(xa0, y0,  0));
    path.lineTo(Point3(xa0, ya0, 0));
    path.lineTo(Point3(xa1, ya0, 0));
    path.lineTo(Point3(xa1, y0,  0));
    path.lineTo(Point3(x1,  yc,  0));
    path.lineTo(Point3(xa1, y1,  0));
    path.lineTo(Point3(xa1, ya1, 0));
    path.lineTo(Point3(xa0, ya1, 0));
    path.lineTo(Point3(xa0, y1,  0));
    path.close();
}


void StarPolygon::Draw(Layout *where)
// ----------------------------------------------------------------------------
//    We need correct tesselation when q > 1
// ----------------------------------------------------------------------------
{
    GraphicPath path;
    Draw(path);
    if (q <= 1)
    {
        // Regular polygon, no need to tesselate
        path.Draw(where);
    }
    else
    {
        path.Draw(where, GLU_TESS_WINDING_POSITIVE);
    }
}


void StarPolygon::Draw(GraphicPath &path)
// ----------------------------------------------------------------------------
//   Draw a regular polygon or a star
// ----------------------------------------------------------------------------
{
    if (p < 3)
        p = 3;
    if (q < 1)
        q = 1;
    if (q > (p-1)/2)
        q = (p-1)/2;

    scale  w1     = bounds.Width()/2;
    scale  h1     = bounds.Height()/2;
    Point3 center = bounds.Center();
    coord  cx     = center.x;
    coord  cy     = center.y;

    if (q > 0)
    {
        scale  R_r    = cos(q * M_PI/p) / cos((q-1) * M_PI/p);
        scale  w2     = w1 * R_r;
        scale  h2     = h1 * R_r;
        double a      = 0;
        double da     = M_PI/p;

        for (int i = 0; i < p; i++)
        {
            double x1 = cx + w1 * sin(a);
            double y1 = cy + h1 * cos(a);
            a += da;
            double x2 = cx + w2 * sin(a);
            double y2 = cy + h2 * cos(a);
            a += da;

            if (i)
                path.lineTo(Point3(x1, y1, 0));
            else
                path.moveTo(Point3(x1, y1, 0));

            path.lineTo(Point3(x2, y2, 0));


        }
        path.close();
    }
    else
    {
        double a      = 0;
        double da     = 2 * M_PI * q / p;

        for (int i = 0; i <= p; i++)
        {
            if (2*i == p)
            {
                path.close();
                a += da/2;
            }

            double x1 = cx + w1 * sin(a);
            double y1 = cy + h1 * cos(a);
            a += da;

            if (i == 0 || 2*i == p)
            {
                path.moveTo(Point3(x1, y1, 0));
            }
            else
            {
                path.lineTo(Point3(x1, y1, 0));
            }
        }
        path.close();
    }
}


void Star::Draw(Layout *where)
// ----------------------------------------------------------------------------
//    We need correct tesselation when radius ratio != 1
// ----------------------------------------------------------------------------
{
    GraphicPath path;
    Draw(path);
    if (r >= 1.0)
    {
        // Regular polygon, no need to tesselate
        path.Draw(where);
    }
    else
    {
        path.Draw(where, GLU_TESS_WINDING_POSITIVE);
    }
}


void Star::Draw(GraphicPath &path)
// ----------------------------------------------------------------------------
//   Draw a regular polygon or a star
// ----------------------------------------------------------------------------
{
    if (r < 0.0)
        r = 0.0;
    if (r > 1.0)
        r = 1.0;
    if (p < 3)
        p = 3;

    scale  w1     = bounds.Width()/2;
    scale  h1     = bounds.Height()/2;
    Point  c      = bounds.Center();

    scale  w2     = w1 * r;
    scale  h2     = h1 * r;
    double a      = 0;
    double da     = M_PI/p;

    for (int i = 0; i < p; i++)
    {
        double x1 = c.x + w1 * sin(a);
        double y1 = c.y + h1 * cos(a);
        a += da;
        double x2 = c.x + w2 * sin(a);
        double y2 = c.y + h2 * cos(a);
        a += da;

        if (i)
            path.lineTo(Point3(x1, y1, 0));
        else
            path.moveTo(Point3(x1, y1, 0));

        path.lineTo(Point3(x2, y2, 0));

    }
    path.close();
}


void SpeechBalloon::Draw(Layout *where)
// ----------------------------------------------------------------------------
//    We need correct tesselation
// ----------------------------------------------------------------------------
{
    GraphicPath path;
    Draw(path);
    path.Draw(where, GLU_TESS_WINDING_POSITIVE);
}


void SpeechBalloon::Draw(GraphicPath &path)
// ----------------------------------------------------------------------------
//   Draw a speech ballon
// ----------------------------------------------------------------------------
{

    Point c = bounds.Center();
    coord w =  bounds.Width();
    coord h = bounds.Height();
    int sw = w > 0? 1: -1;
    int sh = h > 0? 1: -1;
    double rx = r;
    double ry = r;
    if (r > sw * w/2)
        rx = sw * w/2;
    if (r > sh * h/2)
        ry = sh * h/2;

    QPainterPath balloon;
    balloon.addRoundedRect(bounds.lower.x, bounds.lower.y, w, h, rx, ry);

    double tx = a.x - c.x;
    double ty = a.y - c.y;
    int stx = tx > 0? 1: -1;
    int sty = ty > 0? 1: -1;
    double th = sqrt(tx*tx + ty*ty);
    double tw = th/7;
    tw = tw < w/7? tw: w/7;
    tw = tw < h/5? tw: h/5;

    double theta, s, o;
    if (stx*tx > sty*ty)
    {
        theta = asin(sty*ty/th);
        s = 1.5;
        o = 0;
    }
    else
    {
        theta = acos(stx*tx/th);
        s = 0.5;
        o = M_PI_4;
    }

    QPainterPath tail;
    tail.moveTo( c.x+stx*tw*cos(theta+M_PI_2), c.y+sty*tw*sin(theta+M_PI_2));
    tail.quadTo( c.x+stx*0.5*(th*cos(s*theta+o)+tw*cos(theta+M_PI_2))
               , c.y+sty*0.5*(th*sin(s*theta+o)+tw*sin(theta+M_PI_2))
               , a.x, a.y);
    tail.quadTo( c.x+stx*0.5*(th*cos(s*theta+o)+tw*cos(theta-M_PI_2))
               , c.y+sty*0.5*(th*sin(s*theta+o)+tw*sin(theta-M_PI_2))
               , c.x+stx*tw*cos(theta-M_PI_2), c.y+sty*tw*sin(theta-M_PI_2));

    path.addQtPath(balloon+=tail);
}


void Callout::Draw(Layout *where)
// ----------------------------------------------------------------------------
//    We need correct tesselation
// ----------------------------------------------------------------------------
{
    GraphicPath path;
    Draw(path);
    path.Draw(where, GLU_TESS_WINDING_POSITIVE);
}


void Callout::Draw(GraphicPath &path)
// ----------------------------------------------------------------------------
//   Draw a callout
// ----------------------------------------------------------------------------
{
    Point c = bounds.Center();
    coord w = bounds.Width();
    coord h = bounds.Height();
    int sw = w > 0? 1: -1;
    int sh = h > 0? 1: -1;
    double pw = sw*w;
    double ph = sh*h;

    if (pw < ph)
    {
        if (r > pw/2)
            r = pw/2;
    }
    else
    {
        if (r > ph/2)
            r = ph/2;
    }

    QPainterPath rect;
    rect.addRoundedRect(bounds.lower.x, bounds.lower.y,
                        w, h, r, r);

    double tx = a.x - c.x;
    double ty = a.y - c.y;
    int stx = tx > 0? 1: -1;
    int sty = ty > 0? 1: -1;
    double ptx = stx*tx;
    double pty = sty*ty;
    Point d0, cr;
    double alpha;
    Vector tr;
    bool inside = false;

    if (pty <= ph/2-r)
    {
        if (ptx <= pw/2)
        {
            inside = true;
        }
        else
        {
            d0.x = c.x + stx*pw/2;
            d0.y = c.y + ty;
        }
    }
    else if(ptx <= pw/2-r)
    {
        if (pty <= ph/2)
        {
            inside = true;
        }
        else
        {
            d0.x = c.x + tx;
            d0.y = c.y + sty*ph/2;
        }
    }
    else
    {
        cr.x = c.x + stx*(pw/2-r);
        cr.y = c.y + sty*(ph/2-r);
        Vector tr = a - cr;
        if (tr.Length() <= r)
        {
            inside = true;
        }
        else
        {
            tr.Normalize();
            alpha = sty*acos(tr.x);
            d0.x = cr.x + r*cos(alpha);
            d0.y = cr.y + r*sin(alpha);
        }
    }

    Point d1, d2, cd;
    double beta, theta;
    Vector td;
    if (d < 0)
        d = 0;
    d = d < pw? d: pw;
    d = d < ph? d: ph;
    double mrd = d > 2*r? d: 2*r;

    if (!inside)
    {
        if (pty <= (ph-mrd)/2)
        {
            // Horizontal tail
            cd.x = c.x + stx*(pw-mrd)/2;
            cd.y = c.y + sty*(pty < (ph-mrd)/2? pty: (ph-mrd)/2);

            td = a - cd;
            theta = asin(d/2/td.Length());

            d1.x = cd.x + stx*d/2*sin(theta);
            d1.y = cd.y + d/2*cos(theta);

            d2.x = cd.x + stx*d/2*sin(theta);
            d2.y = cd.y - d/2*cos(theta);
        }
        else if (ptx <= (pw-mrd)/2)
        {
            // Vertical tail
            cd.x = c.x + stx*(ptx < (pw-mrd)/2? ptx: (pw-mrd)/2);
            cd.y = c.y + sty*(ph-mrd)/2;

            td = a - cd;
            theta = asin(d/2/td.Length());

            d1.x = cd.x + d/2*cos(theta);
            d1.y = cd.y + sty*d/2*sin(theta);

            d2.x = cd.x - d/2*cos(theta);
            d2.y = cd.y + sty*d/2*sin(theta);
        }
        else
        {
            // Tail with an angle
            cd.x = c.x + stx*(pw-mrd)/2;
            cd.y = c.y + sty*(ph-mrd)/2;

            td = a - cd;
            theta = asin(d/2/td.Length());

            td.Normalize();
            beta = sty*acos(td.x);

            d1.x = cd.x + d/2*cos(beta+(M_PI_2-theta));
            d1.y = cd.y + d/2*sin(beta+(M_PI_2-theta));

            d2.x = cd.x + d/2*cos(beta-(M_PI_2-theta));
            d2.y = cd.y + d/2*sin(beta-(M_PI_2-theta));
        }


        if (d == 0)
        {
            rect.moveTo(d0.x, d0.y);
            rect.lineTo(a.x, a.y);
        }
        else
        {
            QPainterPath tail;
            tail.moveTo(cd.x, cd.y);
            tail.lineTo(d1.x, d1.y);
            tail.lineTo(a.x, a.y);
            tail.lineTo(d2.x, d2.y);
            tail.closeSubpath();
            rect += tail;
        }
    }
    path.addQtPath(rect);
}


void FixedSizePoint::Draw(Layout *where)
// ----------------------------------------------------------------------------
//   Draw a point at the given location
// ----------------------------------------------------------------------------
{
    setTexture(where);
    if (setFillColor(where))
    {
        glPointSize(radius * where->PrinterScaling());
        glBegin(GL_POINTS);
        glVertex3f(center.x, center.y, center.z);
        glEnd();

#ifdef CONFIG_LINUX
        // This is a workaround for a bug seen on some Linux distros
        // (e.g. Ubuntu 10.04 running on a system with Intel Mobile 4 graphics)
        // where GL_POINTS are not detected in GL_SELECT mode.
        // Drawing a null-sized quad makes the point selectable.
        glBegin(GL_QUADS);
        glVertex3f(center.x, center.y, center.z);
        glVertex3f(center.x, center.y, center.z);
        glVertex3f(center.x, center.y, center.z);
        glVertex3f(center.x, center.y, center.z);
        glEnd();
#endif
    }
}

TAO_END



// ****************************************************************************
// 
//    Code generation from shapes.tbl
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
#include "shapes.tbl"

namespace Tao
{

#include "shapes.tbl"


void EnterShapes()
// ----------------------------------------------------------------------------
//   Enter all the basic operations defined in attributes.tbl
// ----------------------------------------------------------------------------
{
    XL::Context *context = MAIN->context;
#include "opcodes_define.h"
#include "shapes.tbl"
}


void DeleteShapes()
// ----------------------------------------------------------------------------
//   Delete all the global operations defined in attributes.tbl
// ----------------------------------------------------------------------------
{
#include "opcodes_delete.h"
#include "shapes.tbl"
}

}
