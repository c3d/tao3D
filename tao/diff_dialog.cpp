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
//#include "ui_diff_dialog.h"
#include "history_frame.h"
#include "tao_utf8.h"  // debug

namespace Tao {

DiffDialog::DiffDialog(Repository *repo, QWidget *parent)
// ----------------------------------------------------------------------------
//    Create a "History" dialog box
// ----------------------------------------------------------------------------
    : QDialog(parent), repo(repo)
{
    Q_ASSERT(repo);

    setupUi(this);
    aFrame->setMessage(tr("Select first version (A):"));
    bFrame->setMessage(tr("Select second version (B):"));
    connect(aFrame, SIGNAL(revSet(QString)), this, SLOT(diff()));
    connect(bFrame, SIGNAL(revSet(QString)), this, SLOT(diff()));
    aFrame->setRepository(repo);
    bFrame->setRepository(repo);
    connect(symetric, SIGNAL(stateChanged(int)), this, SLOT(diff()));
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
    textBrowser->setText(diff);
}

}
