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
