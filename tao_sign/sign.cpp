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
#include <iostream>

XL_DEFINE_TRACES

int main(int argc, char *argv[])
// ----------------------------------------------------------------------------
//    Generate key files for Tao
// ----------------------------------------------------------------------------
{
    XL::FlightRecorder::Initialize();
    XL::MAIN = new XL::Main(argc, argv);
    for (int arg = 1; arg < argc; arg++)
        Tao::Licences::AddLicenceFile(argv[arg]);
}
