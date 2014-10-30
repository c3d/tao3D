// ****************************************************************************
//  examples_menu.h                                                Tao project
// ****************************************************************************
//
//   File Description:
//
//    Create a menu with several entries to open examples
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
//  (C) 2012 Jerome Forissier <jerome@taodyne.com>
//  (C) 2012 Taodyne SAS
// ****************************************************************************


#include "examples_menu.h"
#include "application.h"
#include <QAction>
#include <QFileInfo>
#include <QVariant>


namespace Tao {

ExamplesMenu::ExamplesMenu(QString caption, QWidget *parent) 
// ----------------------------------------------------------------------------
//   Constructor
// ----------------------------------------------------------------------------
    : QMenu(caption, parent)
{}


ExamplesMenu::~ExamplesMenu()
// ----------------------------------------------------------------------------
//   Destructor
// ----------------------------------------------------------------------------
{}


void ExamplesMenu::addExample(QString caption, QString path, QString tip)
// ----------------------------------------------------------------------------
//   Add an entry to the menu, linking to a particular document
// ----------------------------------------------------------------------------
{
    // If path is relative, prepend the templates directory
    if (!path.contains("://") && !QFileInfo(path).isAbsolute())
        path = QCoreApplication::applicationDirPath() + "/templates/" + path;

    QAction * action = new QAction(caption, this);
    action->setData(QVariant(path));
    if (tip != "")
        action->setToolTip(tip);
    connect(action, SIGNAL(triggered()), this, SLOT(actionTriggered()));

    addAction(action);
}


void ExamplesMenu::actionTriggered()
// ----------------------------------------------------------------------------
//   Emit signal with the path of the document to be opened
// ----------------------------------------------------------------------------
{
    QAction * action = dynamic_cast<QAction *>(sender());
    QString path = action->data().toString();
    emit openDocument(path);
}

}
