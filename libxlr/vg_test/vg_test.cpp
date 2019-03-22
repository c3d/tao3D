// *****************************************************************************
// vg_test.cpp                                                     Tao3D project
// *****************************************************************************
//
// File description:
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
// (C) 2019, Christophe de Dinechin <christophe@dinechin.org>
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
#include "tree.h"
#include <QTime>

bool noerr_test = false;
bool leak_test = false;
bool invalid_read_test = false;
bool perf_test = false;

void do_noerr_test();
void do_leak_test();
void do_invalid_read_test();
void do_perf_test();

int main(int argc, char *argv[])
{
    for (int i=1; i < argc; i++)
    {
        if (!strcmp(argv[i], "-noerr_test"))
            noerr_test = true;
        else
        if (!strcmp(argv[i], "-leak_test"))
            leak_test = true;
        else
        if (!strcmp(argv[i], "-ir_test"))
            invalid_read_test = true;
        else
        if (!strcmp(argv[i], "-perf_test"))
            perf_test = true;
    }

    if (noerr_test)
        do_noerr_test();
    if (leak_test)
        do_leak_test();
    if (invalid_read_test)
        do_invalid_read_test();
    if (perf_test)
        do_perf_test();

    return 0;
}


void gc()
// ----------------------------------------------------------------------------
//    Force garbage collection
// ----------------------------------------------------------------------------
{
    XL::GarbageCollector::Collect(true);
    XL::GarbageCollector::Collect(true);
}


void delete_gc()
// ----------------------------------------------------------------------------
//    Delete GC
// ----------------------------------------------------------------------------
{
    XL::GarbageCollector::Delete();
}


void do_noerr_test()
// ----------------------------------------------------------------------------
//    Normal usage of allocator (no error)
// ----------------------------------------------------------------------------
{
    delete new XL::Integer;
    delete_gc();
}


void do_invalid_read_test()
// ----------------------------------------------------------------------------
//    Cause an invalid read
// ----------------------------------------------------------------------------
{
    XL::Integer *p = NULL;
    delete (p = new XL::Integer);
    gc();
    int i = p->value; (void)i;
}


void do_leak_test()
// ----------------------------------------------------------------------------
//    Cause a memory leak
// ----------------------------------------------------------------------------
{
    new XL::Integer;
}


void do_perf_test()
// ----------------------------------------------------------------------------
//    Measure allocation/deallocation time
// ----------------------------------------------------------------------------
{
    QTime t;
    t.start();

#define SZ 200
    XL::Integer *p[SZ];
    for (int i = 0; i < 50000; i++)
    {
        for (int j = 0; j < SZ; j++)
            p[j] = new XL::Integer(j);
        for (int j = 0; j < SZ; j++)
            delete p[j];
    }
#undef SZ

    std::cerr << "Elapsed: " << t.elapsed() << " ms\n";
}
