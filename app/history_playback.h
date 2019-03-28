#ifndef HISTORY_PLAYBACK_H
#define HISTORY_PLAYBACK_H
// *****************************************************************************
// history_playback.h                                              Tao3D project
// *****************************************************************************
//
// File description:
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
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2011, Jérôme Forissier <jerome@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Tao3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Tao3D, in a file named COPYING.
// If not, see <https://www.gnu.org/licenses/>.
// *****************************************************************************

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
    void         newCommit(QString id);

    void         setValue(int n);

signals:
    void         ready(bool);
    void         rangeChanged(int min, int max);
    void         valueChanged(int n);

private:
    void         checkout(int pos);
    void         debugPrintHistory();

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
