#ifndef PREFERENCES_PAGES_H
#define PREFERENCES_PAGES_H
// ****************************************************************************
//  preferences_pages.h                                            Tao project
// ****************************************************************************
//
//   File Description:
//
//     Various pages for the Preferences Dialog.
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

#include <QWidget>

namespace Tao {

class DebugPage : public QWidget
// ----------------------------------------------------------------------------
//   Page to show and modify the state of debug traces
// ----------------------------------------------------------------------------
{
    Q_OBJECT

public:
    DebugPage(QWidget *parent = 0);

private:
    QStringList  allTraceNames();
    bool         isTraceEnabled(QString tname);
    void         toggleAllTraces(bool on);

private slots:
    void         toggleTrace(bool on);
    void         enableAllTraces();
    void         disableAllTraces();
};

}

#endif
