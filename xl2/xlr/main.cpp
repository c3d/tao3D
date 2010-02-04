// ****************************************************************************
//  main.cpp                                                       XLR project
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 1992-2010 Catherine Burvelle <cathy@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************
// 
//   File Description:
// 
//     Main entry point for the Qt variant of the compiler
//     We simply invoke the old "main" code which is now in master.cpp
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
// ****************************************************************************
// * File       : $RCSFile$
// * Revision   : $Revision$
// * Date       : $Date$
// ****************************************************************************


int start_xl(int argc, char **argv);

int main(int argc, char **argv)
// ----------------------------------------------------------------------------
//   Start the XL compiler from the library
// ----------------------------------------------------------------------------
{
    start_xl(argc, argv);
}



