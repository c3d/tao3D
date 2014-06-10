// ****************************************************************************
//  preview.cpp                                                    Tao project
// ****************************************************************************
//
//   File Description:
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
// ****************************************************************************
//  (C) 2014 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2014 Jérôme Forissier <jerome@taodyne.com>
//  (C) 2014 Catherine Burvelle <cathy@taodyne.com>
//  (C) 2014 Taodyne SAS
// ****************************************************************************

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
    moveToThread(this);
    start();                    // Wait until a preview is posted to save
    connect(this, SIGNAL(imageAvailable()), this, SLOT(savePreview()));
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
}


void PreviewThread::record(QImage &image)
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
        QTimer::singleShot(timeInterval, this, SLOT(savePreview()));

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
