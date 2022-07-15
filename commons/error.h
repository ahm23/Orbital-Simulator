#pragma once
#include <Windows.h>
#include <strsafe.h>

enum ERR {
    FATAL,
    STD
};

void ThrowError(ERR type, LPCTSTR text) {
    LPVOID lpDisplayBuf;

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, (lstrlen(text) + 40) * sizeof(TCHAR));
    StringCchPrintf((LPTSTR)lpDisplayBuf, LocalSize(lpDisplayBuf) / sizeof(TCHAR), TEXT("ERROR | %s"), text);
    
    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, ((type == ERR::FATAL) ? TEXT("Fatal Error") : TEXT("Error")), MB_OK);

    LocalFree(lpDisplayBuf);
    if (type == ERR::FATAL)
        ExitProcess(1);
}

void ThrowWarn(LPCTSTR text) {
    LPVOID lpDisplayBuf;

    lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT, (lstrlen(text) + 40) * sizeof(TCHAR));
    StringCchPrintf((LPTSTR)lpDisplayBuf, LocalSize(lpDisplayBuf) / sizeof(TCHAR), TEXT("WARNING | %s"), text);

    MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("Warning"), MB_OK);

    LocalFree(lpDisplayBuf);
}