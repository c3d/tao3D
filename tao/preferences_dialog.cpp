// ****************************************************************************
//  preferences_dialog.cpp                                         Tao project
// ****************************************************************************
//
//   File Description:
//
//     PreferencesDialog implementation.
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

#include <QtGui>

#include "preferences_dialog.h"
#include "preferences_pages.h"

namespace Tao {

PreferencesDialog::PreferencesDialog(QWidget *parent)
// ----------------------------------------------------------------------------
//   Create the preference dialog
// ----------------------------------------------------------------------------
    : QDialog(parent)
{
    contentsWidget = new QListWidget;
    contentsWidget->setViewMode(QListView::IconMode);
    contentsWidget->setIconSize(QSize(96, 96));
    contentsWidget->setMovement(QListView::Static);
    contentsWidget->setMinimumHeight(280);
    contentsWidget->setMaximumWidth(140);
    contentsWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    contentsWidget->setSpacing(12);

    pagesWidget = new QStackedWidget;
#ifndef CFG_NOMODPREF
    pagesWidget->addWidget(new ModulesPage);
#endif
#ifdef DEBUG
    pagesWidget->addWidget(new DebugPage);
#endif

    QPushButton *closeButton = new QPushButton(tr("Close"));
    closeButton->setDefault(true);

    createIcons();
    contentsWidget->setCurrentRow(0);

    connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));

    QHBoxLayout *horizontalLayout = new QHBoxLayout;
    horizontalLayout->addWidget(contentsWidget);
    horizontalLayout->addWidget(pagesWidget, 1);

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addStretch(1);
    buttonsLayout->addWidget(closeButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(horizontalLayout);
    mainLayout->addSpacing(12);
    mainLayout->addLayout(buttonsLayout);
    setLayout(mainLayout);

    setWindowTitle(tr("Tao Preferences"));
}


void PreferencesDialog::createIcons()
// ----------------------------------------------------------------------------
//   Add one icon per preference page
// ----------------------------------------------------------------------------
{
#ifndef CFG_NOMODPREF
    // modules.png downloaded from:
    // http://www.iconfinder.com/icondetails/17854/128/cubes_modules_icon
    // Author: Everaldo Coelho (Crystal Project)
    // License: LGPL
    QListWidgetItem *modulesButton = new QListWidgetItem(contentsWidget);
    modulesButton->setIcon(QIcon(":/images/modules.png"));
    modulesButton->setText(tr("Module options"));
    modulesButton->setTextAlignment(Qt::AlignHCenter);
    modulesButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
#endif

#ifdef DEBUG
    // bug.png downloaded from:
    // http://www.iconfinder.com/icondetails/17857/128/animal_bug_insect_ladybird_icon
    // Author: Everaldo Coelho (Crystal Project)
    // License: LGPL
    QListWidgetItem *debugButton = new QListWidgetItem(contentsWidget);
    debugButton->setIcon(QIcon(":/images/bug.png"));
    debugButton->setText(tr("Debug options"));
    debugButton->setTextAlignment(Qt::AlignHCenter);
    debugButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
#endif

    connect(contentsWidget,
            SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(changePage(QListWidgetItem*,QListWidgetItem*)));
}


void PreferencesDialog::changePage(QListWidgetItem *current,
                                   QListWidgetItem *previous)
// ----------------------------------------------------------------------------
//   Change current page when icon is clicked in contents widget
// ----------------------------------------------------------------------------
{
    if (!current)
        current = previous;

    pagesWidget->setCurrentIndex(contentsWidget->row(current));
}

}
