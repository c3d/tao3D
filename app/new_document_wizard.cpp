// *****************************************************************************
// new_document_wizard.cpp                                         Tao3D project
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
// (C) 2013-2015,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2011-2013, Jérôme Forissier <jerome@taodyne.com>
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

#include <QtGui>

#include "new_document_wizard.h"
#include "application.h"
#include "window.h"
#include "repository.h"
#include "texture_cache.h"

#include <QPushButton>
#include <QMessageBox>
#include <QLineEdit>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QListWidget>
#include <QLabel>
#include <QFileDialog>

namespace Tao {


static inline QString exampleThumbnail(QString name)
// ----------------------------------------------------------------------------
//   Return the URL for a thumbnail picture
// ----------------------------------------------------------------------------
{
    return "http://www.taodyne.com/tao-examples-thumbnails/" + name + ".png";
}


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
    XL_ASSERT(next);
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
    QString url(tr("http://tao3d.sourceforge.net#samples"));
    QDesktopServices::openUrl(url);
    reject();
}


static void Rename(QDir &dir, QString oldName, QString newName, QString ext)
// ----------------------------------------------------------------------------
//   Rename the file with the given extension
// ---------------------------------------------------------------------------
{
    newName += ext;
    oldName += ext;
    dir.remove(newName);
    dir.rename(oldName, newName);
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
                       "Do you want to use it anyway (current content "
                       "will be deleted)?\n"
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
        QString oldName = t.mainFile.replace(QRegExp("\\.ddd$"), "");
        QString newName = docName;
        if (oldName != newName)
        {
            // Rename template main file to doc name.
            // We need to remove the destination file if it is there
            QDir dstDir = QDir(dstPath);
            Rename(dstDir, oldName, newName, ".ddd");
            Rename(dstDir, oldName, newName, ".ddd.sig");
            Rename(dstDir, oldName, newName, ".json");
            docPath = dstDir.filePath(newName + ".ddd");
        }
    }

#if !defined(CFG_NOGIT)
    // Create project to avoid prompt when document is first opened
    RepositoryFactory::repository(dstPath, RepositoryFactory::Create);
#endif
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
    connect(search, SIGNAL(textChanged(QString)),
            this,   SLOT(filterItems()));

    showAll = new QCheckBox(tr("Show all examples"));
    connect(showAll, SIGNAL(toggled(bool)),
            this,    SLOT(filterItems()));
    showAll->setChecked(false);

    QHBoxLayout *searchLayout = new QHBoxLayout;
    searchLayout->addWidget(showAll, 2);
    searchLayout->addStretch(1);
    searchLayout->addWidget(search, 2);


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
    QDir appPath(TaoApp->applicationDirPath() + "/templates");
    QDir userPath(TaoApp->defaultTaoPreferencesFolderPath() + "/templates");
    QList<QDir> dirs;
    dirs << appPath << userPath;
    wiz->templates = Templates(dirs);

    // Insert entries from the settings (remote menus)
    QSharedPointer<TextureCache> tc = TextureCache::instance();
    QString docPath = TaoApp->window()->currentProjectFolderPath();
    QSettings settings;
    int size = settings.beginReadArray("examples");
    for (int i = 0; i < size; i++)
    {
        settings.setArrayIndex(i);
        QString caption = settings.value("caption").toString();
        QString url = settings.value("url").toString();
        QString tip = settings.value("description").toString();

        Template tmpl(userPath);
        tmpl.name = caption;
        tmpl.description = tip;
        tmpl.type = "url";
        tmpl.mainFile = url;

        QString thumbPath = exampleThumbnail(caption);

        CachedTexture *tex = tc->load(thumbPath, docPath);
        tmpl.thumbnail = QPixmap::fromImage(tex->loadedImage());
        if (tmpl.thumbnail.isNull())
        {
            connect(tex,    SIGNAL(textureUpdated(CachedTexture *)),
                    this,   SLOT(thumbnailChanged(CachedTexture *)));
        }

        wiz->templates.append(tmpl);
    }
    settings.endArray();

    foreach (Template tmpl, wiz->templates)
    {
        QListWidgetItem *t = new QListWidgetItem(templateListWidget);
        QPixmap        pm   = tmpl.thumbnail;
        if (!pm.isNull())
            t->setIcon(QIcon(pm));
        t->setText(tmpl.name.mid(tmpl.name.lastIndexOf('/')+1));
        t->setTextAlignment(Qt::AlignHCenter);
        t->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

        if(tmpl.type != "theme")
            t->setHidden(true);
    }

    templateListWidget->setCurrentRow(0);
}


void TemplateChooserPage::thumbnailChanged(CachedTexture *)
// ----------------------------------------------------------------------------
//    Update the thumbnail once loaded from the network
// ----------------------------------------------------------------------------
{
    NewDocumentWizard * wiz = (NewDocumentWizard *)wizard();
    QString docPath = TaoApp->window()->currentProjectFolderPath();
    QSharedPointer<TextureCache> tc = TextureCache::instance();
    int idx = 0;

    foreach (Template tmpl, wiz->templates)
    {
        QListWidgetItem *item = templateListWidget->item(idx++);
        QPixmap          pm = tmpl.thumbnail;
        if (pm.isNull())
        {
            QString thumbPath = exampleThumbnail(tmpl.name);
            CachedTexture *  tex  = tc->load(thumbPath, docPath);
            QPixmap          pm   = QPixmap::fromImage(tex->loadedImage());
            if (!pm.isNull())
                item->setIcon(QIcon(pm));
        }
    }
}


void TemplateChooserPage::updateDescription()
// ----------------------------------------------------------------------------
//   Set the description text for the currently selected item
// ----------------------------------------------------------------------------
{
    QString desc;
    NewDocumentWizard * wiz = (NewDocumentWizard *) wizard();
    int idx = field("templateIdx").toInt();

    if (idx != -1)
    {
        Template t = wiz->templates.at(idx);
        desc = t.description;
        description->setText(desc);
    }
}


void TemplateChooserPage::filterItems()
// ----------------------------------------------------------------------------
//   Show or hide items, depending on wether they match current search string
// ----------------------------------------------------------------------------
{
    NewDocumentWizard * wiz = (NewDocumentWizard *) wizard();
    QString searchString = search->text();
    int firstShown = -1;
    bool themeOnly = !showAll->isChecked();
    bool searching = !searchString.isEmpty();

    for (int idx = 0; idx < templateListWidget->count(); idx++)
    {
        QListWidgetItem *item = templateListWidget->item(idx);
        bool hide = false;
        Template t = wiz->templates.at(idx);
        hide = ((searching && !t.contains(searchString)) ||
                (themeOnly && t.type != "theme"));
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
        NewDocumentWizard * wiz = (NewDocumentWizard *)wizard();
        QString name;
        Template t = wiz->templates.at(idx);
        if (t.mainFile != "")
        {
            name = t.mainFile;
            name.replace(QRegExp("\\.ddd$"), "");
            name.replace(QRegExp("^[A-Za-z]+:/*"), "");
        }
        if (name != "")
        {
            docNameLineEdit->setText(name);
            docNameLineEdit->selectAll();
        }

        if (t.type == "url")
        {
            TaoApp->window()->open(t.mainFile);
            wiz->reject();
        }
    }
}


void DocumentNameAndLocationPage::chooseLocation()
// ----------------------------------------------------------------------------
//   Open a folder selection dialog to select target folder
// ----------------------------------------------------------------------------
{
    QString location = QFileDialog::getExistingDirectory
        (this,
         tr("Choose Document Location"),
         Application::defaultProjectFolderPath(),
         QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    if (!location.isEmpty())
        docLocationLineEdit->setText(QDir::toNativeSeparators(location));
}

}

