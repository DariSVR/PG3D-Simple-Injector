#include <cstdio>
#include <Windows.h>
#include <TlHelp32.h>

// Constants for process and DLL names
static const char* PROCESS = "Pixel Gun 3D.exe";
static const char* DLL_NAME = "Test.dll";

// Enum for console text colors
enum class ConsoleColor : WORD { Info = 11, Error = 12, Warning = 14, Reset = 7 };

// Sets the console text color
static void setConsoleColor(ConsoleColor color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), static_cast<WORD>(color));
}

// Logging function template for colored messages
template<ConsoleColor Color>
static void log(const char* message) {
    setConsoleColor(Color);
    const char* prefix = (Color == ConsoleColor::Info ? "INFO" : (Color == ConsoleColor::Error ? "ERROR" : "WARNING"));
    printf("[%s] %s\n", prefix, message);
    setConsoleColor(ConsoleColor::Reset);
}

// Checks if the DLL file exists in the current directory
static bool fileExists(const char* filePath) {
    return GetFileAttributesA(filePath) != INVALID_FILE_ATTRIBUTES;
}

// Launches the game via Steam
static void launchGame() {
    log<ConsoleColor::Info>("Launching Pixel Gun 3D via Steam...");
    ShellExecuteA(NULL, "open", "steam://rungameid/2524890", NULL, NULL, SW_SHOWNORMAL);
}

// Injects a DLL into the target process using QueueUserAPC
static bool injectDLL(const char* processName, const char* dllPath) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS | TH32CS_SNAPTHREAD, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return false;

    PROCESSENTRY32 pe{ sizeof(pe) };
    THREADENTRY32 te{ sizeof(te) };
    DWORD processId = 0;
    DWORD threadId = 0;

    if (Process32First(hSnapshot, &pe)) {
        do {
            if (_stricmp(pe.szExeFile, processName) == 0) {
                processId = pe.th32ProcessID;
                break;
            }
        } while (Process32Next(hSnapshot, &pe));
    }

    if (processId == 0) {
        CloseHandle(hSnapshot);
        return false;
    }

    if (Thread32First(hSnapshot, &te)) {
        do {
            if (te.th32OwnerProcessID == processId) {
                threadId = te.th32ThreadID;
                break;
            }
        } while (Thread32Next(hSnapshot, &te));
    }

    CloseHandle(hSnapshot);

    if (threadId == 0) return false;

    // Open the process with minimal permissions
    HANDLE hProcess = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_QUERY_INFORMATION, FALSE, processId);
    if (!hProcess) return false;

    LPVOID pDllPath = VirtualAllocEx(hProcess, nullptr, strlen(dllPath) + 1, MEM_COMMIT, PAGE_READWRITE);
    if (!pDllPath) {
        CloseHandle(hProcess);
        return false;
    }

    if (!WriteProcessMemory(hProcess, pDllPath, dllPath, strlen(dllPath) + 1, nullptr)) {
        VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    LPTHREAD_START_ROUTINE loadLibraryAddr = reinterpret_cast<LPTHREAD_START_ROUTINE>(GetProcAddress(GetModuleHandleA("Kernel32.dll"), "LoadLibraryA"));
    if (!loadLibraryAddr) {
        VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    // Open the thread with minimal permissions
    HANDLE hThread = OpenThread(THREAD_SET_CONTEXT | THREAD_QUERY_INFORMATION, FALSE, threadId);
    if (!hThread) {
        VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return false;
    }

    if (QueueUserAPC(reinterpret_cast<PAPCFUNC>(loadLibraryAddr), hThread, reinterpret_cast<ULONG_PTR>(pDllPath)) == 0) {
        VirtualFreeEx(hProcess, pDllPath, 0, MEM_RELEASE);
        CloseHandle(hThread);
        CloseHandle(hProcess);
        return false;
    }

    CloseHandle(hThread);
    CloseHandle(hProcess);
    return true;
}

// Displays a countdown timer before the program exits
static void countdown(int seconds) {
    for (int i = seconds; i > 0; --i) {
        printf("\rClosing in %d seconds...   ", i);
        fflush(stdout);
        Sleep(1000);
    }
    printf("\n");
}

int main() {
    SetConsoleTitleA("PG3D Simple DLL Injector");

    char dllPath[MAX_PATH];
    GetCurrentDirectoryA(MAX_PATH, dllPath);
    strcat_s(dllPath, "\\");
    strcat_s(dllPath, DLL_NAME);

    if (!fileExists(dllPath)) {
        log<ConsoleColor::Error>(DLL_NAME);
        log<ConsoleColor::Warning>("DLL file not found.");
        log<ConsoleColor::Info>("Ensure that the DLL is in the same folder as the injector.");
        countdown(10);
        return 1;
    }

    log<ConsoleColor::Info>("Checking if Pixel Gun 3D is running...");

    if (!injectDLL(PROCESS, dllPath)) {
        log<ConsoleColor::Warning>("Pixel Gun 3D not found. Launching the game...");
        launchGame();

        while (!injectDLL(PROCESS, dllPath)) {
            log<ConsoleColor::Warning>("Waiting for Pixel Gun 3D to start...");
            Sleep(2000);
        }
    }

    log<ConsoleColor::Info>("DLL injected successfully.");
    countdown(10);
    return 0;
}
