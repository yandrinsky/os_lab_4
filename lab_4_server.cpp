#include <windows.h>
#include <iostream>
#include <string>

using namespace std;

int main() {
    char *data = new char[256];
    OVERLAPPED overlapped;
    DWORD written;

    ZeroMemory(&overlapped, sizeof(overlapped));

    string eventName = "eventName";
    overlapped.hEvent = CreateEventA(NULL, TRUE, FALSE, eventName.c_str());

    string pipeName = R"(\\.\pipe\name)";

    HANDLE pipe = CreateNamedPipeA(
            pipeName.c_str(),
            PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
            PIPE_TYPE_BYTE | PIPE_WAIT,
            1,
            256,
            256, 0,
            NULL
    );

    ConnectNamedPipe(pipe, &overlapped);

    cout << "Enter the data (break - to stop): " << endl;

    bool running = true;

    while (running) {
        cin >> data;
        if (strcmp(data, "break") != 0) {
            cout << endl;

            WriteFile(
                    pipe,
                    data,
                    strlen(data) + 1,
                    &written,
                    &overlapped
            );

            WaitForSingleObject(overlapped.hEvent, INFINITE);
        } else {
            cout << "Finish";
            running = false;
        }
    }

    DisconnectNamedPipe(pipe);

    return 0;
}
