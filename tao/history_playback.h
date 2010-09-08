#ifndef HISTORY_PLAYBACK_H
#define HISTORY_PLAYBACK_H
// ****************************************************************************
//  history_playback.h                                             Tao project
// ****************************************************************************
//
//   File Description:
//
//     Allows to navigate through the document history, step by step, along
//     the current branch. The document is put in read-only mode when the
//     current version is not the head of the branch.
//     The GUI part is in history_playback_toolbar.h.
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

#include "repository.h"
#include <QObject>

namespace Tao {

class HistoryPlayback : public QObject
// ----------------------------------------------------------------------------
//   Manage navigation through the history of the current branch
// ----------------------------------------------------------------------------
{
    Q_OBJECT

public:
    HistoryPlayback(QObject *parent);
    HistoryPlayback(QObject *parent, Repository *repo);

public:
    int          min()  { return -span + 1; }
    int          max()  { return 0; }

public slots:
    void         back();
    void         forward();
    void         begin();
    void         end();

    void         setRepository(Repository *);
    void         reset();
    void         newCommit();

    void         setValue(int n);

signals:
    void         ready(bool);
    void         rangeChanged(int min, int max);
    void         valueChanged(int n);

private:
    void         checkout(int pos);

private:
    QList<Repository::Commit> history;
    Repository * repo;
    QString      head;  // name of the branch
    int          pos;   // Position relative to current HEAD (which is pos 0)
    int          span;  // Number of commits covered by this playback object
    int          max_span; // How far in the past we want to go
};

}

#endif
