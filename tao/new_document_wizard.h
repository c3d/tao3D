#ifndef NEW_DOCUMENT_WIZARD_H
#define NEW_DOCUMENT_WIZARD_H
// ****************************************************************************
//  new_wizard.h                                                   Tao project
// ****************************************************************************
//
//   File Description:
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
// ****************************************************************************
// This software is licensed under the GNU General Public License v3.
// See file COPYING for details.
//  (C) 2011 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Lionel Schaffhauser <lionel@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

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
