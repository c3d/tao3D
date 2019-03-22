// *****************************************************************************
// gc_thread.cpp                                                   Tao3D project
// *****************************************************************************
//
// File description:
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
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2012, Catherine Burvelle <catherine@taodyne.com>
// (C) 2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2011, Jérôme Forissier <jerome@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can r redistribute it and/or modify
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


#include "base.h"
#include "gc_thread.h"
#include "gc.h"
#include "widget.h"
#include <QEvent>
#include <QTime>
#include <iostream>

namespace Tao {

void GCThread::collect()
// ----------------------------------------------------------------------------
//   Run garbage collection
// ----------------------------------------------------------------------------
{
    Widget *w = (Widget *)sender();

    IFTRACE(memory)
        debug() << "GC collect started\n";
    QTime t;
    t.start();
    w->stats.begin(Statistics::GC);
    XL::GarbageCollector::Collect();
    w->stats.end(Statistics::GC);
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
