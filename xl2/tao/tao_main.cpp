// ****************************************************************************
//  tao_main.cpp                                                    XLR project
// ****************************************************************************
// 
//   File Description:
// 
//     Main entry point for Tao invoking XL
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
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************
// * File       : $RCSFile$
// * Revision   : $Revision$
// * Date       : $Date$
// ****************************************************************************

#include <QtGui>
#include <QtGui/QApplication>
#include <QtGui/QMessageBox>
#include "tao_widget.h"
#include "tao_window.h"
#include "main.h"
#include "graphics.h"



int main(int argc, char **argv)
// ----------------------------------------------------------------------------
//    Main entry point of the graphical front-end
// ----------------------------------------------------------------------------
{
    Q_INIT_RESOURCE(framebufferobject);

    // Initialize the Tao applications
    QApplication tao(argc, argv);
    tao.setApplicationName ("Tao");
    tao.setOrganizationName ("Taodyne SAS");

    // Basic sanity tests to check if we can actually run
    if (!QGLFormat::hasOpenGL())
    {
        QMessageBox::information(0, "OpenGL support",
                                 "This system doesn't support OpenGL.");
        return -1;
    }
    if (!QGLFramebufferObject::hasOpenGLFramebufferObjects())
    {
	QMessageBox::information(0,
                                 "Framebuffer support",
				 "This system does not support framebuffers.");
        return -1;
    }

    // Setup the XL runtime environment
    XL::Compiler compiler("xl_tao");
    XL::Main *xlr = new XL::Main(argc, argv, compiler);
    EnterGraphics(&xlr->context);
    xlr->LoadFiles();

    // Create the window and place a widget in it
    TaoWindow *window = new TaoWindow (xlr);
    window->show();

    return tao.exec();
}



