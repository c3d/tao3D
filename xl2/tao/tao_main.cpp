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
#include "widget.h"
#include "window.h"
#include "main.h"
#include "graphics.h"



int main(int argc, char **argv)
// ----------------------------------------------------------------------------
//    Main entry point of the graphical front-end
// ----------------------------------------------------------------------------
{
    Q_INIT_RESOURCE(tao);

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
    XL::MAIN = xlr;
    EnterGraphics(&xlr->context);
    xlr->LoadFiles();

    // Create the windows for each file on the command line
    XL::source_names::iterator it;
    XL::source_names &names = xlr->file_names;
    for (it = names.begin(); it != names.end(); it++)
    {
        XL::SourceFile &sf = xlr->files[*it];
        Tao::Window *window = new Tao::Window (xlr, &sf);
        window->show();
    }
    if (names.begin() == names.end())
    {
        Tao::Window *untitled = new Tao::Window(xlr, NULL);
        untitled->show();
    }

    return tao.exec();
}



