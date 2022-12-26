#include <windows.h>
#include <string>
#include <stdlib.h>
#include <ctime>

using namespace std;

int PAGES = 15;

HANDLE *writersSemaphores = new HANDLE[PAGES];
HANDLE *readersSemaphores = new HANDLE[PAGES];
string BASE_PATH = R"(C:\Users\pro10\Documents\codeblocks\os4\)";

class Logger {
private:
    FILE *baseLog;
    FILE *liteLog;
public:
    Logger() {
        string BASE_LOGFILE_PATH = BASE_PATH + "reading_logs\\";
        string LITE_LOGFILE_PATH = BASE_PATH + "reading_lite_logs\\";

        string baseLogName = BASE_LOGFILE_PATH + to_string(GetCurrentProcessId()) + ".txt";
        baseLog = fopen(baseLogName.c_str(), "w");

        string liteLogName = LITE_LOGFILE_PATH + to_string(GetCurrentProcessId()) + ".txt";
        liteLog = fopen(liteLogName.c_str(), "w");
    }

    void logWaiting() {
        fprintf(baseLog, "|State: WAITING|\t\t\t\t|Time: %lu|\n", GetTickCount());
        fprintf(liteLog, "%lu %d\n", GetTickCount(), 0);
    }

    void logReading(DWORD page) {
        fprintf(baseLog, "|State: WRITING; Page: %lu|\t\t\t|Time: %lu|\n", page, GetTickCount());
        fprintf(liteLog, "%lu %d\n", GetTickCount(), 1);
    }

    void logReleased(char *data) {
        fprintf(
                baseLog,
                "|State: RELEASED; Read data: %s|\t\t|time: %lu|\n",
                data,
                GetTickCount()
        );
        fprintf(liteLog, "%lu %d\n", GetTickCount(), 2);
    }

    static void appendGeneralLog(DWORD page, int state) {
        string logName = BASE_PATH + "pages_for_excel\\page_" + to_string(page) + ".txt";
        FILE *log = fopen(logName.c_str(), "a");

        fprintf(log, "%lu %d\n", GetTickCount(), state);
        fclose(log);
    }
};

void openSemaphores() {
    string semaName;

    for (int i = 0; i < PAGES; ++i) {
        semaName = "semaphore_" + to_string(i) + "_writer";

        writersSemaphores[i] = OpenSemaphoreA(
                SEMAPHORE_ALL_ACCESS,
                FALSE,
                semaName.c_str()
        );

        semaName = "semaphore_" + to_string(i) + "_reader";

        readersSemaphores[i] = OpenSemaphoreA(
                SEMAPHORE_ALL_ACCESS,
                FALSE,
                semaName.c_str()
        );
    }
}

int main() {
    srand(time(NULL));

    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);
    int bufferSize = PAGES * systemInfo.dwPageSize;

    string baseBufferName = "buffer";

    LPVOID view = MapViewOfFile(
            OpenFileMappingA(GENERIC_READ, FALSE, baseBufferName.c_str()),
            FILE_MAP_READ,
            0,
            0,
            bufferSize
    );

    Logger Logger;

    openSemaphores();

    char *data;

    DWORD finishTime = GetTickCount() + 15000;

    while (GetTickCount() < finishTime) {
        Logger.logWaiting();

        DWORD page = WaitForMultipleObjects(
                PAGES,
                readersSemaphores,
                FALSE,
                INFINITE
        );

        Logger.logReading(page);

        data = (char *) ((long long) view + page * systemInfo.dwPageSize);

        Sleep(500 + rand() % 1001);

        Logger::appendGeneralLog(page, 2);

        ReleaseSemaphore(writersSemaphores[page], 1, NULL);

        Logger::appendGeneralLog(page, 0);

        Logger.logReleased(data);
    }

    return 0;
}
