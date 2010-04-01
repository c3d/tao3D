// ****************************************************************************
//  tao_main.cpp                                                    Tao project
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

#include "application.h"
#include "widget.h"
#include "window.h"
#include "main.h"
#include "graphics.h"
#include "process.h"

#include <QtGui>
#include <QtGui/QApplication>
#include <QtGui/QMessageBox>


int main(int argc, char **argv)
// ----------------------------------------------------------------------------
//    Main entry point of the graphical front-end
// ----------------------------------------------------------------------------
{
    Q_INIT_RESOURCE(tao);

    // We need to brute-force option parsing here, the OpenGL choice must
    // be made before calling the QApplication constructor...
    for (int a = 1; a < argc; a++)
        if (text(argv[a]) == "-gl")
            QGL::setPreferredPaintEngine(QPaintEngine::OpenGL);

    // Setup the XL runtime environment
    XL::Compiler compiler("xl_tao");
    XL::Main *xlr = new XL::Main(argc, argv, compiler);
    XL::MAIN = xlr;
    EnterGraphics(&xlr->context);
    xlr->LoadFiles();

    // Initialize the Tao application
    Tao::Application tao(argc, argv);

    // Create the windows for each file on the command line
    XL::source_names::iterator it;
    XL::source_names &names = xlr->file_names;
    bool hadFile = false;
    for (it = names.begin(); it != names.end(); it++)
    {
        using namespace Tao;
        if (xlr->files.count(*it))
        {
            XL::SourceFile &sf = xlr->files[*it];
            hadFile = true;
            Tao::Window *window = new Tao::Window (xlr, &sf);
            if (window->isUntitled)
                delete window;
            else
                window->show();
        }
        else
        {
            QMessageBox::warning(NULL, tao.tr("Invalid input file"),
                                 tao.tr("The file %1 cannot be read.")
                                 .arg(+*it));
        }
    }
    if (!hadFile)
    {
        Tao::Window *untitled = new Tao::Window(xlr, NULL);
        untitled->show();
    }

    return tao.exec();
}



