// *****************************************************************************
// preview.cpp                                                     Tao3D project
// *****************************************************************************
//
// File description:
//
//
//     Thread saving previews of the current output widget
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

#include <QFileInfo>
#include <QDir>
#include "preview.h"
#include "base.h"
#include "tao_utf8.h"

namespace Tao {

PreviewThread::PreviewThread(uint timeInterval, uint maxWidth, uint maxHeight)
// ----------------------------------------------------------------------------
//   Start preview-save thread by saving
// ----------------------------------------------------------------------------
    : mutex(), path(), image(), nextSave(),
      timeInterval(timeInterval), maxWidth(maxWidth), maxHeight(maxHeight)
{
    IFTRACE(preview)
        debug() << "Starting preview thread (" << timeInterval << " ms)\n";

    // Ready the thread to receive events
    moveToThread(this);
    connect(this, SIGNAL(imageAvailable()), this, SLOT(savePreview()));

    // Configure the timer
    nextSave.setSingleShot(true);
    nextSave.setInterval(timeInterval);
    connect(&nextSave, SIGNAL(timeout()), this, SLOT(savePreview()));

    start();                    // Wait until a preview is posted to save

}


PreviewThread::~PreviewThread()
// ----------------------------------------------------------------------------
//   Stop the monitor thread
// ----------------------------------------------------------------------------
{
    IFTRACE(preview)
        debug() << "Stopping\n";
    quit();
    wait(100);
}


void PreviewThread::setPath(QString path)
// ----------------------------------------------------------------------------
//    Record the path where we will save the picture and trigger save
// ----------------------------------------------------------------------------
{
    QMutexLocker locker(&mutex);
    this->path = path;
    if (!image.isNull() && !isBusy())
        emit imageAvailable();
}


void PreviewThread::setMaxSize(uint maxWidth, uint maxHeight)
// ----------------------------------------------------------------------------
//   Record the maximum size for resizing the picture
// ----------------------------------------------------------------------------
{
    QMutexLocker locker(&mutex);
    this->maxWidth = maxWidth;
    this->maxHeight = maxHeight;
}


void PreviewThread::setInterval(uint interval)
// ----------------------------------------------------------------------------
//   Record the interval between saves of the picture
// ----------------------------------------------------------------------------
{
    QMutexLocker locker(&mutex);
    this->timeInterval = interval;
    this->nextSave.setInterval(interval);
}


void PreviewThread::recordImage(QImage &image)
// ----------------------------------------------------------------------------
//   Store the image to record next
// ----------------------------------------------------------------------------
{
    if (mutex.tryLock())
    {
        this->image = image;
        mutex.unlock();
        if (path != "" && !isBusy())
            emit imageAvailable();
    }
}


void PreviewThread::savePreview()
// ----------------------------------------------------------------------------
//   Save a preview of the image, and wait until awoken again
// ----------------------------------------------------------------------------
{
    // Acquire input parameters
    mutex.lock();
    QImage toSave = image;
    QString filePath = path;
    uint width = maxWidth;
    uint height = maxHeight;
    image = QImage();
    mutex.unlock();

    // Check if there is anything interesting to save
    if (!toSave.isNull() && filePath != "")
    {
        // Arm singles shot time for next save
        nextSave.start(timeInterval);

        // Rescale image to maximum dimensions
        if ((uint) toSave.width() > maxWidth)
            toSave = toSave.scaledToWidth(width);
        if ((uint) toSave.height() > maxHeight)
            toSave = toSave.scaledToHeight(height);

        // Save image in a temporary file
        QFileInfo fileInfo (filePath);
        QFileInfo tmpInfo(fileInfo.dir(), "tmp-" + fileInfo.fileName());
        toSave.save(tmpInfo.filePath());

        // Rename file to target (automic or near-atomic operation)
        QDir dir = fileInfo.dir();
        dir.remove(fileInfo.fileName());
        dir.rename(tmpInfo.fileName(), fileInfo.fileName());
    }
}


bool PreviewThread::isBusy()
// ----------------------------------------------------------------------------
//   Return true if we are currently unwilling to save a new preview
// ----------------------------------------------------------------------------
{
    return nextSave.isActive();
}


bool PreviewThread::isReady()
// ----------------------------------------------------------------------------
//   Return true if we are ready to save previews
// ----------------------------------------------------------------------------
{
    return path != "" && !nextSave.isActive();
}


std::ostream &PreviewThread::debug()
// ----------------------------------------------------------------------------
//   Convenience method to log with a common prefix
// ----------------------------------------------------------------------------
{
    std::cerr << "[PreviewThread] ";
    return std::cerr;
}


}
