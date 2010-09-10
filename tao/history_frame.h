#ifndef HISTORY_FRAME_H
#define HISTORY_FRAME_H
// ****************************************************************************
//  history_frame.h                                               Tao project
// ****************************************************************************
//
//   File Description:
//
//    The class to display the "History" frame box, which is used to select
//    a commit. Contains a branch selection combo box, a commit selection combo
//    box and a free text edit area.
//    Used by HistoryDialog (1) and DiffDialog (2).
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

#include "ui_history_frame.h"
#include "repository.h"
#include <QFrame>
#include <QWidget>
#include <QPushButton>

namespace Tao {

struct Repository;

class HistoryFrame : public QFrame, protected Ui::HistoryFrame
{
    Q_OBJECT

public:
    HistoryFrame(QWidget *parent = 0);
    void    setMessage(QString text);
    void    setRepository(Repository *repo);
    QString rev();

private slots:
    void    tableView_commitSelected(Repository::Commit commit);

protected:
    Repository  *repo;
};

}

#endif // HISTORY_FRAME_H
