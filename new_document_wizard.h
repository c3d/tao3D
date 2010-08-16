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
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 2010 Lionel Schaffhauser <lionel@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include <QWizard>

#include "tao.h"

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
    QStringList themeNameList;
};


class ThemeChooserPage : public QWizardPage
{
    Q_OBJECT

public:
    ThemeChooserPage(QWidget *parent = 0);

protected:
    void initializePage();

private:
    QListWidget *themeListWidget;
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
    QGroupBox *groupBox;
    QCheckBox *copyImagesCheckBox;
    QCheckBox *embedFontsCheckBox;
    QCheckBox *copyThemeImagesCheckBox;
};

}

#endif // NEW_DOCUMENT_WIZARD_H
