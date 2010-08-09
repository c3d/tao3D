// ****************************************************************************
//  splash_screen.cpp                                               Tao project
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

#include "tao.h"
#include "version.h"
#include "splash_screen.h"

#include <QSplashScreen>
#include <QPixmap>
#include <QPainter>
#include <QTextDocument>

TAO_BEGIN

#define TEXT_COLOR "#666666"

SplashScreen::SplashScreen(Qt::WindowFlags flags)
    : QSplashScreen(QPixmap(":/images/splash.png"), flags)
// ----------------------------------------------------------------------------
//    Splash screen constructor: load the Tao bitmap and show program version
// ----------------------------------------------------------------------------
{
    QString version(QObject::tr("Version %1").arg(GITREV));
    showMessage(version);
}


void SplashScreen::mousePressEvent(QMouseEvent *event)
// ----------------------------------------------------------------------------
//    Dismiss the splash screen
// ----------------------------------------------------------------------------
{
    QSplashScreen::mousePressEvent(event);
    emit dismissed();
}


void SplashScreen::drawContents(QPainter *painter)
// ----------------------------------------------------------------------------
//    Draw the splash screen, including custom text area
// ----------------------------------------------------------------------------
{
    QSplashScreen::drawContents(painter);

    const int x = 280, y = 166, w = 455, h = 225;
    const char * txt =
            "<font color=\"" TEXT_COLOR "\"><br/><br/>"
            "Brought to you by:<br/><br/>"
            "Anne Lempereur<br/>"
            "Catherine Burvelle<br/>"
            "J\303\251r\303\264me Forissier<br/>"
            "Lionel Schaffhauser<br/>"
            "Christophe de Dinechin<br/>"
            "</font>";
    QTextDocument doc;
    QRect clip = rect();
    clip.setRect(0, 0, w, h);
    painter->translate(x, y);
    doc.setHtml(trUtf8(txt));
    doc.drawContents(painter, clip);
}


void SplashScreen::showMessage(const QString &message, int alignment,
                               const QColor &color)
// ----------------------------------------------------------------------------
//    Show message with our default alignment and color
// ----------------------------------------------------------------------------
{
    QSplashScreen::showMessage(message, alignment, color);
}

TAO_END
