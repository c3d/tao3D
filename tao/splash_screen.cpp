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
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
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
#include <QLabel>
#include <QDesktopServices>
#include <QUrl>

TAO_BEGIN

#define TEXT_COLOR "#FFFFFF"
#define LINK_COLOR "#EEAA11"

SplashScreen::SplashScreen(Qt::WindowFlags flags)
    : QSplashScreen(QPixmap(":/images/splash.png"), flags),
      mbPressed(false), urlClicked(false)
// ----------------------------------------------------------------------------
//    Splash screen constructor: load the Tao bitmap and show program version
// ----------------------------------------------------------------------------
{
    setMask(QPixmap(":/images/splash.png").mask());
    QString version(QObject::tr("Version %1").arg(GITREV));
    showMessage(version);

    const char * cop = "<html><head><style type=text/css>"
                       "body {color:\"" TEXT_COLOR "\"}"
                       "a:link {color:\"" LINK_COLOR "\"; "
                               "text-decoration:none;}"
                       "</style></head><body>"
                       "\302\251 2010-2011 "
                       "<a href=\"http://taodyne.com\">Taodyne SAS</a>. "
                       "All rights reserved."
                       "</body></html>";
    label = new QLabel(trUtf8(cop), this);
    connect(label, SIGNAL(linkActivated(QString)),
            this,  SLOT(openUrl(QString)));
    label->move(270, 280);
}


void SplashScreen::openUrl(QString url)
// ----------------------------------------------------------------------------
//    Open Url
// ----------------------------------------------------------------------------
{
    QDesktopServices::openUrl(QUrl(url));
    urlClicked = true;
}


bool SplashScreen::event(QEvent *event)
// ----------------------------------------------------------------------------
//    Handle event
// ----------------------------------------------------------------------------
{
    // This is an "good enough" attempt at implementing hyperlinks on the
    // splash screen.
    // Click on a hyperlink to open a web browser. Click the splash screen
    // anywhere else to dismiss it (it is closed on MB up).
    switch (event->type())
    {
    case QEvent::MouseButtonRelease:
        hide();
        emit dismissed();
        return true;
        break;

    case QEvent::Paint:
        // NB: we get no MouseButtonRelease event when URL is clicked, but we
        // get a Paint event
        if (urlClicked)
        {
            hide();
            emit dismissed();
            urlClicked = false;
            return true;
        }
        break;

    default:
        break;
    }
    return QSplashScreen::event(event);
}


void SplashScreen::mousePressEvent(QMouseEvent *)
// ----------------------------------------------------------------------------
//    Handle mouse click: do not dismiss splash screen immediately
// ----------------------------------------------------------------------------
{
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
            "by "
            "Catherine Burvelle, "
            "J\303\251r\303\264me Forissier and "
            "Christophe de Dinechin,<br>" 
            "with help from Anne Lempereur and Lionel Schaffhauser."
           "</font>";
    QTextDocument doc;
    QRect clip = rect();
    clip.setRect(0, 0, w, h);
    painter->save();
    painter->translate(x, y);
    doc.setHtml(trUtf8(txt));
    doc.drawContents(painter, clip);
    painter->restore();

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
