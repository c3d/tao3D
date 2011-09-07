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
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 2011 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Lionel Schaffhauser <lionel@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include <QtGui>

#include "new_document_wizard.h"
#include "application.h"
#include "repository.h"

namespace Tao {

NewDocumentWizard::NewDocumentWizard(QWidget *parent)
// ----------------------------------------------------------------------------
//   Create the wizard
// ----------------------------------------------------------------------------
    : QWizard(parent)
{
    setWindowTitle(tr("New Document"));

    moreButton = new QPushButton(tr("More from the web..."));
    connect(moreButton, SIGNAL(clicked()), this, SLOT(openWebPage()));
    setButton(QWizard::CustomButton1, moreButton);

    QPushButton *next =
            dynamic_cast<QPushButton *>(button(QWizard::NextButton));
    Q_ASSERT(next);
    next->setDefault(true);

    addPage(new TemplateChooserPage(this));
    addPage(new DocumentNameAndLocationPage(this));

    connect(this, SIGNAL(currentIdChanged(int)), this, SLOT(pageChanged(int)));
}


void NewDocumentWizard::pageChanged(int id)
// ----------------------------------------------------------------------------
//   Show the "More from the web..." button only on first page
// ----------------------------------------------------------------------------
{
    setOption(QWizard::HaveCustomButton1, id == 0);
}


void NewDocumentWizard::openWebPage()
// ----------------------------------------------------------------------------
//   Open the templates web page in a browser window and close the wizard
// ----------------------------------------------------------------------------
{
    QString url("http://taodyne.com/taopresentations/1.0/templates/");
    QDesktopServices::openUrl(url);
    reject();
}


void NewDocumentWizard::accept()
// ----------------------------------------------------------------------------
//   Copy template into user's document folder
// ----------------------------------------------------------------------------
{
    QString docName = field("docName").toString();
    QString docLocation = field("docLocation").toString();
    QString dstPath = docLocation + "/" + docName;

    QDir dst(dstPath);
    if (dst.exists())
    {
        QString dstPathNative = QDir::toNativeSeparators(dstPath);
        int r = QMessageBox::warning(this, tr("Folder exists"),
                    tr("Document folder:\n%1\nalready exists. "
                       "Do you want to use it anyway?\n"
                       "Click No to choose another location.")
                       .arg(dstPathNative),
                       QMessageBox::Yes | QMessageBox::No);
        if (r != QMessageBox::Yes)
            return;
    }

    Template t = templates.at(field("templateIdx").toInt());

    bool ok = t.copyTo(dst);
    if (!ok)
    {
        QMessageBox::warning(this, tr("Error"),
            tr("Failed to copy document template."));
        return;
    }

    docPath = dstPath;
    if (t.mainFile != "")
    {
        QString oldPath = dstPath + "/" + t.mainFile;
        QString newPath = dstPath + "/" + docName + ".ddd";
        if (oldPath != newPath)
        {
            // Rename template main file to doc name.
            // Don't use QDir::rename nor QFile::rename because these
            // methods assume that the destination file does not exist.
            // We want to overwrite any existing file.
            QFile src(oldPath);
            QFile dst(newPath);
            src.open(QIODevice::ReadOnly);
            dst.open(QIODevice::ReadWrite | QIODevice::Truncate);
            QByteArray data = src.readAll();
            dst.write(data);
            dst.close();
            src.close();
            QDir(dstPath).remove(t.mainFile);
        }
        docPath = newPath;
    }

    // Create project to avoid prompt when document is first opened
    RepositoryFactory::repository(dstPath, RepositoryFactory::Create);

    QDialog::accept();
}

TemplateChooserPage::TemplateChooserPage(QWidget *parent)
// ----------------------------------------------------------------------------
//   Create the template selection page
// ----------------------------------------------------------------------------
    : QWizardPage(parent)
{
    setTitle(tr("Template Chooser"));
    setSubTitle(tr("Choose a template to create your document with."));

    search = new QLineEdit;
    search->setPlaceholderText(tr("Search"));
    connect(search, SIGNAL(textChanged(QString)), this, SLOT(doSearch()));
    QHBoxLayout *searchLayout = new QHBoxLayout;
    searchLayout->addStretch(2);
    searchLayout->addWidget(search, 1);

    templateListWidget = new QListWidget;
    templateListWidget->setViewMode(QListView::IconMode);
    templateListWidget->setIconSize(QSize(96, 72));
    templateListWidget->setMovement(QListView::Static);
    templateListWidget->setResizeMode(QListView::Adjust);
    templateListWidget->setMinimumWidth(3 * 144 + 25);
    templateListWidget->setMinimumHeight(2 * 144);
    templateListWidget->setGridSize(QSize(144, 144));
    templateListWidget->setWordWrap(true);
    connect(templateListWidget, SIGNAL(itemSelectionChanged()),
            this, SLOT(updateDescription()));
    NewDocumentWizard *wiz = (NewDocumentWizard *)parent;
    connect(templateListWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)),
            wiz, SLOT(next()));

    registerField("templateIdx*", templateListWidget);

    description = new QLabel;
    description->setWordWrap(true);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addLayout(searchLayout);
    layout->addWidget(templateListWidget);
    layout->addWidget(description);
    setLayout(layout);
}


void TemplateChooserPage::initializePage()
// ----------------------------------------------------------------------------
//   Fill the template list with the templates we find on the disk
// ----------------------------------------------------------------------------
{
    NewDocumentWizard * wiz = (NewDocumentWizard *)wizard();
    QList<QDir> dirs;
    dirs << QDir(TaoApp->applicationDirPath() + "/templates")
         << QDir(TaoApp->defaultTaoPreferencesFolderPath() + "/templates");
    wiz->templates = Templates(dirs);
    foreach (Template tmpl, wiz->templates)
    {
        QListWidgetItem *t = new QListWidgetItem(templateListWidget);
        QPixmap pm(tmpl.thumbnail);
        if (pm.isNull())
            pm = QPixmap(":/images/default_image.svg");
        t->setIcon(QIcon(pm));
        t->setText(tmpl.name);
        t->setTextAlignment(Qt::AlignHCenter);
        t->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    }

    templateListWidget->setCurrentRow(0);
}


void TemplateChooserPage::updateDescription()
// ----------------------------------------------------------------------------
//   Set the description text for the currently slected item
// ----------------------------------------------------------------------------
{
    QString desc;
    NewDocumentWizard * wiz = (NewDocumentWizard *)wizard();
    int idx = field("templateIdx").toInt();
    if (idx != -1)
    {
        Template t = wiz->templates.at(idx);
        desc = t.description;
    }
    description->setText(desc);
}


void TemplateChooserPage::doSearch()
// ----------------------------------------------------------------------------
//   Show or hide items, depending on wether they match current search string
// ----------------------------------------------------------------------------
{
    NewDocumentWizard * wiz = (NewDocumentWizard *)wizard();
    QString searchString = search->text();
    int firstShown = -1;
    for (int idx = 0; idx < templateListWidget->count(); idx++)
    {
        QListWidgetItem *item = templateListWidget->item(idx);
        Template t = wiz->templates.at(idx);
        bool hide = !searchString.isEmpty() && !t.contains(searchString);
        item->setHidden(hide);
        if (firstShown == -1 && !hide)
            firstShown = idx;
    }

    templateListWidget->setCurrentRow(firstShown);
}


DocumentNameAndLocationPage::DocumentNameAndLocationPage(QWidget *parent)
// ----------------------------------------------------------------------------
//   Create the page where user can select the destination folder
// ----------------------------------------------------------------------------
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
    docLocationChooseButton->setDefault(false);
    docLocationChooseButton->setAutoDefault(false);

    QObject::connect(docLocationChooseButton, SIGNAL(clicked()),
                     this,  SLOT(chooseLocation()));

    registerField("docName*", docNameLineEdit);
    registerField("docLocation*", docLocationLineEdit);

    QGridLayout *layout = new QGridLayout(this);
    layout->addWidget(docNameLabel, 0, 0, Qt::AlignRight);
    layout->addWidget(docNameLineEdit, 0, 1);
    layout->addWidget(docLocationLabel, 1, 0, Qt::AlignRight);
    layout->addWidget(docLocationLineEdit, 1, 1);
    layout->addWidget(docLocationChooseButton, 1, 2);
    setLayout(layout);
}


void DocumentNameAndLocationPage::initializePage()
// ----------------------------------------------------------------------------
//   Fill the page where user can select the destination folder
// ----------------------------------------------------------------------------
{
    docLocationLineEdit->setText(Application::defaultProjectFolderPath());

    // By default, propose to name new document like template
    int idx = field("templateIdx").toInt();
    if (idx != -1)
    {
        QString name;
        NewDocumentWizard * wiz = (NewDocumentWizard *)wizard();
        Template t = wiz->templates.at(idx);
        if (t.mainFile != "")
        {
            name = t.mainFile;
            name.replace(".ddd", "");
        }
        if (name != "")
        {
            docNameLineEdit->setText(name);
            docNameLineEdit->selectAll();
        }
    }
}


void DocumentNameAndLocationPage::chooseLocation()
// ----------------------------------------------------------------------------
//   Open a folder selection dialog to select target folder
// ----------------------------------------------------------------------------
{
    QString location = QFileDialog::getExistingDirectory(this,
                           tr("Choose Document Location"),
                           Application::defaultProjectFolderPath(),
                           QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!location.isEmpty())
        docLocationLineEdit->setText(QDir::toNativeSeparators(location));
}

}

