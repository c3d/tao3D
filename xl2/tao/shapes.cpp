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
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Lionel Schaffhauser <lionel@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "shapes.h"
#include "layout.h"
#include "attributes.h"
#include "path3d.h"
#include "gl_keepers.h"
#include <GL/glew.h>
#include <QtOpenGL>
#include <QPainterPath>

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
    if (where->fillTexture)
    {
        glBindTexture(GL_TEXTURE_2D, where->fillTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_MULTISAMPLE);
    }
    else
    {
        glDisable(GL_TEXTURE_2D);
    }

    return where->fillTexture ? true : false;
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
        if (color.alpha > 0.0)
        {
            glColor4f(color.red, color.green, color.blue, color.alpha);
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
        if (color.alpha > 0.0)
        {
            glColor4f(color.red, color.green, color.blue, color.alpha);
            where->PolygonOffset();
            return true;
        }
    }
    return false;
}


void Shape::Draw(GraphicPath &path)
// ----------------------------------------------------------------------------
//    Draw the shape in a path
// ----------------------------------------------------------------------------
{
    (void) path;
}


void Shape::Draw(Layout *where)
// ----------------------------------------------------------------------------
//    Draw the shape using a path
// ----------------------------------------------------------------------------
{
    GraphicPath path;
    Draw(path);
    path.Draw(where);
}



// ============================================================================
//
//   Shape drawing routines
//
// ============================================================================

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

    glColor4f(0.3, 0.7, 0.9, 0.7);
    glLineWidth(1);
    glDisable(GL_LINE_STIPPLE);
    
    where->PolygonOffset();
    path.Draw(where, GL_LINE_STRIP);
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


void IsoscelesTriangle::Draw(GraphicPath &path)
// ----------------------------------------------------------------------------
//   Draw an isosceles triangle
// ----------------------------------------------------------------------------
{
    path.moveTo(Point3(bounds.lower.x, bounds.lower.y, 0));
    path.lineTo(Point3(bounds.upper.x, bounds.lower.y, 0));
    path.lineTo(Point3((bounds.upper.x + bounds.lower.x)/2, bounds.upper.y, 0));
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
    setTexture(where);
    if (setFillColor(where))
        path.Draw(where, GL_POLYGON, GLU_TESS_WINDING_POSITIVE);
    if (setLineColor(where))
        // REVISIT: If lines is thick, use a QPainterPathStroker
        path.Draw(where, GL_LINE_STRIP);
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
    setTexture(where);
    if (setFillColor(where))
        path.Draw(where, GL_POLYGON, GLU_TESS_WINDING_POSITIVE);
    if (setLineColor(where))
        // REVISIT: If lines is thick, use a QPainterPathStroker
        path.Draw(where, GL_LINE_STRIP);
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
    setTexture(where);
    if (setFillColor(where))
        path.Draw(where, GL_POLYGON, GLU_TESS_WINDING_POSITIVE);
    if (setLineColor(where))
        // REVISIT: If lines is thick, use a QPainterPathStroker
        path.Draw(where, GL_LINE_STRIP);
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
        setTexture(where);
        if (setFillColor(where))
            path.Draw(where, GL_POLYGON, GLU_TESS_WINDING_POSITIVE);
        if (setLineColor(where))
            // REVISIT: If lines is thick, use a QPainterPathStroker
            path.Draw(where, GL_LINE_STRIP);
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
        setTexture(where);
        if (setFillColor(where))
            path.Draw(where, GL_POLYGON, GLU_TESS_WINDING_POSITIVE);
        if (setLineColor(where))
            // REVISIT: If lines is thick, use a QPainterPathStroker
            path.Draw(where, GL_LINE_STRIP);
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
    setTexture(where);
    if (setFillColor(where))
        path.Draw(where, GL_POLYGON, GLU_TESS_WINDING_POSITIVE);
    if (setLineColor(where))
        // REVISIT: If lines is thick, use a QPainterPathStroker
        path.Draw(where, GL_LINE_STRIP);
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
    setTexture(where);
    if (setFillColor(where))
        path.Draw(where, GL_POLYGON, GLU_TESS_WINDING_POSITIVE);
    if (setLineColor(where))
        // REVISIT: If lines is thick, use a QPainterPathStroker
        path.Draw(where, GL_LINE_STRIP);
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

    if (d < 0)
        d = 0;
    d = d < w/2? d: w/2;
    d = d < h/2? d: h/2;

    Point d0, d1, d2, cc;
    double alpha;
    bool inside = false;

    if (pty <= ph/2-r)
    {
        // Horizontal tail
        if (ptx <= pw/2)
        {
            inside = true;
        }
        else
        {
            d0.x = c.x + stx*pw/2;
            d0.y = c.y + ty;

            /*
            double l1 = ph/2-r-pty;
            double s1 = l1 + M_PI_2*r;
            bool tangent1 = false;

            if (d <= l1)
            {
                d1.x = d0.x;
                d1.y = d0.y + sty*d;
            } 
            else if (d <= s1)
            {
                // Consider tangent
                // FIXME: Really consider tangent
                tangent1 = true;
            }
            else
            {
                // Tangent
                tangent1 = true;
            }
            if (tangent1)
            {
                cc.x = c.x + stx*(pw/2-r);
                cc.y = c.y + sty*(ph/2-r);
                double xa = stx*(a.x - cc.x);
                double ya = l1;
                double aa = sqrt(xa*xa + ya*ya);
                double theta = M_PI_2 - asin(r/aa) - asin(ya/aa);
                d1.x = cc.x + stx*r*cos(theta);
                d1.y = cc.y + sty*r*sin(theta);
            }
        
            double l2 = ph/2-r+pty;
            double s2 = l2 + M_PI_2*r;
            bool tangent2 = false;

            if (d <= l2)
            {
                d2.x = d0.x;
                d2.y = d0.y - sty*d;
            } 
            else if (d <= s2)
            {
                // Consider tangent
                // FIXME: Really consider tangent
                tangent2 = true;
            }
            else
            {
                // Tangent
                tangent2 = true;
            }
            if (tangent2)
            {
                cc.x = c.x + stx*(pw/2-r);
                cc.y = c.y - sty*(ph/2-r);
                double xa = stx*(a.x - cc.x);
                double ya = l2;
                double aa = sqrt(xa*xa + ya*ya);
                double theta = M_PI_2 - asin(r/aa) - asin(ya/aa);
                d2.x = cc.x + stx*r*cos(theta);
                d2.y = cc.y - sty*r*sin(theta);
            }
            */
        }
    }
    else if (ptx <= pw/2-r)
    {
        // Vertical tail
        if (pty <= ph/2)
        {
            inside = true;
        }
        else
        {
            d0.x = c.x + tx;
            d0.y = c.y + sty*h/2;

            /*
            double l1 = pw/2-r-ptx;
            double s1 = l1 + M_PI_2*r;
            bool tangent1 = false;

            if (d <= l1)
            {
                d1.x = d0.x + stx*d;
                d1.y = d0.y;
            } 
            else if (d <= s1)
            {
                // Consider tangent
                // FIXME: Really consider tangent
                tangent1 = true;
            }
            else
            {
                // Tangent
                tangent1 = true;
            }
            if (tangent1)
            {
                cc.x = c.x + stx*(pw/2-r);
                cc.y = c.y + sty*(ph/2-r);
                double xa = l1;
                double ya = sty*(a.y - cc.y);
                double aa = sqrt(xa*xa + ya*ya);
                double theta = M_PI_2 - asin(r/aa) - asin(xa/aa);
                d1.x = cc.x + stx*r*sin(theta);
                d1.y = cc.y + sty*r*cos(theta);
            }
        
            double l2 = pw/2-r+ptx;
            double s2 = l2 + M_PI_2*r;
            bool tangent2 = false;

            if (d <= l2)
            {
                d2.x = d0.x - stx*d;
                d2.y = d0.y;
            } 
            else if (d <= s2)
            {
                // Consider tangent
                // FIXME: Really consider tangent
                tangent2 = true;
            }
            else
            {
                // Tangent
                tangent2 = true;
            }
            if (tangent2)
            {
                cc.x = c.x - stx*(pw/2-r);
                cc.y = c.y + sty*(ph/2-r);
                double xa = l2;
                double ya = sty*(a.y - cc.y);
                double aa = sqrt(xa*xa + ya*ya);
                double theta = M_PI_2 - asin(r/aa) - asin(xa/aa);
                d2.x = cc.x - stx*r*sin(theta);
                d2.y = cc.y + sty*r*cos(theta);
            }
            */
        }
    }
    else
    {
        // Tail with an angle
        cc.x = c.x + stx*(pw/2-r);
        cc.y = c.y + sty*(ph/2-r);
        Vector v = a - cc;

        if (v.Length() <= r)
        {
            inside = true;
        }
        else
        {
            v.Normalize();
            alpha = sty*acos(v.x);
            d0.x = cc.x + r*cos(alpha);
            d0.y = cc.y + r*sin(alpha);

            /*
            double s1,s2;
            double l1,l2;
            bool tangent1 = false;
            bool tangent2 = false;

            if (v.x > 0 && v.y > 0)
            {
                s1 = r*alpha;
                l1 = s1 + (ph-2*r);

                if (d <= s1)
                {
                    d1.x = cc.x + r*cos(alpha-d/r);
                    d1.y = cc.y + r*sin(alpha-d/r);
                } 
                else if (d <= l1)
                {
                    d1.x = c.x + pw/2;
                    d1.y = c.y + (ph/2-r) - (d-s1);
                }
                else if ()
                {
                }
                else
                {
                    d1.x = c.x + pw/2;
                    d1.y = c.y - (ph/2-r);
                }

                s2 = r*(M_PI_2 - alpha);

                if (d <= s2)
                {
                    d2.x = cc.x + r*cos(alpha+d/r);
                    d2.y = cc.y + r*sin(alpha+d/r);
                }
                else if (d <= s2 + (pw-2*r))
                {
                    d2.x = c.x + (pw/2-r) - (d-s2);
                    d2.y = c.y + ph/2;
                }
                else
                {
                    d2.x = c.x - (pw/2-r);
                    d2.y = c.y + ph/2;
                }
            }
            */           
        }
    }

    if (!inside)
    {
        /*
        if (d == 0)
        {
        */
            rect.moveTo(d0.x, d0.y);
            rect.lineTo(a.x, a.y);
        /*
        }
        else
        {
            QPainterPath tail;
            tail.moveTo(c.x, c.y);
            tail.lineTo(d1.x, d1.y);
            tail.lineTo(a.x, a.y);
            tail.lineTo(d2.x, d2.y);
            tail.closeSubpath();
            rect += tail;
        }
        */
    }
    path.addQtPath(rect);
}


TAO_END
