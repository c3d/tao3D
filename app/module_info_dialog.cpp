// *****************************************************************************
// module_info_dialog.cpp                                          Tao3D project
// *****************************************************************************
//
// File description:
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
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2013-2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2011, Jérôme Forissier <jerome@taodyne.com>
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

#include "module_info_dialog.h"
#include "tao_utf8.h"
#include <QDesktopServices>
#include <QFile>
#include <QDir>
#include <QApplication>
#include <QTextDocument>

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
#if QT_VERSION >= 0x050000
    mi.desc = +autoLinkUrls((+mi.desc).toHtmlEscaped());
#else
    mi.desc = +autoLinkUrls(Qt::escape(+mi.desc));
#endif


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
