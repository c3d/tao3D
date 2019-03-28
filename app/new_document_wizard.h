#ifndef NEW_DOCUMENT_WIZARD_H
#define NEW_DOCUMENT_WIZARD_H
// *****************************************************************************
// new_document_wizard.h                                           Tao3D project
// *****************************************************************************
//
// File description:
//
//    Wizard for creating new documents
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
// (C) 2010-2011, Catherine Burvelle <catherine@taodyne.com>
// (C) 2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2011,2013, Jérôme Forissier <jerome@taodyne.com>
// (C) 2010, Lionel Schaffhauser <lionel@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can redistribute it and/or modify
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

#include <QWizard>

#include "tao.h"
#include "templates.h"

QT_BEGIN_NAMESPACE
class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QListWidget;
class QRadioButton;
class QPushButton;
QT_END_NAMESPACE

namespace Tao {

class CachedTexture;

class NewDocumentWizard : public QWizard
{
    Q_OBJECT

public:
    NewDocumentWizard(QWidget *parent = 0);

    void accept();

public:
    Templates templates;
    QString   docPath;

protected slots:
    void         pageChanged(int id);
    void         openWebPage();

protected:
    QPushButton *moreButton;
};


class TemplateChooserPage : public QWizardPage
{
    Q_OBJECT

public:
    TemplateChooserPage(QWidget *parent = 0);

protected:
    void initializePage();

protected slots:
    void updateDescription();
    void filterItems();
    void thumbnailChanged(CachedTexture *tex);

private:
    QListWidget *templateListWidget;
    QLabel      *description;
    QLineEdit   *search;
    QCheckBox   *showAll;
};


class DocumentNameAndLocationPage : public QWizardPage
{
    Q_OBJECT

public:
    DocumentNameAndLocationPage(QWidget *parent = 0);

protected:
    void initializePage();

protected slots:
    void chooseLocation();

private:
    QLabel *docNameLabel;
    QLabel *docLocationLabel;
    QLineEdit *docNameLineEdit;
    QLineEdit *docLocationLineEdit;
    QPushButton *docLocationChooseButton;
};

}

#endif // NEW_DOCUMENT_WIZARD_H
