#ifndef HISTORY_PLAYBACK_TOOL_H
#define HISTORY_PLAYBACK_TOOL_H
// ****************************************************************************
//  history_playback_tool.h                                        Tao project
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
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "tool_window.h"
#include <QObject>
#include <QString>

QT_BEGIN_NAMESPACE
class QSlider;
QT_END_NAMESPACE

namespace Tao {

class Repository;
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
