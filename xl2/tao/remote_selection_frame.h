#ifndef REMOTE_SELECTION_FRAME_H
#define REMOTE_SELECTION_FRAME_H

// ****************************************************************************
//  remote_selection_frame.cpp                                     Tao project
// ****************************************************************************
//
//   File Description:
//
//    A class to display a frame that allows users to select a remote
//    repository. This frame can be used ina dialog box.
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

#include "ui_remote_selection_frame.h"
#include <QFrame>
#include <QWidget>
#include <QString>

namespace Tao {

struct Repository;

class RemoteSelectionFrame : public QFrame, private Ui::RemoteSelectionFrame
{
    Q_OBJECT

public:
    RemoteSelectionFrame(QWidget *parent = 0);

public:
    void    setRepository(Repository *repo, QString preferredRemote = "");
    QString remote();

signals:
    void    noneSelected();
    void    nameSelected();

private slots:
    void    on_nameCombo_activated(QString selected);
    void    on_urlEdit_editingFinished();

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
    Repository * repo;
    QString      prevSelected;
};

}

#endif // REMOTE_SELECTION_FRAME_H
