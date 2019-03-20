#ifndef GC_THREAD_H
#define GC_THREAD_H
// ****************************************************************************
//  gc_thread.h                                                    Tao project
// ****************************************************************************
//
//   File Description:
//
//    Run XL garbage collection in a dedicated thread or in the main thread
//
//
//
//
//
//
//
//
// ****************************************************************************
// This software is licensed under the GNU General Public License v3.
// See file COPYING for details.
//  (C) 2011 Jerome Forissier <jerome@taodyne.com>
//  (C)s 2011 Taodyne SAS
// ****************************************************************************


#include <QThread>
#include <QWaitCondition>
#include <QMutex>

namespace Tao {

struct GCThread : public QThread
// ----------------------------------------------------------------------------
//   Run XL garbage collection in a dedicated thread
// ----------------------------------------------------------------------------
{
    Q_OBJECT

public:
    GCThread() : QThread(NULL), collectDone(false) {}
    ~GCThread() {}

    // Synchronisation with another thread
    void                     clearCollectDone();
    void                     waitCollectDone();

public slots:
    void                     collect();

protected:
    static std::ostream &    debug();

protected:
    bool                     collectDone;
    QWaitCondition           cond;
    QMutex                   mutex;
};

}

#endif // GC_THREAD_H
