#ifndef SPLASH_SCREEN_H
#define SPLASH_SCREEN_H
// *****************************************************************************
// splash_screen.h                                                 Tao3D project
// *****************************************************************************
//
// File description:
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
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2010-2011, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010,2012-2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2012, Jérôme Forissier <jerome@taodyne.com>
// (C) 2010, Lionel Schaffhauser <lionel@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can r redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Tao3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Tao3D, in a file named COPYING.
// If not, see <https://www.gnu.org/licenses/>.
// *****************************************************************************

#include <QWidget>
#include <QSplashScreen>
#include <QTextDocument>

class QLabel;

namespace Tao {


struct SplashScreen : QSplashScreen
// ----------------------------------------------------------------------------
//    The Tao splash screen
// ----------------------------------------------------------------------------
{
    Q_OBJECT

public:
    SplashScreen(Qt::WindowFlags flags = 0);

public slots:
    virtual void showMessage(const QString &message,
                             int alignment = Qt::AlignBottom,
                             const QColor &color = QColor(255, 255, 255));

protected slots:
    void         openUrl(QString url);

protected:
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void drawContents(QPainter *painter);
    virtual bool event(QEvent *e);

    void         showCredits();
    void         showChangelog();

    QString      message;
    QLabel *     label;
    QLabel *     edition, * version;
    bool         mbPressed, urlClicked;
    QTextDocument licensedTo;

signals:
    void dismissed();
};

}

#endif // SPLASH_SCREEN_H
