#include <windows.h>
#include <iostream>
#include <string>

using namespace std;

void CALLBACK completionCallback(DWORD errorCode, DWORD bytestransfered, LPOVERLAPPED lpOverlapped) {
    cout << "Completion callback works!" << endl;
}

int main() {
    char *data = new char[256];
    OVERLAPPED overlapped;
    string pipeName = R"(\\.\pipe\name)";

    ZeroMemory(&overlapped, sizeof(overlapped));

    HANDLE pipe = CreateFileA(
            pipeName.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            0, NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL,
            NULL
    );

    cout << "Results:" << endl;

    BOOL readFileRes;
    BOOL running = true;

    while (running) {
        readFileRes = ReadFileEx(
                pipe,
                data,
                256,
                &overlapped,
                completionCallback
        );

        if (readFileRes && pipe != INVALID_HANDLE_VALUE) {
            cout << data << endl << endl;
            SleepEx(INFINITE, TRUE);
        } else {
            cout << "Finish";
            running = false;
        }
    }

    return 0;
}
