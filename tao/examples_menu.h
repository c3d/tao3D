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
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 2012 Jerome Forissier <jerome@taodyne.com>
//  (C) 2012 Taodyne SAS
// ****************************************************************************


#include <QMenu>

namespace Tao {

class ExamplesMenu : public QMenu
// ----------------------------------------------------------------------------
//   Menu with entries to open Tao examples
// ----------------------------------------------------------------------------
{
    Q_OBJECT

public:
    ExamplesMenu(QWidget *parent = 0);
    virtual ~ExamplesMenu();
    void addExample(QString caption, QString path, QString tip = "", QString type = "theme");

protected:
    QMenu *themesMenu;
    QMenu *miscMenu;

signals:
    void openDocument(QString path);

protected slots:
    void actionTriggered();
};

}

#endif // EXAMPLES_MENU_H
