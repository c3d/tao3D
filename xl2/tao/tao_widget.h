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
    
public:
    // XLR entry points
    static TaoWidget *Tao() { return current; }
    Tree *caption(Tree *self, text t);
    Tree *drawSvg(Tree *self, text t);

    Tree *rotateX(Tree *self, double rx);
    Tree *rotateY(Tree *self, double ry);
    Tree *rotateZ(Tree *self, double rz);
    Tree *rotate(Tree *self, double rx, double ry, double rz, double ra);

    Tree *translateX(Tree *self, double x);
    Tree *translateY(Tree *self, double y);
    Tree *translateZ(Tree *self, double z);
    Tree *translate(Tree *self, double x, double y, double z);

    Tree *color(Tree *self, double r, double g, double b, double a);

    Tree *refresh(Tree *self, double delay);

public slots:
    void draw();

public:
    // XL Runtime
    XL::SourceFile *  xlProgram;
    text              caption_text;
    static TaoWidget *current;
};


#define TAO(x)  (TaoWidget::Tao() ? TaoWidget::Tao()->x : 0)
#define RTAO(x) return TAO(x)
