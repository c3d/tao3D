// ****************************************************************************
//  module_info_dialog.cpp                                         Tao project
// ****************************************************************************
//
//   File Description:
//
//    Display a dialog box with information about a Tao module
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
//  (C) 2011 Taodyne SAS
// ****************************************************************************

#include "module_info_dialog.h"
#include "tao_utf8.h"
#include <QDesktopServices>
#include <QFile>
#include <QDir>
#include <QApplication>

namespace Tao {

ModuleInfoDialog::ModuleInfoDialog(const ModuleInfo &info, QWidget *parent)
// ----------------------------------------------------------------------------
//    Create a module info dialog box
// ----------------------------------------------------------------------------
    : QDialog(parent)
{
    adjustSize();

    textArea = new QTextBrowser;
    textArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    textArea->setOpenLinks(false);
    textArea->setOpenExternalLinks(false);
    connect(textArea, SIGNAL(anchorClicked(QUrl)), this, SLOT(openUrl(QUrl)));

    buttonBox = new QDialogButtonBox;
    buttonBox->setOrientation(Qt::Horizontal);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok);
    buttonBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));

    layout = new QVBoxLayout;
    layout->addWidget(textArea);
    layout->addWidget(buttonBox);
    setLayout(layout);


    updateInfo(info);
}


void ModuleInfoDialog::updateInfo(const ModuleInfo &info)
// ----------------------------------------------------------------------------
//    Update the dialog with the given module info
// ----------------------------------------------------------------------------
{
    ModuleInfo mi(info);

    // Pretty-print some fields in HTML
    if (mi.path != "")
    {
        QString native = QDir::toNativeSeparators(+mi.path);
        QString display = native;
        QString tao = QApplication::applicationDirPath();
        tao = QDir::toNativeSeparators(tao);
        display.replace(tao, tr("&lt;Tao&gt;"));

        QString link("<a href=\"file:///%1\">%2</a>");
        link = link.arg(native).arg(display);
        mi.path = +link;
    }
    if (mi.icon == "")
    {
        mi.icon = "qrc:/images/modules.png";
    }
    mi.website = +autoLinkUrls(+mi.website);
    mi.desc = +autoLinkUrls(+mi.desc);


    // Load HTML template
    // The file name is translated with tr() instead of using a language
    // tag in the .qrc file, because the latter depends on
    // QLocale()::system() which cannot be changed through app preferences.
    QString fname(tr(":/html/module_info_dialog.html"));
    QFile file(fname);
    QString t = tr("<b>Error</b>: Could not open: <i>%1</i>.<br>"
                   "Re-installing might fix the problem.").arg(fname);
    if (file.open(QIODevice::ReadOnly))
        t = QString::fromUtf8(file.readAll().data());

    QString vstr = QString::number(mi.ver);
    if (!vstr.contains("."))
        vstr.append(".0");

    // Substitute placeholders in HTML template
    t.replace("@ID@", +mi.id);
    t.replace("@PATH@", +mi.path);
    t.replace("@NAME@", +mi.name);
    t.replace("@DESC@", +mi.desc);
    t.replace("@ICON@", +mi.icon);
    t.replace("@VER@", vstr);
    t.replace("@AUTHOR@", +mi.author);
    t.replace("@WEBSITE@", +mi.website);
    t.replace("@IMPORT_NAME@", +mi.importName);

    // Update dialog
    textArea->setHtml(t);
}


QString ModuleInfoDialog::autoLinkUrls(QString str)
// ----------------------------------------------------------------------------
//    Turn any http://, https:// or www. into a link (a href)
// ----------------------------------------------------------------------------
{
    QRegExp rx ("\\b(?:(http)(s?)(://)|(www\\.))"
                "([-a-zA-Z0-9+&@#/%=~_|$?!:,.]*[-a-zA-Z0-9+&@#/%=~_|$])");
    str.replace(rx, "<a href=\"http\\2://\\4\\5\">\\1\\2\\3\\4\\5</a>");
    return str;
}


void ModuleInfoDialog::openUrl(QUrl url)
// ----------------------------------------------------------------------------
//    Open url in an external browser window
// ----------------------------------------------------------------------------
{
    QDesktopServices::openUrl(url);
}

}
