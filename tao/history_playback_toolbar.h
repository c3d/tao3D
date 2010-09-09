#ifndef HISTORY_PLAYBACK_TOOLBAR_H
#define HISTORY_PLAYBACK_TOOLBAR_H
// ****************************************************************************
//  history_playback_toolbar.h                                     Tao project
// ****************************************************************************
//
//   File Description:
//
//     This class provides a media-player like interface to navigate through
//     the document history.
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

#include <QToolBar>

QT_BEGIN_NAMESPACE
class QSlider;
QT_END_NAMESPACE

namespace Tao {

class Repository;
class HistoryPlayback;

class HistoryPlaybackToolBar : public QToolBar
// ----------------------------------------------------------------------------
//   Manage navigation through the history of the current branch
// ----------------------------------------------------------------------------
{
    Q_OBJECT

public:
    HistoryPlaybackToolBar(const QString & title, QWidget *parent = 0);

public slots:
    void              setRepository(Repository *);
    void              refresh();

signals:
    void              documentChanged();

private slots:
    void              setSliderRange(int min, int max);

private:
    HistoryPlayback * playback;
    QSlider         * slider;
};

}

#endif
