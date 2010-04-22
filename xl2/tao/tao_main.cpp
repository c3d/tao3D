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
#include "tao_utf8.h"

#include <QDir>
#include <QtGui>
#include <QtGui/QApplication>
#include <QtGui/QMessageBox>


int main(int argc, char **argv)
// ----------------------------------------------------------------------------
//    Main entry point of the graphical front-end
// ----------------------------------------------------------------------------
{
    using namespace Tao;

    Q_INIT_RESOURCE(tao);

    // We need to brute-force option parsing here, the OpenGL choice must
    // be made before calling the QApplication constructor...
    for (int a = 1; a < argc; a++)
        if (text(argv[a]) == "-gl")
            QGL::setPreferredPaintEngine(QPaintEngine::OpenGL);

    // Initialize the Tao application
    Tao::Application tao(argc, argv);

    // This name is present in the argument list if ever.
    QString project_name = "";

    // Initialize dir search path for XL files
    QStringList xl_dir_list;
    xl_dir_list << tao.defaultProjectFolderPath() + project_name
                << tao.defaultTaoPreferencesFolderPath()
                << tao.defaultTaoApplicationFolderPath();
    QDir::setSearchPaths("xl", xl_dir_list);
    QFileInfo builtins  ("xl:builtins");
    QFileInfo syntax    ("xl:xl.syntax");
    QFileInfo stylesheet("xl:xl.stylesheet");

    // Setup the XL runtime environment
    XL::Compiler compiler("xl_tao");
    XL::Main *xlr = new XL::Main(argc, argv, compiler,
                                 +builtins.canonicalFilePath(),
                                 +syntax.canonicalFilePath(),
                                 +stylesheet.canonicalFilePath());
    XL::MAIN = xlr;
    EnterGraphics(&xlr->context);
    xlr->LoadFiles();

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
