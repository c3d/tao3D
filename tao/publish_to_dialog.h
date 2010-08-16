#ifndef PUBLISH_TO_DIALOG_H
#define PUBLISH_TO_DIALOG_H
// ****************************************************************************
//  publish_to_dialog.h                                            Tao project
// ****************************************************************************
//
//   File Description:
//
//    The class to display the "Publish to" dialog box
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
#include "ui_publish_to_dialog.h"
#include "repository.h"
#include <QDialog>
#include <QWidget>

namespace Tao {

struct Repository;

class PublishToDialog : public QDialog, private Ui::PublishToDialog
{
    Q_OBJECT

public:
    PublishToDialog(Repository *repo, QWidget *parent = 0);

public:
    QString      pushTo();

public slots:
    virtual void accept();
    void         on_rsFrame_noneSelected();
    void         on_rsFrame_nameSelected();

private:
    Repository * repo;
};

}

#endif // PUBLISH_TO_DIALOG_H
