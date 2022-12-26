#include <windows.h>
#include <string>

using namespace std;

int PAGES = 15;
HANDLE *activeProcesses = new HANDLE[PAGES << 1];

string BASE_PATH = R"(C:\Users\pro10\Documents\codeblocks\os4\)";

bool WRITER_OPTION = true;
bool READER_OPTION = false;

void createProcesses(bool option) {
    string WRITER_PATH = BASE_PATH + "exe\\lab_4_writer.exe";
    string READER_PATH = BASE_PATH + "exe\\lab_4_reader.exe";

    auto startupInfo = new STARTUPINFOA[PAGES];
    auto processInfo = new PROCESS_INFORMATION[PAGES];

    for (int i = 0; i < PAGES; ++i) {
        ZeroMemory(&startupInfo[i], sizeof(startupInfo[i]));

        CreateProcessA(
                option ? WRITER_PATH.c_str() : READER_PATH.c_str(),
                NULL, NULL,
                NULL,
                TRUE,
                0,
                NULL,
                NULL,
                &startupInfo[i],
                &processInfo[i]
        );

        if (option) {
            activeProcesses[i] = processInfo[i].hProcess;
        } else {
            activeProcesses[PAGES + i] = processInfo[i].hProcess;
        }
    }
}

void createSemaphores(bool option) {
    HANDLE *writersSemaphores = new HANDLE[PAGES];
    HANDLE *readersSemaphores = new HANDLE[PAGES];

    for (int i = 0; i < PAGES; ++i) {
        if (option) {
            string semaName = "semaphore_" + to_string(i) + "_writer";
            writersSemaphores[i] = CreateSemaphoreA(NULL, 1, 1, semaName.c_str());
        } else {
            string semaName = "semaphore_" + to_string(i) + "_reader";
            readersSemaphores[i] = CreateSemaphoreA(NULL, 0, 1, semaName.c_str());
        }
    }
}

int main() {
    SYSTEM_INFO system_info;

    GetSystemInfo(&system_info);
    int bufferSize = PAGES * system_info.dwPageSize;

    string bufferPath = BASE_PATH + "buffer.txt";
    string baseBufferName = "buffer";

    HANDLE file = CreateFileA(
            bufferPath.c_str(),
            GENERIC_READ | GENERIC_WRITE,
            FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL,
            CREATE_ALWAYS,
            FILE_ATTRIBUTE_NORMAL,
            NULL
    );

    HANDLE fileMapping = CreateFileMappingA(
            file,
            NULL,
            PAGE_READWRITE,
            0,
            bufferSize,
            baseBufferName.c_str()
    );

    LPVOID view = MapViewOfFile(fileMapping, FILE_MAP_WRITE, 0, 0, bufferSize);
    VirtualLock(view, bufferSize);

    createSemaphores(WRITER_OPTION);
    createSemaphores(READER_OPTION);

    createProcesses(WRITER_OPTION);
    createProcesses(READER_OPTION);

    WaitForMultipleObjects(PAGES << 1, activeProcesses, TRUE, INFINITE);

    return 0;
}
