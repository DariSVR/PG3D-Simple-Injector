#include <iostream>
#include <thread>
#include <chrono>
#include <string>
#include <Windows.h>
#include <TlHelp32.h>
#include <sys/stat.h>

#define PROCESS "Pixel Gun 3D.exe"
#define DLL_NAME "Test.dll"

// Define colors
#define COLOR_INFO 11    // Light cyan
#define COLOR_ERROR 12   // Red
#define COLOR_WARNING 14 // Yellow
#define COLOR_RESET 7    // White (default color)

void setConsoleColor(int color) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hConsole, color);
}

void log(const std::string& message) {
    setConsoleColor(COLOR_INFO);
    std::cout << "[INFO] " << message << std::endl;
    setConsoleColor(COLOR_RESET);
}

void logError(const std::string& message) {
    setConsoleColor(COLOR_ERROR);
    std::cerr << "[ERROR] " << message << std::endl;
    setConsoleColor(COLOR_RESET);
}

void logWarning(const std::string& message) {
    setConsoleColor(COLOR_WARNING);
    std::cerr << "[WARNING] " << message << std::endl;
    setConsoleColor(COLOR_RESET);
}

bool fileExists(const std::string& filename) {
    struct stat buffer;
    return (stat(filename.c_str(), &buffer) == 0);
}

DWORD GetProcessID(const std::string& processName) {
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    PROCESSENTRY32 pe;
    pe.dwSize = sizeof(PROCESSENTRY32);

    if (Process32First(hSnapshot, &pe)) {
        do {
            if (std::string(pe.szExeFile) == processName) {
                CloseHandle(hSnapshot);
                return pe.th32ProcessID;
            }
        } while (Process32Next(hSnapshot, &pe));
    }

    CloseHandle(hSnapshot);
    return 0;
}

void countdown(int seconds) {
    for (int i = seconds; i > 0; --i) {
        std::cout << "Closing in " << i << " seconds...\r";
        std::cout.flush();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout << std::endl;
}

void launchGame() {
    log("Launching Pixel Gun 3D via Steam...");
    system("start steam://rungameid/2524890");
}

int main()
{
    SetConsoleTitleA("PG3D Injector by Dari");

    // Get the injector's execution directory
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);

    std::string exeDir = exePath;
    size_t pos = exeDir.find_last_of("\\/");
    if (pos != std::string::npos) {
        exeDir = exeDir.substr(0, pos);
    }

    std::string dllPath = exeDir + "\\" + DLL_NAME;

    // Check if the DLL file exists
    if (!fileExists(dllPath)) {
        logError("DLL file not found: " + dllPath);
        countdown(10);
        return 1;
    }

    log("Checking if Pixel Gun 3D is already running...");

    DWORD procID = GetProcessID(PROCESS);

    // If the process is not running, launch the game
    if (procID == 0) {
        logWarning("Pixel Gun 3D not found. Launching the game...");
        launchGame();

        // Wait for the process to appear
        while (procID == 0) {
            procID = GetProcessID(PROCESS);
            if (procID == 0) {
                logWarning("Waiting for Pixel Gun 3D to start...");
                std::this_thread::sleep_for(std::chrono::seconds(2));
            }
        }
    }

    log("Pixel Gun 3D process found!");

    LPCSTR DllPath = dllPath.c_str();

    // Dynamically load functions
    HMODULE hKernel32 = GetModuleHandleA("Kernel32.dll");
    auto pOpenProcess = (HANDLE(WINAPI*)(DWORD, BOOL, DWORD))GetProcAddress(hKernel32, "OpenProcess");
    auto pVirtualAllocEx = (LPVOID(WINAPI*)(HANDLE, LPVOID, SIZE_T, DWORD, DWORD))GetProcAddress(hKernel32, "VirtualAllocEx");
    auto pWriteProcessMemory = (BOOL(WINAPI*)(HANDLE, LPVOID, LPCVOID, SIZE_T, SIZE_T*))GetProcAddress(hKernel32, "WriteProcessMemory");
    auto pCreateRemoteThread = (HANDLE(WINAPI*)(HANDLE, LPSECURITY_ATTRIBUTES, SIZE_T, LPTHREAD_START_ROUTINE, LPVOID, DWORD, LPDWORD))GetProcAddress(hKernel32, "CreateRemoteThread");
    auto pVirtualFreeEx = (BOOL(WINAPI*)(HANDLE, LPVOID, SIZE_T, DWORD))GetProcAddress(hKernel32, "VirtualFreeEx");

    // Open the process with full access
    HANDLE handle = pOpenProcess(PROCESS_ALL_ACCESS, FALSE, procID);
    if (handle == NULL) {
        logError("Error opening Pixel Gun 3D process.");
        countdown(10);
        return 1;
    }

    // Allocate memory in the remote process for the DLL path
    LPVOID pDllPath = pVirtualAllocEx(handle, 0, strlen(DllPath) + 1, MEM_COMMIT, PAGE_READWRITE);

    // Write the DLL path to the remote process memory
    pWriteProcessMemory(handle, pDllPath, (LPVOID)DllPath, strlen(DllPath) + 1, 0);

    // Create a remote thread that calls LoadLibraryA in the remote process
    HANDLE hLoadThread = pCreateRemoteThread(handle, 0, 0,
        (LPTHREAD_START_ROUTINE)GetProcAddress(hKernel32, "LoadLibraryA"), pDllPath, 0, 0);

    if (hLoadThread == NULL) {
        logError("Error creating remote thread in target process.");
        pVirtualFreeEx(handle, pDllPath, strlen(DllPath) + 1, MEM_RELEASE);
        CloseHandle(handle);
        countdown(10);
        return 1;
    }

    WaitForSingleObject(hLoadThread, INFINITE);

    pVirtualFreeEx(handle, pDllPath, 0, MEM_RELEASE);
    CloseHandle(hLoadThread);
    CloseHandle(handle);

    log("DLL successfully injected from: " + dllPath);

    // Countdown before closing the console window
    countdown(10);
    return 0;
}
