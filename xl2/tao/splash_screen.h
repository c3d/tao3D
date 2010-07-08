#ifndef SPLASH_SCREEN_H
#define SPLASH_SCREEN_H
// ****************************************************************************
//  splash_screen.h                                                 Tao project
// ****************************************************************************
//
//   File Description:
//
//     The Tao splash screen, shown at startup and as an "About" box
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
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include <QSplashScreen>

namespace Tao {

class SplashScreen : public QSplashScreen
// ----------------------------------------------------------------------------
//    The Tao splash screen
// ----------------------------------------------------------------------------
{
    Q_OBJECT

public:
    SplashScreen();
    virtual void mousePressEvent(QMouseEvent *event);

protected:
    virtual void drawContents(QPainter *painter);

    QString      textColor;

signals:
    void dismissed();
};

}

#endif // SPLASH_SCREEN_H
