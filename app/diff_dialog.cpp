// *****************************************************************************
// diff_dialog.cpp                                                 Tao3D project
// *****************************************************************************
//
// File description:
//
//    DiffDialog implementation.
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
// (C) 2010, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010,2014-2015,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2011, Jérôme Forissier <jerome@taodyne.com>
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

#include "tao.h"
#include "diff_dialog.h"
#include "repository.h"
#include "history_frame.h"
#include "diff_highlighter.h"

namespace Tao {

DiffDialog::DiffDialog(Repository *repo, QWidget *parent)
// ----------------------------------------------------------------------------
//    Create a "History" dialog box
// ----------------------------------------------------------------------------
    : QDialog(parent), repo(repo)
{
    XL_ASSERT(repo);

    setupUi(this);
    setupEditor();
    aFrame->setMessage(tr("Select first version (A):"));
    bFrame->setMessage(tr("Select second version (B):"));
    connect(aFrame, SIGNAL(revSet(QString)), this, SLOT(diff()));
    connect(bFrame, SIGNAL(revSet(QString)), this, SLOT(diff()));
    aFrame->setRepository(repo);
    bFrame->setRepository(repo);
    connect(symetric, SIGNAL(stateChanged(int)), this, SLOT(diff()));
}


void DiffDialog::setupEditor()
// ----------------------------------------------------------------------------
//    Select text font and attach syntax highlighter
// ----------------------------------------------------------------------------
{
    QFont font("unifont");
    font.setPixelSize(16);
    textEdit->setFont(font);
    highlighter = new DiffHighlighter(textEdit->document());
    textEdit->setReadOnly(true);
}


void DiffDialog::diff()
// ----------------------------------------------------------------------------
//    Action for checkout button
// ----------------------------------------------------------------------------
{
    QString a = aFrame->rev();
    QString b = bFrame->rev();

    if (a == "" || b == "")
        return;

    bool sym = symetric->isChecked();
    QString diff = repo->diff(a, b, sym);
    if (diff == "")
        diff = "Identical\n";
    textEdit->setPlainText(diff);

    // Syntax highlighting is mostly done in the DiffHighlighter class, except
    // for one thing: we want the background color of highlighted sections to
    // cover the whole line width. This accomplished with QTextEdit 'extra
    // selections'.

    QList<QTextEdit::ExtraSelection> extraSelections;

    QTextDocument *doc = textEdit->document();
    for (QTextBlock b = doc->begin(); b != doc->end(); b = b.next())
    {
        QTextBlockUserData *ud = b.userData();
        DiffBlockData *dbd = (DiffBlockData *)(ud);
        if (ud)
        {
            QTextEdit::ExtraSelection selection;

            selection.format.setBackground(dbd->format.background());
            selection.format.setProperty(QTextFormat::FullWidthSelection,
                                         true);
            selection.cursor = QTextCursor(b);
            selection.cursor.clearSelection();
            extraSelections.append(selection);
        }
    }

    textEdit->setExtraSelections(extraSelections);
}

}
