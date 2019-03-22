#ifndef EXAMPLES_MENU_H
#define EXAMPLES_MENU_H
// *****************************************************************************
// examples_menu.h                                                 Tao3D project
// *****************************************************************************
//
// File description:
//
//    Create a menu with several entries that open examples
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
// (C) 2013, Baptiste Soulisse <baptiste.soulisse@taodyne.com>
// (C) 2013-2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2012, Jérôme Forissier <jerome@taodyne.com>
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

#include <QMenu>
#include <QMap>
#include <QNetworkAccessManager>
#include <QNetworkReply>



namespace Tao {

class ExamplesMenu : public QMenu
// ----------------------------------------------------------------------------
//   Menu with entries to open Tao examples
// ----------------------------------------------------------------------------
{
    Q_OBJECT

public:
    ExamplesMenu(QString caption, QWidget *parent = 0);
    virtual ~ExamplesMenu();
    ExamplesMenu *addExample(QString caption, QString url, QString tip = "");

signals:
    void openDocument(QString path);

protected slots:
    void actionTriggered();

public:
    typedef QMap<QString, ExamplesMenu *> submenus_t;

private:
    submenus_t  submenus;
};


class ExamplesMenuUpdater : public QObject
// ----------------------------------------------------------------------------
//   A class that downloads the latest set of examples and updates settings
// ----------------------------------------------------------------------------
{
    Q_OBJECT
public:
    ExamplesMenuUpdater(QUrl url);

public slots:
    void        downloaded(QNetworkReply *reply);

public:
    QNetworkAccessManager       network;
};

}

#endif // EXAMPLES_MENU_H
