// ****************************************************************************
//  glwidget.h                                                      XLR project
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
//  (C) 2010 Taodyne SAS
// ****************************************************************************
// 
//   File Description:
// 
//     Description of the GL widget (derived from original Qt example)
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
// ****************************************************************************
// * File       : $RCSFile$
// * Revision   : $Revision$
// * Date       : $Date$
// ****************************************************************************

#include <QtOpenGL>
#include <QImage>
#include <QTimeLine>
#include <QSvgRenderer>
#include "main.h"

class GLWidget : public QGLWidget
// ----------------------------------------------------------------------------
//   This is the widget we use to display the runtime environment
// ----------------------------------------------------------------------------
{
    Q_OBJECT

public:
    GLWidget(QWidget *parent, XL::Main *xlr);
    ~GLWidget();

    void saveGLState();
    void restoreGLState();

    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseDoubleClickEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void timerEvent(QTimerEvent *);
    void wheelEvent(QWheelEvent *);
                             
    void caption(text t) { caption_text = t; }
     
    static GLWidget *current;

public slots:
    void animate(qreal);
    void animFinished();
    void draw();

private:
    QPoint anchor;
    float scale;
    float rot_x, rot_y, rot_z;
    GLuint tile_list;
    GLfloat *wave;

    QImage logo;
    QTimeLine *anim;
    QSvgRenderer *svg_renderer;
    QGLFramebufferObject *render_fbo;
    QGLFramebufferObject *texture_fbo;

private:
    // XL Runtime
    XL::Main *xl_runtime;
    text  caption_text;
};


#define GW (*GLWidget::current)
