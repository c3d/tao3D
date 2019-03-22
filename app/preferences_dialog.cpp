// *****************************************************************************
// preferences_dialog.cpp                                          Tao3D project
// *****************************************************************************
//
// File description:
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
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2013-2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2012, Jérôme Forissier <jerome@taodyne.com>
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

#include <QtGui>
#include <QListWidget>
#include <QListView>
#include <QStackedWidget>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>

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
    contentsWidget->setMaximumWidth(145);
    contentsWidget->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    contentsWidget->setSpacing(12);

    pagesWidget = new QStackedWidget;
    pagesWidget->addWidget(new GeneralPage);
    pagesWidget->addWidget(new PerformancesPage);
#ifndef CFG_NOMODPREF
    pagesWidget->addWidget(new ModulesPage);
#endif
#ifdef DEBUG
    pagesWidget->addWidget(new DebugPage);
#endif

    createIcons();
    contentsWidget->setCurrentRow(0);

    QHBoxLayout *horizontalLayout = new QHBoxLayout;
    horizontalLayout->addWidget(contentsWidget);
    horizontalLayout->addWidget(pagesWidget, 1);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(horizontalLayout);

#if !defined(Q_OS_MACX)
    QPushButton *closeButton = new QPushButton(tr("Close"));
    closeButton->setDefault(true);
    connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));
    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addStretch(1);
    buttonsLayout->addWidget(closeButton);
    mainLayout->addSpacing(12);
    mainLayout->addLayout(buttonsLayout);
#endif
    setLayout(mainLayout);

    resize(size() - QSize(0, 30));

    setWindowTitle(tr("Tao Preferences"));
}


void PreferencesDialog::createIcons()
// ----------------------------------------------------------------------------
//   Add one icon per preference page
// ----------------------------------------------------------------------------
{
    // general.png downloaded from:
    // http://www.iconfinder.com/icondetails/17814/128/preferences_settings_tools_icon
    // Author: Everaldo Coelho (Crystal Project)
    // License: LGPL
    QListWidgetItem *generalButton = new QListWidgetItem(contentsWidget);
    generalButton->setIcon(QIcon(":/images/general.png"));
    generalButton->setText(tr("General options"));
    generalButton->setTextAlignment(Qt::AlignHCenter);
    generalButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    // performances.png downloaded from:
    // http://www.iconfinder.com/icondetails/47542/128/performance_settings_speed_icon
    // Author: webiconset.com
    // License: Free for commercial use (Do not redistribute)
    QListWidgetItem *perfButton = new QListWidgetItem(contentsWidget);
    perfButton->setIcon(QIcon(":/images/performances.png"));
    perfButton->setText(tr("Performances"));
    perfButton->setTextAlignment(Qt::AlignHCenter);
    perfButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

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
