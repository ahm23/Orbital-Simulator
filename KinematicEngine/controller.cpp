#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <string>

#include "engine.h"

#include "../commons/ipc.h"

int main() {
    DWORD dwRead;
    HANDLE hStdin, hStdout;
    BOOL bSuccess;
    std::condition_variable_any cv;
    std::mutex mut;
    int status = 0;

    ENGINE* e = new ENGINE(4, &mut, &cv, &status);

    while (!status);
    if (status == 2)
        cv.notify_all();


    hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    hStdin = GetStdHandle(STD_INPUT_HANDLE);

    if (hStdout == INVALID_HANDLE_VALUE || hStdin == INVALID_HANDLE_VALUE)
        ExitProcess(1);

    for (;;) {
        // Read from StdIN
        P_PACKET p;
        bSuccess = ReadFile(hStdin, p.buffer, sizeof(PACKET), &dwRead, NULL);
        if (p.p.type == REQ_TYPE::INIT) {
            B_INIT dat;
            memcpy(&dat, p.p.body, sizeof(B_INIT));
            e->addObject(p.p.id, dat);
        }
    }
    return 0;
}