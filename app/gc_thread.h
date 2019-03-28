#ifndef GC_THREAD_H
#define GC_THREAD_H
// *****************************************************************************
// gc_thread.h                                                     Tao3D project
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
// (C) 2013-2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2011, Jérôme Forissier <jerome@taodyne.com>
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
