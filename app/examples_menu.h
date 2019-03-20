#ifndef EXAMPLES_MENU_H
#define EXAMPLES_MENU_H
// ****************************************************************************
//  examples_menu.h                                                Tao project
// ****************************************************************************
//
//   File Description:
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
// ****************************************************************************
// This software is licensed under the GNU General Public License v3.
// See file COPYING for details.
//  (C) 2014 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2012 Jerome Forissier <jerome@taodyne.com>
//  (C) 2012 Taodyne SAS
// ****************************************************************************

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
