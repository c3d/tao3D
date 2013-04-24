// ****************************************************************************
//  keygen.cpp                                                      Tao project
// ****************************************************************************
// 
//   File Description:
// 
// 
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
//  (C) 2011 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2011 Taodyne SAS
// ****************************************************************************

#include "license.h"
#include "version.h"
#include "main.h"
#include "flight_recorder.h"
#include <iostream>

XL_DEFINE_TRACES

void usage(const char *progname);

int main(int argc, char *argv[])
// ----------------------------------------------------------------------------
//    Generate key files for Tao
// ----------------------------------------------------------------------------
{
    if ((argc > 1) && std::string(argv[1]) == "-h")
        usage(argv[0]);

    XL::FlightRecorder::Initialize();
    XL::MAIN = new XL::Main(argc, argv);
    for (int arg = 1; arg < argc; arg++)
        Tao::Licenses::AddLicenseFile(argv[arg]);
}

void usage(const char *progname)
// ----------------------------------------------------------------------------
//    Show usage
// ----------------------------------------------------------------------------
{
    std::cerr << "Taodyne license file signing tool version "
              << GITREV " (" GITSHA1 ")\n";
    std::cerr << "Usage: " << progname << " <files>\n\n";
    exit(0);
}
