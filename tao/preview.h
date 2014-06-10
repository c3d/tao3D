#ifndef PREVIEW_H
#define PREVIEW_H
// ****************************************************************************
//  preview.h                                                      Tao project
// ****************************************************************************
//
//   File Description:
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
// ****************************************************************************
//  (C) 2014 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2014 Jérôme Forissier <jerome@taodyne.com>
//  (C) 2014 Catherine Burvelle <cathy@taodyne.com>
//  (C) 2014 Taodyne SAS
// ****************************************************************************

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
    void        record(QImage &image);
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
