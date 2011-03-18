#ifndef NEW_DOCUMEMT_WIZARD_H
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
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
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
};


class TemplateChooserPage : public QWizardPage
{
    Q_OBJECT

public:
    TemplateChooserPage(QWidget *parent = 0);

protected:
    void initializePage();

private:
    QListWidget *templateListWidget;
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
