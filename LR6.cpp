#include <iostream>
#include <windows.h>
#include <tchar.h>
#include <string>

void PrintSystemInfo() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);

    std::cout << "-----System Information\n";
    std::cout << "Processor Architecture: ";
    switch (sysInfo.wProcessorArchitecture) {
    case PROCESSOR_ARCHITECTURE_AMD64: std::cout << "x64"; break;
    case PROCESSOR_ARCHITECTURE_INTEL: std::cout << "x86"; break;
    case PROCESSOR_ARCHITECTURE_ARM: std::cout << "ARM"; break;
    default: std::cout << "Unknown"; break;
    }
    std::cout << "\nNumber of Processors: " << sysInfo.dwNumberOfProcessors << "\n";
}

void PrintOSVersion() {
    HKEY hKey;
    LONG lRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"), 0, KEY_READ, &hKey);

    if (lRes == ERROR_SUCCESS) {
        DWORD dwMajorVersion = 0, dwMinorVersion = 0;
        DWORD dwBufferSize = sizeof(DWORD);

        if (RegQueryValueEx(hKey, TEXT("CurrentMajorVersionNumber"), NULL, NULL, (LPBYTE)&dwMajorVersion, &dwBufferSize) == ERROR_SUCCESS &&
            RegQueryValueEx(hKey, TEXT("CurrentMinorVersionNumber"), NULL, NULL, (LPBYTE)&dwMinorVersion, &dwBufferSize) == ERROR_SUCCESS) {
            std::cout << "\nOS Information\n";
            std::cout << "Operating System: Windows\n";
            std::cout << "Version: " << dwMajorVersion << "." << dwMinorVersion;

            TCHAR szBuildNumber[256];
            dwBufferSize = sizeof(szBuildNumber);
            if (RegQueryValueEx(hKey, TEXT("CurrentBuildNumber"), NULL, NULL, (LPBYTE)szBuildNumber, &dwBufferSize) == ERROR_SUCCESS) {
                std::wcout << L" (Build " << szBuildNumber << L")\n";
            }
        }
        else {
            std::cerr << "Failed to retrieve OS version numbers from the registry." << std::endl;
        }

        RegCloseKey(hKey);
    }
    else {
        std::cerr << "Failed to open registry key for OS version." << std::endl;
    }
}



void PrintRegistryInfo() {
    HKEY hKey;
    LONG lRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"), 0, KEY_READ, &hKey);

    if (lRes == ERROR_SUCCESS) {
        std::cout << "\n-------Registry Information\n";

        TCHAR szProductName[256];
        DWORD dwBufferSize = sizeof(szProductName);

        if (RegQueryValueEx(hKey, TEXT("ProductName"), NULL, NULL, (LPBYTE)szProductName, &dwBufferSize) == ERROR_SUCCESS) {
            std::wcout << L"Product Name: " << szProductName << std::endl;
        }

        TCHAR szReleaseId[256];
        dwBufferSize = sizeof(szReleaseId);

        if (RegQueryValueEx(hKey, TEXT("ReleaseId"), NULL, NULL, (LPBYTE)szReleaseId, &dwBufferSize) == ERROR_SUCCESS) {
            std::wcout << L"Release ID: " << szReleaseId << std::endl;
        }

        TCHAR szCurrentBuild[256];
        dwBufferSize = sizeof(szCurrentBuild);

        if (RegQueryValueEx(hKey, TEXT("CurrentBuild"), NULL, NULL, (LPBYTE)szCurrentBuild, &dwBufferSize) == ERROR_SUCCESS) {
            std::wcout << L"Current Build: " << szCurrentBuild << std::endl;
        }

        RegCloseKey(hKey);
    }
    else {
        std::cerr << "Failed to open registry key." << std::endl;
    }
}

void PrintMemoryStatus() {
    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(MEMORYSTATUSEX);

    if (GlobalMemoryStatusEx(&memStatus)) {
        std::cout << "\n---------Memory Information:\n";
        std::cout << "Total Physical Memory: " << memStatus.ullTotalPhys / (1024 * 1024) << " MB\n";
        std::cout << "Available Physical Memory: " << memStatus.ullAvailPhys / (1024 * 1024) << " MB\n";
        std::cout << "Memory Load: " << memStatus.dwMemoryLoad << " %\n";
    }
    else {
        std::cerr << "Failed to retrieve memory status." << std::endl;
    }
}

void PrintDiskSpace(const std::wstring& drive) {
    ULARGE_INTEGER freeBytesAvailable, totalBytes, totalFreeBytes;

    if (GetDiskFreeSpaceEx(drive.c_str(), &freeBytesAvailable, &totalBytes, &totalFreeBytes)) {
        std::wcout << L"\n-------Disk Space Information:\n";
        std::wcout << L"Drive: " << drive << std::endl;
        std::wcout << L"Total Space: " << totalBytes.QuadPart / (1024 * 1024 * 1024) << L" GB\n";
        std::wcout << L"Free Space: " << totalFreeBytes.QuadPart / (1024 * 1024 * 1024) << L" GB\n";
        std::wcout << L"Available Space to Current User: " << freeBytesAvailable.QuadPart / (1024 * 1024 * 1024) << L" GB\n";
    }
    else {
        std::cerr << "Failed to retrieve disk space information for drive " << std::string(drive.begin(), drive.end()) << ".\n";
    }
}



int main() {
    PrintSystemInfo();
    PrintOSVersion();
    PrintRegistryInfo();
    PrintMemoryStatus();
    PrintDiskSpace(L"C:\\");

    return 0;
}
