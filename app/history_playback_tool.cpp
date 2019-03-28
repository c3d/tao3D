// *****************************************************************************
// history_playback_tool.cpp                                       Tao3D project
// *****************************************************************************
//
// File description:
//
//     HistoryPlaybackTool implementation.
//
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

#include "history_playback_tool.h"
#include "history_playback.h"
#include <QLabel>
#include <QSlider>
#include <QIcon>
#include <QToolButton>
#include <QFrame>
#include <QHBoxLayout>
#include <QStyle>

namespace Tao {

HistoryPlaybackTool::HistoryPlaybackTool(const QString &title,
                                         QWidget *parent,
                                         const QString &objName)
// ----------------------------------------------------------------------------
//    Create a history playback tool window
// ----------------------------------------------------------------------------
    : ToolWindow(title, parent, objName)
{
    QHBoxLayout * layout = new QHBoxLayout();

    // Button icons are from the Glaze 0.4.6 icon set
    // http://www.notmart.org/index.php/Graphics
    // License: LGPL
    QToolButton * begin = new QToolButton;
    begin->setIcon(QIcon(":/images/start.png"));
    layout->addWidget(begin);

    QToolButton * back = new QToolButton;
    back->setIcon(QIcon(":/images/previous.png"));
    layout->addWidget(back);

    QToolButton * forward = new QToolButton;
    forward->setIcon(QIcon(":/images/next.png"));
    layout->addWidget(forward);

    QToolButton * end = new QToolButton;
    end->setIcon(QIcon(":/images/finish.png"));
    layout->addWidget(end);

    slider = new QSlider(Qt::Horizontal);
    slider->setTracking(false);
    layout->addWidget(slider);

    setLayout(layout);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    setMinimumWidth(250);
    setEnabled(false);

    playback = new HistoryPlayback(this);

    connect(begin,    SIGNAL(clicked()),
            playback, SLOT  (begin()));

    connect(back,     SIGNAL(clicked()),
            playback, SLOT  (back()));

    connect(forward,  SIGNAL(clicked()),
            playback, SLOT  (forward()));

    connect(end,      SIGNAL(clicked()),
            playback, SLOT  (end()));

    connect(playback, SIGNAL(ready(bool)),
            this,     SLOT  (setEnabled(bool)));
    connect(playback, SIGNAL(rangeChanged(int,int)),
            this,     SLOT  (setSliderRange(int,int)));
    connect(slider,   SIGNAL(valueChanged(int)),
            playback, SLOT  (setValue(int)));
    connect(playback, SIGNAL(valueChanged(int)),
            slider,   SLOT  (setValue(int)));

    connect(playback, SIGNAL(valueChanged(int)),
            this,     SIGNAL(documentChanged()));
}


void HistoryPlaybackTool::setRepository(Repository *repo)
// ----------------------------------------------------------------------------
//    Select a repository and update the UI accordingly
// ----------------------------------------------------------------------------
{
    playback->setRepository(repo);
}


void HistoryPlaybackTool::refresh()
// ----------------------------------------------------------------------------
//    Update the UI when current branch was changed externally
// ----------------------------------------------------------------------------
{
    playback->reset();
}


void HistoryPlaybackTool::setSliderRange(int min, int max)
// ----------------------------------------------------------------------------
//    Update min and max values of slider
// ----------------------------------------------------------------------------
{
    slider->setRange(min, max);
}

}
