// ****************************************************************************
//  diff_dialog.cpp                                                Tao project
// ****************************************************************************
//
//   File Description:
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
// ****************************************************************************
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

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
    Q_ASSERT(repo);

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
