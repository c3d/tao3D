// ****************************************************************************
//  detach.cpp                                                     Tao project
// ****************************************************************************
//
// What's this?
//
// The source file for the detach.exe program (MS Windows only).
//
// How do I use it?
//
//   detach.exe \path\to\some\program.exe -any parameter...
//
// What does it do?
//
//   Starts program.exe as a detached process (DETACHED_PROCESS flag), and
//   redirect the child's standard output and standard error to our own
//   stdout and stderr channels.
//   The child has no console allocated, but can still send output to the
//   parent's stdout and stderr channels.
//
// Why would I want to do this?
//
//   This is quite useful in the following situation:
//     - A GUI application (app.exe) needs to perform some Git commands, such
//       as "git clone <ssh URI>"
//     - git.exe (from the MSysGit distribution) will execute ssh.exe to deal
//       with ssh:// URIs
//     - ssh typically needs to authenticate the user. By default it does
//       it through the command line, more specifically, using the console
//       (/dev/tty on Unix, the Console on Windows). If there is no console
//       allocated, it checks if the environment variables DISPLAY and
//       SSH_ASK_PASS are set. If so, ssh executes the command $SSH_ASK_PASS.
//     - Our app.exe application wants Git to display a dialog box when a
//       password is needed. In order to do that, it has to:
//         1. Set the DISPLAY variable to a dummy value
//         2. Set the SSH_ASK_PASS variable to be the path to a simple dialog
//            box application that will prompt the user and return the answer
//            on its stdout
//         3. Start git.exe through detach.exe in order to get rid of the
//            console.
//
// This software is property of Taodyne SAS - Confidential
// Ce logiciel est la propriété de Taodyne SAS - Confidentiel
//  (C) 2010 Jerome Forissier <jerome@taodyne.com>
//  (C) 2010 Taodyne SAS

#if defined(UNICODE) && !defined(_UNICODE)
#define _UNICODE 1
#endif

#include <windows.h>
#include <stdio.h>
#include <tchar.h>

HANDLE hChildOut = INVALID_HANDLE_VALUE;
HANDLE hChildErr = INVALID_HANDLE_VALUE;
FILE * debug = NULL;

DWORD WINAPI ReadFromPipe(LPVOID args);
void         ShowLastError();


#define DEBUGP(args...) do {     \
    if (debug) {                 \
        _ftprintf(debug, args);  \
        fflush(debug);           \
    }                            \
} while (0)


int main(int argc, char *argv[])
// ----------------------------------------------------------------------------
//    Program entry point
// ----------------------------------------------------------------------------
{
    char *p = NULL;
    if ((p = getenv("DETACH_DEBUG")))
        debug = fopen(p, "w");

    //
    // Parse command line
    //

    LPTSTR pCmd = ::GetCommandLine();

    // Skip the executable (detach.exe)
    if (*pCmd++ == L'"')
    {
        while (*pCmd++ != L'"');
    }
    else
    {
        while (*pCmd != '\0' && *pCmd != L' ')
            ++pCmd;
    }

    while (*pCmd == L' ')
        pCmd++;

    //
    // Prepare redirection of child stdout/stderr
    //

    HANDLE hProcess = GetCurrentProcess();
    HANDLE h = INVALID_HANDLE_VALUE;

    STARTUPINFO si;
    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    si.dwFlags |= STARTF_USESTDHANDLES;
    si.hStdInput = INVALID_HANDLE_VALUE;
    si.hStdOutput = INVALID_HANDLE_VALUE;
    si.hStdError = INVALID_HANDLE_VALUE;

    SECURITY_ATTRIBUTES sa;
    ZeroMemory(&sa, sizeof(SECURITY_ATTRIBUTES));
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = FALSE;

    CreatePipe(&hChildOut, &h, &sa, 0);
    DuplicateHandle(hProcess, h, hProcess, &si.hStdOutput, 0, TRUE,
                    DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE);

    CreatePipe(&hChildErr, &h, &sa, 0);
    DuplicateHandle(hProcess, h, hProcess, &si.hStdError, 0, TRUE,
                    DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE);

    PROCESS_INFORMATION pi;
    ZeroMemory( &pi, sizeof(pi) );

    //
    // Start child as a detached process
    //

    DEBUGP(L"Creating process - command line: %s\n", pCmd);

    BOOL ok = CreateProcess
        (
            NULL, // No module name (use command line)
            pCmd, // lpCommandLine
            NULL, // Process handle not inheritable
            NULL, // Thread handle not inheritable
            TRUE, // bInheritHandles
            DETACHED_PROCESS, // Detach processes have no console
            NULL, // Use parent's environment block
            NULL, // Use parent's starting directory
            &si,  // Pointer to STARTUPINFO structure
            &pi   // Pointer to PROCESS_INFORMATION structure (returned)
        );
    if (!ok)
    {
        ShowLastError();
        return -1;
    }

    //
    // Close handles that have been inherited
    //

    CloseHandle(si.hStdOutput);
    CloseHandle(si.hStdError);

    WaitForInputIdle(pi.hProcess, 5000);
    CloseHandle(pi.hThread);

    //
    // Start reader threads
    //

    HANDLE thread[2] = {INVALID_HANDLE_VALUE, INVALID_HANDLE_VALUE};

    DEBUGP(L"Starting reader threads\n");
    thread[0] = CreateThread(NULL, 0, ReadFromPipe, &hChildOut, 0, NULL);
    thread[1] = CreateThread(NULL, 0, ReadFromPipe, &hChildErr, 0, NULL);

    //
    // Block until child process ends
    //

    DEBUGP(L"Waiting for child to end\n");
    WaitForSingleObject(pi.hProcess, INFINITE);
    DEBUGP(L"Process ended\n");
    CloseHandle(pi.hProcess);

    //
    // Let reader threads end their jobs
    //

    DEBUGP(L"Waiting for threads to end\n");
    WaitForMultipleObjects(2, thread, TRUE, INFINITE);
    DEBUGP(L"Threads ended\n");
    CloseHandle(thread[0]);
    CloseHandle(thread[1]);

    //
    // We're done
    //

    _flushall();
    return 0;
}


DWORD WINAPI ReadFromPipe(LPVOID args)
// ----------------------------------------------------------------------------
//    Forward child's stdout (or stderr) to our stdout (or stderr)
// ----------------------------------------------------------------------------
{
    HANDLE in = *(HANDLE *)args;
    HANDLE out = (in == hChildOut) ?
                 GetStdHandle(STD_OUTPUT_HANDLE) :
                 GetStdHandle(STD_ERROR_HANDLE);
#   define BUFSIZE 1000
    char *buf = (char *)malloc(BUFSIZE*sizeof(char));
    DWORD nRead, nWritten;

    DEBUGP(L"Reader thread starting: in %p out %p\n", in, out);
    for (;;)
    {
        nRead = 0;
        bool ok = ReadFile(in, buf, BUFSIZE, &nRead, NULL);
        if (!ok || nRead == 0)
        {
            DEBUGP(L"ReadFile error, ok = %d nRead = %d\n", ok, nRead);
            CloseHandle(in);
            return 0;
        }
        DEBUGP(L"[%p>] Read %d bytes\n", in, nRead);
        if (out != INVALID_HANDLE_VALUE)
        {
            nWritten = 0;
            WriteFile(out, buf, nRead, &nWritten, NULL);
            DEBUGP(L"[%p<] Wrote %d bytes\n", out, nWritten);
            FlushFileBuffers(out);
        }
    }

    return 0;
}


void ShowLastError()
// ----------------------------------------------------------------------------
//    Display a message when an error occurred
// ----------------------------------------------------------------------------
{
    _TCHAR msg[2048];
    FormatMessage(
        FORMAT_MESSAGE_FROM_SYSTEM, NULL, ::GetLastError(),
        MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT),
        msg, sizeof(msg), NULL);
    _ftprintf(stderr, msg);
    _flushall();
    DEBUGP(msg);
}
