#ifndef PULL_FROM_DIALOG_H
#define PULL_FROM_DIALOG_H
// ****************************************************************************
//  pull_from_dialog.h                                             Tao project
// ****************************************************************************
//
//   File Description:
//
//    The class to display the "Pull from" dialog box
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

#include "ui_pull_from_dialog.h"
#include "repository.h"
#include <QDialog>
#include <QWidget>

namespace Tao {

struct Repository;

class PullFromDialog : public QDialog, private Ui::PullFromDialog
{
    Q_OBJECT

public:
    PullFromDialog(Repository *repo, QWidget *parent = 0);

public:
    QString                        pullFrom();
    Repository::ConflictResolution conflictResolution();

public slots:
    virtual void accept();

private slots:
    void on_nameCombo_activated(QString selected);
    void on_urlEdit_editingFinished();

private:
    bool    populateNameCombo();
    bool    populateNameComboAndSelect(QString sel);
    QString addNewRemote();
    QString renameRemote();

private:
    enum ComboItemKind
    // ------------------------------------------------------------------------
    //    Values to identify each entry of the name combo box
    // ------------------------------------------------------------------------
    {
        CIK_None,   // The "<None>" item
        CIK_Name,   // Any valid remote name
        CIK_AddNew, // The "Add new..." item
        CIK_Delete, // The "Delete <name>" item
        CIK_Rename, // The "Rename <name>" item
    };

private:
    Repository *repo;
    QString     prevSelected;
};

}

#endif // PULL_FROM_DIALOG_H
