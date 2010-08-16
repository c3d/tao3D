#ifndef HISTORY_DIALOG_H
#define HISTORY_DIALOG_H
// ****************************************************************************
//  history_dialog.h                                               Tao project
// ****************************************************************************
//
//   File Description:
//
//    The class to display the "History" dialog box, which is used to select
//    a commit. Contains a branch selection combo box, a commit selection combo
//    box and a free text edit area.
//    Used as a base class for actions like "revert to" and "selective undo".
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

#include "ui_history_dialog.h"
#include "repository.h"
#include <QDialog>
#include <QWidget>
#include <QPushButton>

namespace Tao {

struct Repository;

class HistoryDialog : public QDialog, protected Ui::HistoryDialog
{
    Q_OBJECT

public:
    HistoryDialog(Repository *repo, QWidget *parent = 0);

private slots:
    void    revCombo_commitSelected(Repository::Commit commit);

private:
    void    setRepository(Repository *repo);

protected:
    Repository  *repo;
};

}

#endif // HISTORY_DIALOG_H
