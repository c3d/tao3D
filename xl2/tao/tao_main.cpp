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
#include "basics.h"
#include "graphics.h"
#include "tao_utf8.h"
#include "gc.h"

#include <QDir>
#include <QtGui>
#include <QtGui/QApplication>
#include <QtGui/QMessageBox>


static void cleanup();

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

    // Fetch info for XL files
    QFileInfo user      ("xl:user.xl");
    QFileInfo theme     ("xl:theme.xl");
    QFileInfo syntax    ("system:xl.syntax");
    QFileInfo stylesheet("system:xl.stylesheet");
    QFileInfo builtins  ("system:builtins.xl");
    QFileInfo tutorial  ("system:tutorial.ddd");

    // Setup the XL runtime environment
    XL::Compiler compiler("xl_tao");
    XL::Main *xlr = new XL::Main(argc, argv, compiler,
                                 +syntax.canonicalFilePath(),
                                 +stylesheet.canonicalFilePath(),
                                 +builtins.canonicalFilePath());
    XL::MAIN = xlr;

    XL::source_names contextFiles;
    EnterGraphics(xlr->context);
    if (user.exists())
        contextFiles.push_back(+user.canonicalFilePath());
    if (theme.exists())
        contextFiles.push_back(+theme.canonicalFilePath());
    xlr->LoadContextFiles(contextFiles);

    // Load the files
    xlr->LoadFiles();

    // Create the windows for each file on the command line
    bool hadFile = false;
    XL::source_names &names = xlr->file_names;
    XL::source_names::iterator it;
    for (it = names.begin(); it != names.end(); it++)
    {
        using namespace Tao;
        if (xlr->files.count(*it))
        {
            XL::SourceFile &sf = xlr->files[*it];
            hadFile = true;
            Tao::Window *window = new Tao::Window (xlr, contextFiles, &sf);
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
        text tuto = +tutorial.canonicalFilePath();
        if (!xlr->LoadFile(tuto))
        {
            if (xlr->files.count(tuto))
            {
                XL::SourceFile &sf = xlr->files[tuto];
                sf.readOnly = true;
                Tao::Window *untitled = new Tao::Window(xlr, contextFiles, &sf);
                untitled->show();
            }
        }
    }

    int ret = tao.exec();

    cleanup();

    // HACK: it seems that cleanup() does not clean everything, at least on
    // Windows -- without the exit() call, the windows build crashes at exit
    exit(ret);

    return ret;
}

namespace TaoFormulas { void DeleteFormulas(); }

void cleanup()
// ----------------------------------------------------------------------------
//   Cleaning up before exit
// ----------------------------------------------------------------------------
{
    // First, discard ALL global (smart) pointers to XL types/names
    XL::Symbols::symbols = NULL;
    XL::Context::context = NULL;
    XL::DeleteBasics();
    DeleteGraphics();     // REVISIT: move to Tao:: namespace?
    TaoFormulas::DeleteFormulas();

    // No more global refs, deleting GC will purge everything
    XL::GarbageCollector::Delete();
}

XL_BEGIN
text Main::SearchFile(text file)
// ----------------------------------------------------------------------------
//   Find the file in the application path
// ----------------------------------------------------------------------------
{
    using namespace Tao;
    text header = "xl:";
    header += file;

    QFileInfo fileInfo(header.c_str());
    return +fileInfo.canonicalFilePath();
}
XL_END
