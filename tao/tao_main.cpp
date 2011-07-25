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
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 1992-2010 Christophe de Dinechin <christophe@taodyne.com>
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS
// ****************************************************************************

#ifdef CONFIG_MINGW
#define WINVER 0x501
#endif

#include "application.h"
#include "main.h"
#include "basics.h"
#include "graphics.h"
#include "formulas.h"
#include "widget.h"
#include "tao_utf8.h"
#include "gc.h"
#include "tao_main.h"
#include "flight_recorder.h"
#include "tao_utf8.h"

#include <QApplication>
#include <QGLWidget>
#include <QFileInfo>
#include <QTimer>

#include <signal.h>
#include <iomanip>

#ifndef CONFIG_MINGW
#include <execinfo.h>
#include <dlfcn.h>
#endif

#ifdef CONFIG_MINGW
#include <windows.h>
static void win_redirect_io();
#define SIGSTKSZ        4096
#endif

static void cleanup();


int main(int argc, char **argv)
// ----------------------------------------------------------------------------
//    Main entry point of the graphical front-end
// ----------------------------------------------------------------------------
{
    using namespace Tao;

    Tao::FlightRecorder::recorder = new Tao::FlightRecorder;
    RECORD("Tao Starting");
    install_signal_handler(signal_handler);

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

    RECORD("Cleaning up");
    cleanup();

    // HACK: it seems that cleanup() does not clean everything, at least on
    // Windows -- without the exit() call, the windows build crashes at exit
    exit(ret);

    return ret;
}


void cleanup()
// ----------------------------------------------------------------------------
//   Cleaning up before exit
// ----------------------------------------------------------------------------
{
    // First, discard ALL global (smart) pointers to XL types/names
    XL::DeleteBasics();
    DeleteGraphics();     // REVISIT: move to Tao:: namespace?
    TaoFormulas::DeleteFormulas();

    // No more global refs, deleting GC will purge everything
    XL::GarbageCollector::Delete();
}



static text signal_handler_log_file = "tao_flight_recorder.log";

#ifndef CONFIG_MINGW
static char sig_alt_stack[SIGSTKSZ];
#endif


void install_signal_handler(sig_t handler)
// ----------------------------------------------------------------------------
//   Install a signal handler for all "dangerous" signals
// ----------------------------------------------------------------------------
{
    // Record correct path for log file
    using namespace Tao;
    QDir dir(Tao::Application::defaultProjectFolderPath());
    dir.mkpath(dir.absolutePath());
    signal_handler_log_file = +dir.absoluteFilePath("tao_flight_recorder.log");

    // Insert signal handlers
#ifdef CONFIG_MINGW
    static int sigids[] = { SIGINT, SIGILL, SIGABRT,
                            SIGFPE, SIGSEGV, SIGTERM };
#else
    static int sigids[] = { SIGHUP, SIGINT, SIGQUIT, SIGILL, SIGTRAP, SIGABRT,
                            SIGFPE, SIGBUS, SIGSEGV, SIGSYS, SIGPIPE, SIGTERM,
                            SIGXCPU, SIGXFSZ, SIGVTALRM, SIGPROF };
#endif

    for (uint sig = 0; sig < sizeof(sigids) / sizeof(sigids[0]); sig++)
        signal(sigids[sig], handler);

#ifndef CONFIG_MINGW
    if (handler != (sig_t) SIG_DFL)
    {
        // Define alternate stack for signal handler
        stack_t stk;
        stk.ss_sp = sig_alt_stack;
        stk.ss_size = SIGSTKSZ;
        stk.ss_flags = 0;
        if (sigaltstack(&stk, NULL) != 0)
        {
            fprintf(stderr, "WARNING: Could not set alternate stack");
        }
        else
        {
            struct sigaction act;
            act.sa_handler = handler;
            act.sa_flags = SA_ONSTACK | SA_NODEFER | SA_SIGINFO;
            sigemptyset(&act.sa_mask);
            sigaction(SIGSEGV, &act, NULL);
        }
    }
#endif
}


#ifdef CONFIG_MINGW
int backtrace(void **addr, int max)
// ----------------------------------------------------------------------------
//    We are working on it for Windows...
// ----------------------------------------------------------------------------
{
#define BT(n)   if (max > n) addr[n] = __builtin_return_address(n)
    BT(0);
    BT(1);
    BT(2);
    BT(3);
    BT(4);
    BT(5);
    BT(6);
    BT(7);
    return max < 8 ? max : 8;
}
#endif


void signal_handler(int sigid)
// ----------------------------------------------------------------------------
//   Signal handler
// ----------------------------------------------------------------------------
{
    using namespace std;

    // Show something if we get there, even if we abort
    std::cerr << "RECEIVED SIGNAL " << sigid
              << " FROM " << __builtin_return_address(0)
              << " DUMP IN " << signal_handler_log_file
              << "\n";

    // Prevent recursion in the signal handler
    static int recursive = 0;
    if (recursive)
        _Exit(-77);
    recursive = 0;


    // Open directory file
    std::ofstream out(signal_handler_log_file.c_str());

    // Record which signal
    out << "RECEIVED SIGNAL " << sigid
        << " FROM " << __builtin_return_address(0)
        << '\n';

    // Backtrace
    void *addresses[128];
    int count = backtrace(addresses, 128);
    for (int i = 0; i < count; i++)
    {
        out << setw(4)  << i << ' '
            << setw(18) << addresses[i] << ' ';

#ifndef CONFIG_MINGW
        Dl_info info;
        if (dladdr(addresses[i], &info))
            out << setw(32) << setiosflags(ios::left)
                << info.dli_sname << " @ " << info.dli_fname;
#endif

        out << '\n';
    }

    // Dump the flight recorder
    Tao::FlightRecorder::recorder->Dump(out);

    // Flush the output log
    out.close();

    // Install the default signal handler and resume
    install_signal_handler((sig_t) SIG_DFL);
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
        dir.mkpath(dir.absolutePath());
        QString path = dir.absoluteFilePath("tao.log");

        const char *f = path.toStdString().c_str();
        FILE *fp = fopen(f, "w");
        if (fp)
        {
            fclose(fp);
            freopen(f, "a", stdout);
            freopen(f, "a", stderr);
        }
    }
}
#endif

TAO_BEGIN
text Main::SearchFile(text file)
// ----------------------------------------------------------------------------
//   Find the file in the application path
// ----------------------------------------------------------------------------
{
    QFileInfo fileInfo1(+file);
    if (fileInfo1.exists())
        return +fileInfo1.canonicalFilePath();

    text header = "xl:";
    header += file;

    QFileInfo fileInfo2(header.c_str());
    return +fileInfo2.canonicalFilePath();
}


bool Main::Refresh(double delay)
// ----------------------------------------------------------------------------
//   Refresh the current top-level widget if there is one
// ----------------------------------------------------------------------------
{
    Widget *widget = Tao::Widget::Tao();
    QTimer::singleShot(delay * 1000, widget, SLOT(refreshNow(QEvent*)));
    return true;
}


Main *Main::MAIN = NULL;

TAO_END
