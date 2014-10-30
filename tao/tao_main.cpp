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
#include "init_cleanup.h"
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
#include "../config.h"
#include "crypto.h"
#include "normalize.h"
#include "opengl_state.h"
#ifndef CFG_NO_DOC_SIGNATURE
#include "document_signature.h"
#endif

#include <QApplication>
#include <QGLWidget>
#include <QFileInfo>
#include <QTimer>

#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#ifndef CONFIG_MINGW
#include <execinfo.h>
#include <dlfcn.h>
#else // CONFIG_MINGW
#include <windows.h>
#include <imagehlp.h>
#endif // MINGW

#if QT_VERSION >= 0x050000
static void taoQt5MessageHandler(QtMsgType type,
                                 const QMessageLogContext &,
                                 const QString &);
#else
static void taoQtMessageHandler(QtMsgType type, const char *msg);
#endif

namespace Tao {
    extern const char * GITREV_;
    extern const char * GITSHA1_;
}

static void redirect_console();

int main(int argc, char **argv)
// ----------------------------------------------------------------------------
//    Main entry point of the graphical front-end
// ----------------------------------------------------------------------------
{
    using namespace Tao;

    // Do our best to send stdout/stderr output somewhere
    redirect_console();

    // Process --version before graphic initialization so that this option may
    // be used without a valid X11 display on Linux
    for (int i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "--version"))
        {
#ifdef TAO_PLAYER
#define EDSTR "Player "
#else
#define EDSTR
#endif
            std::cout << "Tao3D " EDSTR << Tao::GITREV_  <<
                                               " (" << Tao::GITSHA1_ << ")\n";
#undef EDSTR
#ifdef CONFIGURE_OPTIONS
            std::cout << "Configure options: " << CONFIGURE_OPTIONS << "\n";
#endif
            exit(0);
        }
    }

    XL::FlightRecorder::Initialize();
    RECORD(ALWAYS, "Tao Starting");
    install_signal_handler(signal_handler);

    Q_INIT_RESOURCE(tao);

    // Messages sent by the Qt implementation (for instance, with qWarning())
    // should be handled like other Tao error messages
#if QT_VERSION >= 0x050000
    qInstallMessageHandler(taoQt5MessageHandler);
#else
    qInstallMsgHandler(taoQtMessageHandler);
#endif

    // Initialize and run the Tao application
    int ret = 0;

    try
    {
        // Note: keep this inside a block so that ~Application gets called!
        Tao::Application tao(argc, argv);
        ret = tao.exec();

#if QT_VERSION >= 0x050000
        qInstallMessageHandler(0);
#else
        qInstallMsgHandler(0);
#endif
    }
    catch(...)
    {
        RECORD(ALWAYS, "Exception caught at top level");
        std::cerr << "Exception caught, exiting\n";
    }

    try
    {
        RECORD(ALWAYS, "Cleaning up");
        Cleanup();
        
        // HACK: it seems that cleanup() does not clean everything, at least on
        // Windows -- without the exit() call, the windows build crashes at exit
        exit(ret);
    }
    catch(...)
    {
        std::cerr << "Exception caught during cleanup\n";
    }

    return ret;
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


void install_first_exception_handler(void)
// ----------------------------------------------------------------------------
//   Install an unhandled exception handler that happens before LLVM
// ----------------------------------------------------------------------------
{
    RECORD(ALWAYS, "Setting unhandled exception filter");
    // Windows-specific ugliness
    PrimaryExceptionFilter =
        SetUnhandledExceptionFilter(TaoPrimaryExceptionFilter);
}


#else // Real operating systems below

void install_first_exception_handler(void)
// ----------------------------------------------------------------------------
//   Install an unhandled exception handler that happens before LLVM
// ----------------------------------------------------------------------------
{
}

#endif // CONFIG_MINGW


void redirect_console()
// ----------------------------------------------------------------------------
//   No console: log to file. Un-redirected console: log to parent console.
// ----------------------------------------------------------------------------
{
    bool redirectToTaoLog = true;
#ifdef CONFIG_MINGW

    // Note: must be done before AttachConsole()
    DWORD outType, errType;
    outType = GetFileType(GetStdHandle(STD_OUTPUT_HANDLE));
    errType = GetFileType(GetStdHandle(STD_ERROR_HANDLE));

    if (!getenv("TAO_LOG") && AttachConsole(ATTACH_PARENT_PROCESS))
    {
        // Parent has a console.
        // Tested with cmd.exe and MinGW shell (bash):
        //  1/ With no redirection, type is FILE_TYPE_UNKNOWN and output
        //     goes nowhere (at least not to the console).
        //  2/ When redirecting to a file (tao.exe -tfps >tao.log) type is
        //     FILE_TYPE_DISK and output goes to the file.
        //  3/ When sending to a pipe (tao.exe -tfps | grep Time) type is
        //     FILE_TYPE_PIPE and output goes to the pipe.
        // So only case (1) has to be handled specifically.
        if (outType == FILE_TYPE_UNKNOWN)
            freopen("CON", "a", stdout);
        if (errType == FILE_TYPE_UNKNOWN)
            freopen("CON", "a", stderr);
        std::cerr.rdbuf(std::cout.rdbuf());
        redirectToTaoLog = false;
    }

#else // Non Windows, normal systems

    // Check if we have a shell or if stdout is a terminal. Redirect otherwise.
    if (!getenv("TAO_LOG"))
    {
        // Linux: If the output is a TTY. Not sure if it works on MacOSX...
        if (isatty(1))
            redirectToTaoLog = false;

        // MacOSX 10.9
        if (getenv("TERM_PROGRAM"))
            redirectToTaoLog = false;
    }
#endif

    if (redirectToTaoLog)
    {
        // Parent has no console, log to a file
        kstring log = getenv("TAO_LOG");
        if (!log)
            log = "tao.log";

        QDir dir(Tao::Application::defaultProjectFolderPath());
        dir.mkpath(dir.absolutePath());
        QString path = dir.absoluteFilePath(log);
        log = path.toStdString().c_str();

#ifdef CONFIG_MINGW
        freopen(log, "w", stdout);
        stderr->_file = stdout->_file;
        stderr->_flag = stdout->_flag;
        setbuf(stdout, NULL);
        setbuf(stderr, NULL);
        std::cerr.rdbuf(std::cout.rdbuf());
#else
        int fd = open(log, O_RDWR | O_CREAT | O_TRUNC, 0644);
        if (fd != -1)
        {
            dup2(fd, 1);
            dup2(fd, 2);
            fprintf(stderr, "Redirected to tao.log\n");
        }
        else
        {
            fprintf(stderr, "Unable to redirect to %s: %s\n=",
                    log, strerror(errno));
        }
#endif
    }

    // Check that the various output logs are all functioning
    fprintf(stderr, "Initializing Tao - stderr is ready\n");
    std::cerr <<    "                   std::cerr is ready\n";
    printf(         "                   stdout is ready\n");
    std::cout <<    "                   std::cout is ready\n";

    // Defaults for our logging
    std::cout.setf(std::ios::fixed, std::ios::floatfield);
    std::cout.setf(std::ios::showpoint);
    std::cout.precision(3);
}


static char sig_handler_log[512];

void install_signal_handler(sig_t handler)
// ----------------------------------------------------------------------------
//   Install a signal handler for all "dangerous" signals
// ----------------------------------------------------------------------------
{
    // Insert signal handlers
#ifdef CONFIG_MINGW
    static int sigids[] = { SIGILL, SIGABRT, SIGFPE, SIGSEGV, SIGTERM };
#else
    static int sigids[] = { SIGHUP, SIGQUIT, SIGILL, SIGTRAP, SIGABRT,
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
        QString dateTimeS = dateTime.toString("yyyyMMdd-HHmmss");
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
    TopLevelExceptionFilter =
        SetUnhandledExceptionFilter(TaoUnhandledExceptionFilter);
#endif // CONFIG_MINGW
    }
}


static void Write(int fd, const char *buf, size_t size)
// ----------------------------------------------------------------------------
//   write() wrapper
// ----------------------------------------------------------------------------
{
    size_t left = size;
    while (left)
    {
        size_t s = write(fd, buf + size - left, left);
        if (s > 0)
            left -= s;
    }
}


void signal_handler(int sigid)
// ----------------------------------------------------------------------------
//   Signal handler
// ----------------------------------------------------------------------------
{
    using namespace std;
    using namespace Tao;
    static char buffer[512];
    int two = fileno(stderr);

    kstring vendor = "Unknown";
    kstring renderer = "Unknown";
    kstring version = "Unknown";

    if (OpenGLState *state = OpenGLState::State())
    {
        vendor = state->Vendor().c_str();
        renderer = state->Renderer().c_str();
        version = state->Version().c_str();
    }
    else if (QGLContext::currentContext() &&
             QGLContext::currentContext()->isValid())
    {
        // Get OpenGL supported version
        vendor = (kstring) glGetString(GL_VENDOR);
        renderer = (kstring) glGetString(GL_RENDERER);
        version = (kstring) glGetString(GL_VERSION);
    }

    // Show something if we get there, even if we abort
    size_t size = snprintf(buffer, sizeof buffer,
                           "RECEIVED SIGNAL %d FROM %p\n"
                           "DUMP IN %s\n"
                           "TAO VERSION: %s (%s)\n"
                           "GL VENDOR:   %s\n"
                           "GL RENDERER: %s\n"
                           "GL VERSION:  %s\n"
                           "\n\n"
                           "STACK TRACE:\n",
                           sigid, __builtin_return_address(0),
                           sig_handler_log,
                           Tao::GITREV_,
                           Tao::GITSHA1_,
                           vendor, renderer, version);

    Write(two, buffer, size);

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
    Write(fd, buffer, size);

    // Backtrace
    tao_stack_trace(fd);
    tao_stack_trace(two);
        
    // Dump the flight recorder
    Write (fd, "\n\n", 2);
    XL::FlightRecorder::SDump(fd, false);

    // Close the output stream
    close(fd);

#ifdef CONFIG_MINGW
    if (sigid == SIGTERM)
#else
    if (sigid == SIGQUIT || sigid == SIGTERM)
#endif
    {
        ::exit(0);
    }
    else
    {
        // Install the default signal handler and resume
        install_signal_handler((sig_t) SIG_DFL);
    }
}


void tao_stack_trace(int fd)
// ----------------------------------------------------------------------------
//    Dump a stack trace on the given file descriptor
// ----------------------------------------------------------------------------
// We are still  working on it for Windows...
{
#ifndef CONFIG_MINGW
    void *addresses[128];
    char buffer[512];
    int count = backtrace(addresses, 128);
    for (int i = 0; i < count; i++)
    {
        uint size = snprintf(buffer, sizeof buffer,
                             "%4d %18p ", i, addresses[i]);
            
        Dl_info info;
        if (dladdr(addresses[i], &info))
            size += snprintf(buffer + size, sizeof buffer - size,
                             "%32s [%s]",
                             info.dli_sname, info.dli_fname);
        if (size < sizeof buffer)
            buffer[size++] = '\n';

        Write(fd, buffer, size);
    }

#else // Test for the non-operating systems

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
                                 ", %s, line %lu",
                                 line.FileName, line.LineNumber);
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
#endif // MINGW
}


#if QT_VERSION >= 0x050000
static void taoQt5MessageHandler(QtMsgType type,
                                 const QMessageLogContext &,
                                 const QString &msg)
// ----------------------------------------------------------------------------
//   Handle diagnostic messages from Qt like any other Tao message
// ----------------------------------------------------------------------------
{
    Q_UNUSED(type);
    if (qApp && ((Tao::Application*)qApp)->addError(msg.toUtf8()))
        return;
    std::cerr << msg.toUtf8().constData();
}


#else

static void taoQtMessageHandler(QtMsgType type, const char *msg)
// ----------------------------------------------------------------------------
//   Handle diagnostic messages from Qt like any other Tao message
// ----------------------------------------------------------------------------
{
    Q_UNUSED(type);
    if (qApp && ((Tao::Application*)qApp)->addError(msg))
        return;
    std::cerr << msg;
}
#endif


TAO_BEGIN
int Main::LoadFile(text file, bool updateContext,
                   XL::Context *importContext, XL::Symbols *importSymbols)
// ----------------------------------------------------------------------------
//   Call XLR to load file. Attach signature info to tree if file is signed.
// ----------------------------------------------------------------------------
{
    int ret = XL::Main::LoadFile(file, updateContext, importContext,
                                 importSymbols);

#ifndef CFG_NO_DOC_SIGNATURE
    // (Re-) check file signature.
    XL::SourceFile &sf = XL::MAIN->files[file];
    SignatureInfo * si = sf.GetInfo<SignatureInfo>();
    if (!si)
    {
        si = new SignatureInfo(file);
        sf.SetInfo<SignatureInfo>(si);
    }
    SignatureInfo::Status st = si->loadAndCheckSignature();
    if (st != SignatureInfo::SI_VALID)
    {
        sf.Remove(si);
        delete si;
    }
    // Now if SourceFile has a SignatureInfo it means it has a valid
    // signature.
#endif

    return ret;
}


text Main::SearchFile(text file)
// ----------------------------------------------------------------------------
//   Find the file in the application path
// ----------------------------------------------------------------------------
{
    QFileInfo fileInfo1(+file);
    if (fileInfo1.exists())
        return +fileInfo1.absoluteFilePath();

    text header = "xl:";
    header += file;

    QFileInfo fileInfo2(header.c_str());
    if (fileInfo2.exists())
       return +fileInfo2.absoluteFilePath();

    return "";
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


text Main::Decrypt(text file)
// ----------------------------------------------------------------------------
//   Attempt decryption of file
// ----------------------------------------------------------------------------
{
    QFileInfo fi(+file);
    if (fi.exists())
    {
#ifndef CFG_NO_CRYPT
        QFile f(+file);
        if (f.open(QIODevice::ReadOnly))
        {
            QByteArray ba = f.readAll();
            text in;
            in.append(ba.data(), ba.size());
            return Crypto::Decrypt(in);
        }
#endif
    }
    return "";
}


Tree *Main::Normalize(Tree *input)
// ----------------------------------------------------------------------------
//   Normalize the input tree
// ----------------------------------------------------------------------------
{
    if (Widget *widget = Widget::TaoExists())
    {
        Renormalize renorm(widget);
        input = input->Do(renorm);
    }
    return input;
}


Main *Main::MAIN = NULL;

TAO_END
