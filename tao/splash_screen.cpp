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
#include <QBitmap>
#include <QPainter>
#include <QTextDocument>
#include <QCoreApplication>

TAO_BEGIN

#define TEXT_COLOR "#FFFFFF"

SplashScreen::SplashScreen(Qt::WindowFlags flags)
    : QSplashScreen(QPixmap(":/images/splash.png"), flags)
// ----------------------------------------------------------------------------
//    Splash screen constructor: load the Tao bitmap and show program version
// ----------------------------------------------------------------------------
{
    setMask(QPixmap(":/images/splash.png").mask());
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

    int x = 270, y = 230, w = 500, h = 225;
    const char * txt =
            "<font color=\"" TEXT_COLOR "\">"
            "Brought to you by "
            "Anne Lempereur, "
            "Catherine Burvelle, "
            "J\303\251r\303\264me Forissier,<br>"
            "Lionel Schaffhauser and "
            "Christophe de Dinechin."
            "<br><br>"
            "\302\251 2010-2011 Taodyne SAS. All rights reserved."
            "</font>";
    QTextDocument doc;
    QRect clip = rect();
    clip.setRect(0, 0, w, h);
    painter->translate(x, y);
    doc.setHtml(trUtf8(txt));
    doc.drawContents(painter, clip);
    painter->translate(-x, -y);

    QString msg;
    msg = QString("<font color=\"" TEXT_COLOR "\">%1</font>").arg(message);
    x = 20;
    y = height() - 40;
    w = width() - 40;
    h = 20;
    clip.setRect(0, 0, w, h);
    painter->translate(x, y);
    doc.setHtml(msg);
    doc.drawContents(painter, clip);
}


void SplashScreen::showMessage(const QString &message, int, const QColor &)
// ----------------------------------------------------------------------------
//    Customized version of showMessage, fixed location and color
// ----------------------------------------------------------------------------
{
    this->message = message;
    QCoreApplication::processEvents();
    repaint();
}

TAO_END
