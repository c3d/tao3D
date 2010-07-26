#ifndef COMMIT_SELECTION_COMBOBOX_H
#define COMMIT_SELECTION_COMBOBOX_H

// ****************************************************************************
//  commit_selection_combobox.h                                    Tao project
// ****************************************************************************
//
//   File Description:
//
//    A class to display a combobox that shows the commit log for a branch
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

#include "repository.h"
#include <QComboBox>
#include <QWidget>
#include <QString>

namespace Tao {

class CommitSelectionComboBox : public QComboBox
{
    Q_OBJECT

public:
    enum Mode
    // ------------------------------------------------------------------------
    //   Display options: select what to show in the combo box
    // ------------------------------------------------------------------------
    {
        CSM_HeadEntry  = 1,  // Show "HEAD" entry
        CSM_CommitId   = 2,  // Show commit identifier
        CSM_CommitMsg  = 4,  // Show commit message
        CSM_Default    = (CSM_HeadEntry | CSM_CommitId | CSM_CommitMsg),
    };

public:
    CommitSelectionComboBox(QWidget *parent = 0);

public:
    Repository::Commit currentCommit();
    void               refresh();
    void               setMode(unsigned int mode);

public slots:
    void    setRepository(Repository *repo);
    void    setBranch(QString branch);

signals:
    void    commitSelected(Repository::Commit commit);

protected slots:
    void    on_activated(int selected);

private:
    bool    populate();

private:
    Repository * repo;
    QString      branch;
    int          prevSelected;
    unsigned int mode;
};

}

#endif // COMMIT_SELECTION_COMBOBOX_H
