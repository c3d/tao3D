// ****************************************************************************
//  gc_thread.cpp                                                  Tao project
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
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 2011 Jerome Forissier <jerome@taodyne.com>
//  (C) 2011 Taodyne SAS
// ****************************************************************************


#include "base.h"
#include "gc_thread.h"
#include "gc.h"
#include <QEvent>
#include <QTime>
#include <iostream>

namespace Tao {

void GCThread::collect()
// ----------------------------------------------------------------------------
//   Run garbage collection
// ----------------------------------------------------------------------------
{
    QTime t;
    t.start();
    XL::GarbageCollector::Collect();
    IFTRACE(memory)
        debug() << "Collect time " << t.elapsed() << " ms\n";
    mutex.lock();
    collectDone = true;
    cond.wakeOne();
    mutex.unlock();
}


void GCThread::clearCollectDone()
// ----------------------------------------------------------------------------
//   Mark that collect cycle has not started
// ----------------------------------------------------------------------------
{
    mutex.lock();
    collectDone = false;
    mutex.unlock();
}


void GCThread::waitCollectDone()
// ----------------------------------------------------------------------------
//   Wait until done with collect
// ----------------------------------------------------------------------------
{
    mutex.lock();
    while (!collectDone)
        cond.wait(&mutex);
    mutex.unlock();
}


std::ostream & GCThread::debug()
// ----------------------------------------------------------------------------
//   Convenience method to log with a common prefix
// ----------------------------------------------------------------------------
{
    std::cerr << "[GCThread] ";
    return std::cerr;
}

}
