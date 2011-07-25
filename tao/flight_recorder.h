#ifndef FLIGHT_RECORDER_H
#define FLIGHT_RECORDER_H
// ****************************************************************************
//  flight_recorder.h                                             Tao project
// ****************************************************************************
//
//   File Description:
//
//     Record information about what's going on in the application
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
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "base.h"
#include <vector>
#include <iostream>

namespace Tao {

struct FlightRecorder
// ----------------------------------------------------------------------------
//    Record events 
// ----------------------------------------------------------------------------
{
    struct Entry
    {
        Entry(kstring what = "", void *caller = NULL,
              kstring l1="", intptr_t a1=0,
              kstring l2="", intptr_t a2=0,
              kstring l3="", intptr_t a3=0):
            what(what), caller(caller),
            label1(l1), label2(l2), label3(l3),
            arg1(a1), arg2(a2), arg3(a3) {}
        kstring  what;
        void *   caller;
        kstring  label1, label2, label3;
        intptr_t arg1, arg2, arg3;
    };

    FlightRecorder(uint size=4096) : windex(0), rindex(0), records(size) {}

    void Record(kstring what, void *caller = NULL,
                kstring l1="", intptr_t a1=0,
                kstring l2="", intptr_t a2=0,
                kstring l3="", intptr_t a3=0)
    {
        Entry &e = records[windex++ % records.size()];
        e.what = what;
        e.caller = caller;
        e.label1 = l1;
        e.arg1 = a1;
        e.label2 = l2;
        e.arg2 = a2;
        e.label3 = l3;
        e.arg3 = a3;
    }

    void Dump(std::ostream &out);
    static void DumpToLogFile();

public:
    uint        windex, rindex;
    std::vector<Entry>  records;

    static FlightRecorder *     recorder;
};


#define RECORD(what, args...)                                           \
   (Tao::FlightRecorder::recorder->Record(what,                         \
                                          __builtin_return_address(0),  \
                                          ##args))
#define RECORDER_DUMP(what)                     \
    do                                          \
    {                                           \
        RECORD(what);                           \
        Tao::FlightRecorder::DumpToLogFile();   \
    } while(0)

}

#endif // FLIGHT_RECORDER_H
