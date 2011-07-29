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
#include <stdio.h>
#include <fcntl.h>

#ifndef CONFIG_MINGW
#include <execinfo.h>
#include <dlfcn.h>
#endif

#ifdef CONFIG_MINGW
#include <windows.h>
#include <imagehlp.h>
static void win_redirect_io();
#endif

static void cleanup();

int main(int argc, char **argv)
// ----------------------------------------------------------------------------
//    Main entry point of the graphical front-end
// ----------------------------------------------------------------------------
{
    using namespace Tao;

    XL::FlightRecorder::Initialize();
    RECORD(ALWAYS, "Tao Starting");
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

    RECORD(ALWAYS, "Cleaning up");
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



#ifdef CONFIG_MINGW
// Installed when the program starts
static PTOP_LEVEL_EXCEPTION_FILTER TopLevelExceptionFilter = NULL;
static LPEXCEPTION_POINTERS TopLevelExceptionPointers = NULL;

// Installed after LLVM installs its own, so that we get first dib
static PTOP_LEVEL_EXCEPTION_FILTER PrimaryExceptionFilter = NULL;
static LPEXCEPTION_POINTERS PrimaryExceptionPointers = NULL;


static LONG WINAPI TaoUnhandledExceptionFilter(LPEXCEPTION_POINTERS ep) 
// ----------------------------------------------------------------------------
//   Call signal handler on unhandled exception
// ----------------------------------------------------------------------------
{
    if (!TopLevelExceptionPointers && !PrimaryExceptionPointers)
    {
        TopLevelExceptionPointers = ep;
        signal_handler(SIGSEGV);
    }
    
    // Allow dialog box to pop up allowing choice to start debugger.
    if (TopLevelExceptionFilter)
        return (*TopLevelExceptionFilter)(ep);
    return EXCEPTION_CONTINUE_SEARCH;
}

static BOOL WINAPI TaoConsoleCtrlHandler(DWORD dwCtrlType)
// ----------------------------------------------------------------------------
//   Call signal handler on Control-C
// ----------------------------------------------------------------------------
{
    signal_handler(SIGINT);
    return FALSE; // Kill me, please, I'm running on Windows!
}


static LONG WINAPI TaoPrimaryExceptionFilter(LPEXCEPTION_POINTERS ep) 
// ----------------------------------------------------------------------------
//   Call signal handler on unhandled exception before LLVM does
// ----------------------------------------------------------------------------
{
    if (!PrimaryExceptionPointers)
    {
        PrimaryExceptionPointers = ep;
        signal_handler(SIGSEGV);
    }
        
    // Allow dialog box to pop up allowing choice to start debugger.
    if (PrimaryExceptionFilter)
        return (*PrimaryExceptionFilter)(ep);
    return EXCEPTION_CONTINUE_SEARCH;
}


static void TaoStackTrace(int fd)
// ----------------------------------------------------------------------------
//    We are working on it for Windows...
// ----------------------------------------------------------------------------
{
#ifdef _WIN64
    // TODO: provide a x64 friendly version of the following
#else

    LPEXCEPTION_POINTERS ep = PrimaryExceptionPointers;
    if (!ep)
        ep = TopLevelExceptionPointers;
    if (ep)
    {
        static char buffer[512];
        int two = fileno(stderr);
    
        // Initialize the STACKFRAME structure.
        STACKFRAME StackFrame;
        memset(&StackFrame, 0, sizeof(StackFrame));

        StackFrame.AddrPC.Offset = ep->ContextRecord->Eip;
        StackFrame.AddrPC.Mode = AddrModeFlat;
        StackFrame.AddrStack.Offset = ep->ContextRecord->Esp;
        StackFrame.AddrStack.Mode = AddrModeFlat;
        StackFrame.AddrFrame.Offset = ep->ContextRecord->Ebp;
        StackFrame.AddrFrame.Mode = AddrModeFlat;

        HANDLE hProcess = GetCurrentProcess();
        HANDLE hThread = GetCurrentThread();

        // Initialize the symbol handler.
        SymSetOptions(SYMOPT_DEFERRED_LOADS|SYMOPT_LOAD_LINES);
        SymInitialize(hProcess, NULL, TRUE);

        while (true)
        {
            if (!StackWalk(IMAGE_FILE_MACHINE_I386,
                           hProcess, hThread, &StackFrame,
                           ep->ContextRecord, NULL, SymFunctionTableAccess,
                           SymGetModuleBase, NULL))
                break;

            if (StackFrame.AddrFrame.Offset == 0)
                break;

            // Print the PC in hexadecimal.
            DWORD PC = StackFrame.AddrPC.Offset;
            size_t size = snprintf(buffer, sizeof buffer, "%08lX", PC);

            // Print the parameters.  Assume there are four.
            size += snprintf(buffer + size, sizeof buffer - size,
                             " (0x%08lX 0x%08lX 0x%08lX 0x%08lX)",
                             StackFrame.Params[0], StackFrame.Params[1],
                             StackFrame.Params[2], StackFrame.Params[3]);
            write (fd, buffer, size);
            write (two, buffer, size);

            // Verify the PC belongs to a module in this process.
            if (!SymGetModuleBase(hProcess, PC))
            {
                char msg[] = "<unknown module>";
                write (fd, msg, sizeof msg - 1);
                write (two, msg, sizeof msg - 1);
                continue;
            }

            // Print the symbol name.
            IMAGEHLP_SYMBOL *symbol = (IMAGEHLP_SYMBOL *) buffer;
            memset(symbol, 0, sizeof(IMAGEHLP_SYMBOL));
            symbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
            symbol->MaxNameLength = 512 - sizeof(IMAGEHLP_SYMBOL);

            DWORD dwDisp;
            if (!SymGetSymFromAddr(hProcess, PC, &dwDisp, symbol))
            {
                write(fd, "\n", 1);
                write(two, "\n", 1);
                continue;
            }

            buffer[511] = 0;
            if (dwDisp > 0)
                size = snprintf(buffer, sizeof buffer,
                                ", %s()+%04lu", symbol->Name, dwDisp);
            else
                size = snprintf(buffer, sizeof buffer,
                                ", %s", symbol->Name);
            

            // Print the source file and line number information.
            IMAGEHLP_LINE line;
            memset(&line, 0, sizeof(line));
            line.SizeOfStruct = sizeof(line);
            if (SymGetLineFromAddr(hProcess, PC, &dwDisp, &line))
            {
                size += snprintf(buffer + size, sizeof buffer - size,
                                 ", %s, line %lu", line.FileName, line.LineNumber);
                if (dwDisp > 0)
                    size += snprintf(buffer + size, sizeof buffer - size,
                                     "+%04lu", dwDisp);
            }

            if (size < sizeof buffer)
                buffer[size++] = '\n';
            write (fd, buffer, size);
            write (two, buffer, size);
        } // while(true)
    }
#endif // WIN64
}


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


void install_first_exception_handler(void)
// ----------------------------------------------------------------------------
//   Install an unhandled exception handler that happens before LLVM
// ----------------------------------------------------------------------------
{
    // Windows-specific ugliness
    PrimaryExceptionFilter = SetUnhandledExceptionFilter(TaoPrimaryExceptionFilter);
    SetConsoleCtrlHandler(TaoConsoleCtrlHandler, TRUE);
}


#else // Real operating systems below

void install_first_exception_handler(void)
// ----------------------------------------------------------------------------
//   Install an unhandled exception handler that happens before LLVM
// ----------------------------------------------------------------------------
{
}

#endif // CONFIG_MINGW


static char sig_handler_log[512];

void install_signal_handler(sig_t handler)
// ----------------------------------------------------------------------------
//   Install a signal handler for all "dangerous" signals
// ----------------------------------------------------------------------------
{
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

    if (handler != (sig_t) SIG_DFL)
    {
        // Record correct path for log file
        using namespace Tao;
        QDir dir(Tao::Application::defaultProjectFolderPath());
        dir.mkpath(dir.absolutePath());
        QDateTime dateTime = QDateTime::currentDateTime();
        QString dateTimeS = dateTime.toString(Qt::ISODate);
        text logfile = +dir.absoluteFilePath("tao-dump-" + dateTimeS + ".log");
        strncpy(sig_handler_log, logfile.c_str(), sizeof sig_handler_log);

#if 0
        // Disabled alt-stack, as it doesn't help with catching stack overflows
        // on MacOSX as intended, but also prevents backtrace() from working
#ifndef CONFIG_MINGW
        static char sig_alt_stack[SIGSTKSZ];

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
            act.sa_flags = SA_ONSTACK | SA_NODEFER;
            sigemptyset(&act.sa_mask);
            sigaction(SIGSEGV, &act, NULL);
        }
#endif // CONFIG_MINGW
#endif // if 0

#ifdef CONFIG_MINGW
    // Windows-specific ugliness
    TopLevelExceptionFilter = SetUnhandledExceptionFilter(TaoUnhandledExceptionFilter);
    SetConsoleCtrlHandler(TaoConsoleCtrlHandler, TRUE);
#endif // CONFIG_MINGW
    }
}


void signal_handler(int sigid)
// ----------------------------------------------------------------------------
//   Signal handler
// ----------------------------------------------------------------------------
{
    using namespace std;
    static char buffer[512];
    int two = fileno(stderr);

    // Show something if we get there, even if we abort
    size_t size = snprintf(buffer, sizeof buffer,
                           "RECEIVED SIGNAL %d FROM %p\n"
                           "DUMP IN %s\n"
                           "\n\n"
                           "STACK TRACE:\n",
                           sigid, __builtin_return_address(0),
                           sig_handler_log);
    size_t ignored = write(two, buffer, size);

    // Prevent recursion in the signal handler
    static int recursive = 0;
    if (recursive)
        _Exit(-77);
    recursive = 1;

    // Open stream for location where we'll write information
#ifdef CONFIG_MINGW
    // Can't have : in the time
    for (char *ptr = sig_handler_log + 2; *ptr; ptr++)
        if (*ptr == ':')
            *ptr = '-';
#endif
    int fd = open(sig_handler_log, O_WRONLY|O_CREAT, 0666);
    ignored = write (fd, buffer, size);

    // Backtrace
#ifdef CONFIG_MINGW
    TaoStackTrace(fd);
#else // Real operating systems
    void *addresses[128];
    int count = backtrace(addresses, 128);
    for (int i = 0; i < count; i++)
    {
        size = snprintf(buffer, sizeof buffer,
                        "%4d %18p ", i, addresses[i]);
            
        Dl_info info;
        if (dladdr(addresses[i], &info))
            size += snprintf(buffer + size, sizeof buffer - size,
                             "%32s [%s]",
                             info.dli_sname, info.dli_fname);
        if (size < sizeof buffer)
            buffer[size++] = '\n';

        ignored = write (fd, buffer, size);
        ignored = write (two, buffer, size);
    }
#endif // Test which (non-) operating system we have
        
    // Dump the flight recorder
    ignored = write (fd, "\n\n", 2);
    XL::FlightRecorder::SDump(fd, false);

    // Close the output stream
    close(fd);

    // Install the default signal handler and resume
    install_signal_handler((sig_t) SIG_DFL);
}


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
