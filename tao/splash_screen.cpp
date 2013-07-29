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

#include "version.h"
#include "license.h"
#include "splash_screen.h"
#include "tao_utf8.h"

#include <QtGui>

namespace Tao {

#define TEXT_COLOR "#FFFFFF"
#define LINK_COLOR "#EEAA11"

SplashScreen::SplashScreen(Qt::WindowFlags flags)
    : QSplashScreen(QPixmap(":/images/splash.png"), flags),
      mbPressed(false), urlClicked(false)
// ----------------------------------------------------------------------------
//    Splash screen constructor: load the Tao bitmap and show program version
// ----------------------------------------------------------------------------
{
    // Read license info
    QString s;
    QString name = +Licenses::Name();
    QString company = +Licenses::Company();
    if (name != "" || company != "")
    {
        s = QString("<font color=\"" TEXT_COLOR "\">%1</font>")
                .arg(QString("<b>%1</b><br>%2<br>%3")
                     .arg(tr("This product is licensed to:"))
                     .arg(name).arg(company));
        licensedTo.setHtml(s);
    }

    setMask(QPixmap(":/images/splash.png").mask());

    const char * fmt = "<html><head><style type=text/css>"
            "body {color:\"" TEXT_COLOR "\"}"
            "</style></head><body>%1</body></html>";
    edition = new QLabel(trUtf8(fmt).arg(tr("%1 Edition")
                                         .arg(Application::editionStr())), this);
    edition->move(25, 280);
    version = new QLabel(trUtf8(fmt).arg(tr("Version %1").arg(GITREV)), this);
    version->move(25, 300);

    const char * cop = "<html><head><style type=text/css>"
                       "body {color:\"" TEXT_COLOR "\"}"
                       "a:link {color:\"" LINK_COLOR "\"; "
                               "text-decoration:none;}"
                       "</style></head><body>"
                       "\302\251 2010-2013 "
                       "<a href=\"http://taodyne.com\">Taodyne SAS</a>. "
                       "%1 "
                       "<a href=\"credits:\">%2</a>. "
                       "<a href=\"changelog:\">%3</a>."
                       "</body></html>";
    label = new QLabel(trUtf8(cop).arg(tr("All rights reserved."))
                                  .arg(tr("Credits"))
                                  .arg(tr("News")), this);
    connect(label, SIGNAL(linkActivated(QString)),
            this,  SLOT(openUrl(QString)));
    label->move(270, 280);
}


void SplashScreen::openUrl(QString url)
// ----------------------------------------------------------------------------
//    Open Url
// ----------------------------------------------------------------------------
{
    urlClicked = true;
    if (url.startsWith("credits"))
        return showCredits();
    else if (url.startsWith("changelog"))
        return showChangelog();

    QDesktopServices::openUrl(QUrl(url));
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
    QRect clip;

    int x = 270, y = 220, w = 500, h = 225;
    if (!licensedTo.isEmpty())
    {
        clip.setRect(0, 0, w, h);
        painter->save();
        painter->translate(x, y);
        licensedTo.drawContents(painter, clip);
        painter->restore();
    }

    QString msg;
    msg = QString("<font color=\"" TEXT_COLOR "\">%1</font>").arg(message);
    x = 20;
    y = height() - 40;
    w = width() - 40;
    h = 20;
    clip.setRect(0, 0, w, h);
    painter->translate(x, y);
    QTextDocument doc;
    doc.setHtml(msg);
    doc.drawContents(painter, clip);
}


void SplashScreen::showMessage(const QString &message, int, const QColor &)
// ----------------------------------------------------------------------------
//    Customized version of showMessage, fixed location and color
// ----------------------------------------------------------------------------
{
    this->message = message;
    QPointer<SplashScreen> that(this);
    if (!that)
        return;
    repaint();
    QCoreApplication::processEvents();
}


void SplashScreen::showCredits()
// ----------------------------------------------------------------------------
//    Show credits dialog. Largely inspired from QMessageBox::aboutQt().
// ----------------------------------------------------------------------------
{
#ifdef Q_WS_MAC
    static QPointer<QMessageBox> oldMsgBox;

    if (oldMsgBox) {
        oldMsgBox->show();
        oldMsgBox->raise();
        oldMsgBox->activateWindow();
        return;
    }
#endif

    QString title = tr("Tao Presentations - Credits");
    QString translatedTextCreditsCaption;
    translatedTextCreditsCaption = tr(
        "<h3>Credits</h3>"
        "<p>This program uses the following components.</p>"
        );
    QString translatedTextCreditsText;
    translatedTextCreditsText = tr(
        "<h3>Qt %1</h3>"
        "<p>Qt is a C++ toolkit for cross-platform application "
        "development.</p>"
        "<p>See "
        "<a href=\"http://qt-project.org/\">qt-project.org</a> for more "
        "information.</p>"
        "<h3>Qt Solutions</h3>"
        "<p>This application contains portions of the "
        "Qt Solutions component.</p>"
        "<p>See the credits page under Help>Documentation for the "
        "complete legal notice of this component.</p>"
        "<h3>LLVM</h3>"
        "<p>The LLVM Project is a collection of modular and reusable compiler "
        "and toolchain technologies.</p>"
        "<p>See <a href=\"http://llvm.org/\">llvm.org</a> for more "
        "information.</p>"
        "<h3>XLR</h3>"
        "<p>XLR is a dynamically-compiled language based on parse tree "
        "rewrites.</p>"
        "<p>See <a href=\"http://xlr.sf.net/\">xlr.sf.net</a> for more "
        "information.</p>"
        ).arg(QT_VERSION_STR);
    QMessageBox *msgBox = new QMessageBox;
    msgBox->setAttribute(Qt::WA_DeleteOnClose);
    msgBox->setWindowTitle(title);
    msgBox->setText(translatedTextCreditsCaption);
    msgBox->setInformativeText(translatedTextCreditsText);

    QPixmap pm(":/images/tao_picto.png");
    if (!pm.isNull())
    {
        QPixmap scaled = pm.scaled(64, 64, Qt::IgnoreAspectRatio,
                                   Qt::SmoothTransformation);
        msgBox->setIconPixmap(scaled);
    }

    msgBox->raise();
#ifdef Q_WS_MAC
    oldMsgBox = msgBox;
    msgBox->show();
#else
    msgBox->exec();
#endif
}

void SplashScreen::showChangelog()
// ----------------------------------------------------------------------------
//    Show changelog dialog. Inspired from showCredits().
// ----------------------------------------------------------------------------
{
#ifdef Q_WS_MAC
    static QPointer<QDialog> oldDialog;

    if (oldDialog) {
        oldDialog->show();
        oldDialog->raise();
        oldDialog->activateWindow();
        return;
    }
#endif

    QString title = tr("Tao Presentations - What's new?");
    QString changelogText;
    QFile changelog(":/NEWS");
    if (changelog.open(QFile::ReadOnly | QFile::Text))
    {
        QByteArray ba = changelog.readAll();
        changelogText = QString::fromUtf8(ba.data(), ba.length());
    }
    QDialog *dialog = new QDialog;
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->setWindowTitle(title);
    QTextEdit *edit = new QTextEdit;
    edit->setPlainText(changelogText);
    edit->setLineWrapMode(QTextEdit::NoWrap);
    edit->setReadOnly(true);
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(edit);
    dialog->setLayout(layout);
    dialog->resize(650, 550);

    dialog->raise();
#ifdef Q_WS_MAC
    oldDialog = dialog;
    dialog->show();
#else
    dialog->exec();
#endif
}

}
