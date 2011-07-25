// ****************************************************************************
//  flight_recorder.cpp                                           Tao project
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

#include "flight_recorder.h"
#include "tao_utf8.h"
#include "application.h"
#include <fstream>
#include <iomanip>
#include <QDir>

namespace Tao {

void FlightRecorder::Dump(std::ostream &out)
// ----------------------------------------------------------------------------
//   Dump the contents of the flight recorder to given stream
// ----------------------------------------------------------------------------
{
    using namespace std;

    if (rindex + records.size() <= windex)
        rindex = windex - records.size() + 1;

    time_t now = time(NULL);
    out << "FLIGHT RECORDER DUMP - " << asctime(localtime(&now)) << '\n';

    while (rindex < windex)
    {
        Entry &e = records[rindex % records.size()];
        out << setw(4) << windex - rindex << ' '
            << setw(16) << e.what << ' '
            << setw(18) << e.caller << ' ';
        if (e.label1[0])
            out << e.label1 << '=' << e.arg1 << ' ';
        if (e.label2[0])
            out << e.label2 << '=' << e.arg2 << ' ';
        if (e.label3[0])
            out << e.label3 << '=' << e.arg3;
        out << '\n';
        rindex++;
    }
}


void FlightRecorder::DumpToLogFile()
// ----------------------------------------------------------------------------
//   Dump to standard Tao log file
// ----------------------------------------------------------------------------
{
    QDir dir(Tao::Application::defaultProjectFolderPath());
    dir.mkpath(dir.absolutePath());
    text path = +dir.absoluteFilePath("tao_flight_recorder.log");
    std::ofstream out(path.c_str());
    recorder->Dump(out);
}


FlightRecorder *     FlightRecorder::recorder = NULL;

}

