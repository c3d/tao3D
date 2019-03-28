#ifndef PREVIEW_H
#define PREVIEW_H
// *****************************************************************************
// preview.h                                                       Tao3D project
// *****************************************************************************
//
// File description:
//
//     Thread saving previews of the current output widget
//
//
//
//
//
//
//
//
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2014,2019, Christophe de Dinechin <christophe@dinechin.org>
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

#include "tao.h"
#include <QThread>
#include <QMutex>
#include <QTimer>
#include <QImage>
#include <iostream>


namespace Tao {

class PreviewThread : public QThread
// ----------------------------------------------------------------------------
//   A thread that periodically saves previews of the current GL widget
// ----------------------------------------------------------------------------
{
    Q_OBJECT

public:
    PreviewThread(uint timeInterval = 100,
                  uint maxWidth = ~0U, uint maxHeight = ~0U);
    ~PreviewThread();

public:
    void        setPath(QString path);
    void        setMaxSize(uint maxWidth, uint maxHeight);
    void        setInterval(uint interval);
    void        recordImage(QImage &image);
    bool        isBusy();
    bool        isReady();

signals:
    void        imageAvailable();

protected slots:
    void        savePreview();

protected:
    std::ostream &debug();

private:
    QMutex      mutex;
    QString     path;
    QImage      image;
    QTimer      nextSave;
    uint        timeInterval;
    uint        maxWidth, maxHeight;
};

}

#endif // PREVIEW_H
