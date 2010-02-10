// ****************************************************************************
//  tao_widget.h                                                   XLR project
// ****************************************************************************
//
//   File Description:
//
//    The primary graphical widget used to display Tao contents
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
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include <QtOpenGL>
#include <QImage>
#include <QTimeLine>
#include <QSvgRenderer>
#include "main.h"


using namespace XL;

class TaoWidget : public QGLWidget
// ----------------------------------------------------------------------------
//   This is the widget we use to display XL programs output
// ----------------------------------------------------------------------------
{
    Q_OBJECT
public:
    TaoWidget(QWidget *parent, XL::SourceFile *sf = NULL);
    ~TaoWidget();

    // GL state
    void saveGLState();
    void restoreGLState();

    // Events
    void paintEvent(QPaintEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseDoubleClickEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void timerEvent(QTimerEvent *);
    void wheelEvent(QWheelEvent *);

    // XLR entry points
    static TaoWidget *Tao() { return current; }
    Tree *caption(Tree *self, text t);
    Tree *drawSvg(Tree *self, text t);

public slots:
    void draw();

private:
    QGLFramebufferObject *render_fbo;
    QGLFramebufferObject *texture_fbo;
    GLuint tile_list;

public:
    // XL Runtime
    XL::SourceFile *  xlProgram;
    text              caption_text;
    static TaoWidget *current;
};


#define TAO(x)  (TaoWidget::Tao() ? TaoWidget::Tao()->x : 0)
#define RTAO(x) return TAO(x)
