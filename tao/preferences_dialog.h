#ifndef PREFERENCES_DIALOG_H
#define PREFERENCES_DIALOG_H
// ****************************************************************************
//  preferences_dialog.h                                           Tao project
// ****************************************************************************
//
//   File Description:
//
//     The dialog box to control application preferences.
//     Largely inspired from the Qt "Configuration Dialog" example.
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
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include <QDialog>

class QListWidget;
class QListWidgetItem;
class QStackedWidget;

namespace Tao {

class PreferencesDialog : public QDialog
{
    Q_OBJECT

public:
    PreferencesDialog(QWidget *parent);

public slots:
    void changePage(QListWidgetItem *current, QListWidgetItem *previous);

private:
    void createIcons();

    QListWidget *contentsWidget;
    QStackedWidget *pagesWidget;
};

}

#endif
