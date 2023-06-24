#include <Windows.h>
#include <string>
#include <TlHelp32.h>

class ProcessTerminator {
public:
    static void TerminateProcessByName(const std::string& processName) {
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot != INVALID_HANDLE_VALUE) {
            PROCESSENTRY32 processEntry;
            processEntry.dwSize = sizeof(PROCESSENTRY32);

            if (Process32First(hSnapshot, &processEntry)) {
                do {
                    std::string currentProcessName = processEntry.szExeFile;
                    if (currentProcessName == processName) {
                        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processEntry.th32ProcessID);
                        if (hProcess != NULL) {
                            TerminateProcess(hProcess, 0);
                            CloseHandle(hProcess);
                            break;
                        }
                    }
                } while (Process32Next(hSnapshot, &processEntry));
            }

            CloseHandle(hSnapshot);
        }
    }
};