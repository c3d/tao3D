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
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#include "application.h"
#include "main.h"
#include "basics.h"
#include "graphics.h"
#include "tao_utf8.h"
#include "gc.h"

#include <QApplication>
#include <QGLWidget>
#include <QFileInfo>

#ifdef CONFIG_MINGW
#include <windows.h>
static void win_redirect_io();
#endif

static void cleanup();

int main(int argc, char **argv)
// ----------------------------------------------------------------------------
//    Main entry point of the graphical front-end
// ----------------------------------------------------------------------------
{
    using namespace Tao;

    Q_INIT_RESOURCE(tao);

#ifdef CONFIG_MINGW
    // Do our best to send stdout/stderr output somewhere
    win_redirect_io();
#endif

    // We need to brute-force option parsing here, the OpenGL choice must
    // be made before calling the QApplication constructor...
    for (int a = 1; a < argc; a++)
        if (text(argv[a]) == "-gl")
            QGL::setPreferredPaintEngine(QPaintEngine::OpenGL);

    // Initialize and run the Tao application
    int ret = 0;
    {
        Tao::Application tao(argc, argv);
        bool ok = tao.processCommandLine();
        if (ok)
            ret = tao.exec();
        // Note: keep this inside a block so that ~Application gets called!
    }
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

#ifdef CONFIG_MINGW
void win_redirect_io()
// ----------------------------------------------------------------------------
//   Send stdout and stderr to parent console if we have one, or to a file
// ----------------------------------------------------------------------------
{
    if (AttachConsole(ATTACH_PARENT_PROCESS))
    {
        // Log to console of parent process
        freopen("CON", "a", stdout);
        freopen("CON", "a", stderr);
    }
    else
    {
        // Parent has no console, log to a file
        QDir dir(Tao::Application::defaultProjectFolderPath());
        QString path = dir.absoluteFilePath("tao.log");
        const char *f = path.toStdString().c_str();
        fclose(fopen(f, "w"));
        freopen(f, "a", stdout);
        freopen(f, "a", stderr);
    }
}
#endif

XL_BEGIN
text Main::SearchFile(text file)
// ----------------------------------------------------------------------------
//   Find the file in the application path
// ----------------------------------------------------------------------------
{
    using namespace Tao;
    QFileInfo fileInfo1(+file);
    if (fileInfo1.exists())
        return +fileInfo1.canonicalFilePath();

    text header = "xl:";
    header += file;

    QFileInfo fileInfo2(header.c_str());
    return +fileInfo2.canonicalFilePath();
}
XL_END
