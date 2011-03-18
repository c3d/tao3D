// ****************************************************************************
//  history_playback_tool.cpp                                      Tao project
// ****************************************************************************
//
//   File Description:
//
//     HistoryPlaybackTool implementation.
//
//
//
//
//
//
// ****************************************************************************
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

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
