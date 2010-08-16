// ****************************************************************************
//  new_wizard.cpp                                                 Tao project
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

#include <QtGui>

#include "new_document_wizard.h"
#include "application.h"

TAO_BEGIN

NewDocumentWizard::NewDocumentWizard(QWidget *parent)
    : QWizard(parent)
{
    addPage(new ThemeChooserPage(this));
    addPage(new DocumentNameAndLocationPage(this));

    setWindowTitle(tr("New Document"));

    for (int i = 0; i < 6; i++)
    {
        themeNameList.append(tr("Theme %1").arg(i));
    }
}

void NewDocumentWizard::accept()
{
    QString docName = field("docName").toString();
    QString docLocation = field("docLocation").toString();
    QString themeName = themeNameList.at(field("themeIdx").toInt());

    QDir docDir(docLocation);
    if( !docDir.exists(docName) && !docDir.mkdir(docName) )
    {
        QMessageBox::warning(this,
                QObject::tr("New Document"),
                QObject::tr("Cannot create directory %1").arg(docName));
    }
    if( !docDir.cd(docName) )
    {
        QMessageBox::warning(this,
                QObject::tr("New Document"),
                QObject::tr("Cannot open directory %1").arg(docName));
    }

    QByteArray xl;
    xl += "theme \"" + themeName + "\"\n\n";
    xl += "slide \"Double-Click to Edit\"\n";
    xl += "    o \"Double-click to edit\"\n";

    QFile xlDoc(docDir.canonicalPath() + "/" + docName + ".ddd");
    if (!xlDoc.open(QFile::WriteOnly | QFile::Text))
    {
        QMessageBox::warning(this,
                QObject::tr("New Document"),
                QObject::tr("Cannot write file %1:\n%2")
                                .arg(xlDoc.fileName())
                                .arg(xlDoc.errorString()));
    }
    xlDoc.write(xl);

    QDialog::accept();
}

ThemeChooserPage::ThemeChooserPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Theme Chooser"));
    setSubTitle(tr("Choose the theme to apply to your slides."));

    themeListWidget = new QListWidget(this);
    themeListWidget->setViewMode(QListView::IconMode);
    themeListWidget->setIconSize(QSize(96, 72));
    themeListWidget->setMovement(QListView::Static);
    themeListWidget->setResizeMode(QListView::Adjust);
    themeListWidget->setMinimumWidth(144);
    themeListWidget->setSpacing(9);

    registerField("themeIdx*", themeListWidget);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(themeListWidget);
    setLayout(layout);
}

void ThemeChooserPage::initializePage()
{
    NewDocumentWizard * wiz = (NewDocumentWizard *)wizard();
    QStringList themes(wiz->themeNameList);

    for (int i = 0; i < themes.size(); i++)
    {
        QListWidgetItem *t = new QListWidgetItem(themeListWidget);
        t->setIcon(QIcon(":/images/default_image.svg"));
        t->setText(themes.at(i));
        t->setTextAlignment(Qt::AlignHCenter);
        t->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    }

    themeListWidget->setCurrentRow(0);
}

DocumentNameAndLocationPage::DocumentNameAndLocationPage(QWidget *parent)
    : QWizardPage(parent)
{
    setTitle(tr("Document Name and Location"));
    setSubTitle(tr("Specify the name and location of your document."));

    docNameLabel = new QLabel(tr("&Name:"), this);
    docNameLineEdit = new QLineEdit(this);
    docNameLabel->setBuddy(docNameLineEdit);

    docLocationLabel = new QLabel(tr("&Location:"), this);
    docLocationLineEdit = new QLineEdit(this);
    docLocationLineEdit->setReadOnly(true);
    docLocationLineEdit->setDisabled(true);
    docLocationLabel->setBuddy(docLocationLineEdit);    
    docLocationChooseButton = new QPushButton(tr("Choose..."), this);

    QObject::connect(docLocationChooseButton, SIGNAL(clicked()),
                     this,  SLOT(chooseLocation()));

    groupBox = new QGroupBox(tr("&Options"), this);

    copyImagesCheckBox = new QCheckBox(tr("&Copy images into document"), this);
    embedFontsCheckBox = new QCheckBox(tr("Embed &fonts into document"), this);
    copyThemeImagesCheckBox = new QCheckBox(tr("Copy &theme images into document"), this);

    registerField("docName*", docNameLineEdit);
    registerField("docLocation*", docLocationLineEdit);
    registerField("copyImages", copyImagesCheckBox);
    registerField("embedFonts", embedFontsCheckBox);
    registerField("copyThemeImages", copyThemeImagesCheckBox);

    QVBoxLayout *groupBoxLayout = new QVBoxLayout(this);
    groupBoxLayout->addWidget(copyImagesCheckBox);
    groupBoxLayout->addWidget(embedFontsCheckBox);
    groupBoxLayout->addWidget(copyThemeImagesCheckBox);
    groupBox->setLayout(groupBoxLayout);

    QGridLayout *layout = new QGridLayout(this);
    layout->addWidget(docNameLabel, 0, 0, Qt::AlignRight);
    layout->addWidget(docNameLineEdit, 0, 1);
    layout->addWidget(docLocationLabel, 1, 0, Qt::AlignRight);
    layout->addWidget(docLocationLineEdit, 1, 1);
    layout->addWidget(docLocationChooseButton, 1, 2);
    layout->addWidget(groupBox, 2, 0, 1, 3);
    setLayout(layout);
}

void DocumentNameAndLocationPage::initializePage()
{
    docLocationLineEdit->setText(Application::defaultProjectFolderPath());
}

void DocumentNameAndLocationPage::chooseLocation()
{
    QString location = QFileDialog::getExistingDirectory(this,
                           tr("Choose Document Location"),
                           Application::defaultProjectFolderPath(),
                           QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!location.isEmpty())
        docLocationLineEdit->setText(QDir::toNativeSeparators(location));
}

TAO_END

