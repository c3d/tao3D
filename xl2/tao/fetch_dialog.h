#ifndef FETCH_DIALOG_H
#define FETCH_DIALOG_H
// ****************************************************************************
//  fetch_dialog.h                                                 Tao project
// ****************************************************************************
//
//   File Description:
//
//    The class to display the "Fetch" dialog box
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

#include "remote_selection_frame.h"
#include "ui_fetch_dialog.h"
#include "repository.h"
#include <QDialog>
#include <QWidget>

namespace Tao {

struct Repository;

class FetchDialog : public QDialog, private Ui::FetchDialog
{
    Q_OBJECT

public:
    FetchDialog(Repository *repo, QWidget *parent = 0);

public:
    QString      fetchUrl();

public slots:
    virtual void accept();
    void         on_rsFrame_noneSelected();
    void         on_rsFrame_nameSelected();

private:
    Repository * repo;
};

}

#endif // FETCH_DIALOG_H
