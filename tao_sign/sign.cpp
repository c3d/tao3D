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
// This document is released under the GNU General Public License.
// See http://www.gnu.org/copyleft/gpl.html and Matthew 25:22 for details
//  (C) 2011 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2011 Taodyne SAS
// ****************************************************************************

#define KEYGEN
#include "licence.h"
#include "licence.cpp"
#include "version.h"
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
        Tao::Licences::AddLicenceFile(argv[arg]);
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
