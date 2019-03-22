// *****************************************************************************
// destination_folder_dialog.cpp                                   Tao3D project
// *****************************************************************************
//
// File description:
//
//    A dialog box for the user to select a destination folder. There is a
//    drop-down list of known locations (paths), and fields to select any other
//    folder.
//
//
//
//
//
//
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2011, Catherine Burvelle <catherine@taodyne.com>
// (C) 2013-2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2011-2012, Jérôme Forissier <jerome@taodyne.com>
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

#include "destination_folder_dialog.h"
#include "application.h"
#include <QDialog>
#include <QComboBox>
#include <QLineEdit>
#include <QGridLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QFileDialog>

namespace Tao {

DestinationFolderDialog::DestinationFolderDialog(QString defaultPath,
                                                 QStringList &knownLocations,
                                                 QWidget *parent)
// ----------------------------------------------------------------------------
//    Create a dialog box
// ----------------------------------------------------------------------------
    : QDialog(parent), knownLocations(knownLocations), saved(defaultPath),
      combo(NULL), rejected(false)
{
    setWindowTitle(tr("Select destination folder"));
    QGridLayout *grid = new QGridLayout;
    grid->addWidget(new QLabel(tr("Select destination folder")), 0, 0, 1, 3);
    if (!knownLocations.empty())
    {
        combo = new QComboBox;
        combo->addItem(tr("New folder"));
        combo->addItems(knownLocations);
        connect(combo, SIGNAL(activated(int)),
                this, SLOT(comboBoxIndexChanged(int)));
        grid->addWidget(combo, 1, 0, 1, 3);
    }
    else
    {
        grid->setRowStretch(0, 1);
    }

    QLabel *nameLabel = new QLabel(tr("&Name:"), this);
    grid->addWidget(nameLabel, 2, 0, Qt::AlignRight);
    nameLineEdit = new QLineEdit(this);
    connect(nameLineEdit, SIGNAL(textEdited(QString)),
            this, SLOT(nameChanged()));
    nameLabel->setBuddy(nameLineEdit);
    grid->addWidget(nameLineEdit, 2, 1);
    QLabel *locationLabel = new QLabel(tr("&Location:"), this);
    grid->addWidget(locationLabel, 3, 0, Qt::AlignRight);
    locationLineEdit = new QLineEdit(this);
    locationLineEdit->setDisabled(true);
    connect(locationLineEdit, SIGNAL(textEdited(QString)),
            this, SLOT(locationChanged()));
    grid->addWidget(locationLineEdit, 3, 1);
    locationLabel->setBuddy(locationLineEdit);
    locationChooseButton = new QPushButton(tr("Choose..."), this);
    locationChooseButton->setDefault(false);
    locationChooseButton->setAutoDefault(false);
    connect(locationChooseButton, SIGNAL(clicked()),
            this,  SLOT(chooseLocation()));
    grid->addWidget(locationChooseButton, 3, 2);
    QDialogButtonBox *bbox = new QDialogButtonBox(QDialogButtonBox::Ok |
                                                  QDialogButtonBox::Cancel);
    connect(bbox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(bbox, SIGNAL(rejected()), this, SLOT(reject()));
    grid->addWidget(bbox, 4, 0, 1, 3);

    setLayout(grid);
    resize(550, 200);

    updateNameAndLocation(defaultPath);

    if (knownLocations.size() == 1)
    {
        combo->setCurrentIndex(1);
        comboBoxIndexChanged(1);
    }
}


QString DestinationFolderDialog::target()
// ----------------------------------------------------------------------------
//    Return currently selected path
// ----------------------------------------------------------------------------
{
    if (rejected)
        return "";
    QString location = locationLineEdit->text();
    QString name = nameLineEdit->text();
    QDir dir(location + "/" + name);
    return QDir::toNativeSeparators(dir.absolutePath());
}


void DestinationFolderDialog::comboBoxIndexChanged(int index)
// ----------------------------------------------------------------------------
//    Update the name and location fields when user changes combo box
// ----------------------------------------------------------------------------
{
    QString path = saved;
    if (index)
        path = combo->itemText(index);
    nameLineEdit->setEnabled(index == 0);
    locationChooseButton->setEnabled(index == 0);
    updateNameAndLocation(path);
}


void DestinationFolderDialog::nameChanged()
// ----------------------------------------------------------------------------
//    Update saved path when user changes project name (if not preset path)
// ----------------------------------------------------------------------------
{
    saved = target();
}


void DestinationFolderDialog::locationChanged()
// ----------------------------------------------------------------------------
//    Update saved path when user changes project location (if not preset path)
// ----------------------------------------------------------------------------
{
    saved = target();
}


void DestinationFolderDialog::chooseLocation()
// ----------------------------------------------------------------------------
//   Open a folder selection dialog to select target folder
// ----------------------------------------------------------------------------
{
    QString location = QFileDialog::getExistingDirectory(this,
                           tr("Choose Document Location"),
                           Application::defaultProjectFolderPath(),
                           QFileDialog::ShowDirsOnly |
                           QFileDialog::DontResolveSymlinks);
    if (!location.isEmpty())
        locationLineEdit->setText(QDir::toNativeSeparators(location));
}


void DestinationFolderDialog::updateNameAndLocation(QString path)
// ----------------------------------------------------------------------------
//   Split folder path into parent folder (location) and directory name
// ----------------------------------------------------------------------------
{
    QDir dir(path);
    QString name = dir.dirName();
    nameLineEdit->setText(name);
    dir.cdUp();
    QString location = QDir::toNativeSeparators(dir.absolutePath());
    locationLineEdit->setText(location);
}


void DestinationFolderDialog::accept()
// ----------------------------------------------------------------------------
//    Update destination folder path and close dialog
// ----------------------------------------------------------------------------
{
    QDialog::accept();
}


void DestinationFolderDialog::reject()
// ----------------------------------------------------------------------------
//    Clear destination folder path and close dialog
// ----------------------------------------------------------------------------
{
    rejected = true;
    QDialog::reject();
}

}
