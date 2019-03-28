#ifndef HISTORY_PLAYBACK_TOOL_H
#define HISTORY_PLAYBACK_TOOL_H
// *****************************************************************************
// history_playback_tool.h                                         Tao3D project
// *****************************************************************************
//
// File description:
//
//     This class provides a media-player like interface to navigate through
//     the document history.
//
//
//
//
//
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2010, Catherine Burvelle <catherine@taodyne.com>
// (C) 2010,2014-2015,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2011, Jérôme Forissier <jerome@taodyne.com>
// (C) 2010, Lionel Schaffhauser <lionel@taodyne.com>
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

#include "tool_window.h"
#include <QObject>
#include <QString>

QT_BEGIN_NAMESPACE
class QSlider;
QT_END_NAMESPACE

namespace Tao {

struct Repository;
class HistoryPlayback;

class HistoryPlaybackTool : public ToolWindow
// ----------------------------------------------------------------------------
//   Manage navigation through the history of the current branch
// ----------------------------------------------------------------------------
{
    Q_OBJECT

public:
    HistoryPlaybackTool(const QString &title, QWidget *parent = 0,
                        const QString &objName = "");

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
