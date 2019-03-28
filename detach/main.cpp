// *****************************************************************************
// main.cpp                                                        Tao3D project
// *****************************************************************************
//
// File description:
//
//
//
//
//
//
//
//
// *****************************************************************************
// This software is licensed under the GNU General Public License v3
// (C) 2010-2011, Catherine Burvelle <catherine@taodyne.com>
// (C) 2014,2019, Christophe de Dinechin <christophe@dinechin.org>
// (C) 2010-2012, Jérôme Forissier <jerome@taodyne.com>
// *****************************************************************************
// This file is part of Tao3D
//
// Tao3D is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Tao3D is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Tao3D, in a file named COPYING.
// If not, see <https://www.gnu.org/licenses/>.
// *****************************************************************************

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


int main(int, char *[])
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

    //
    // Read child process exit status
    //

    DWORD status = 0;
    GetExitCodeProcess(pi.hProcess, &status);
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

    DEBUGP(L"Return status = %d", status);
    _flushall();
    return status;
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
