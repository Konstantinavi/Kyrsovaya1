#include 'common.h'

void GetProcessorTopology() {
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    systemLogicalProcs = sysInfo.dwNumberOfProcessors;

    DWORD length = 0;
    GetLogicalProcessorInformation(nullptr, &length);
    if (length > 0) {
        PSYSTEM_LOGICAL_PROCESSOR_INFORMATION buffer = (PSYSTEM_LOGICAL_PROCESSOR_INFORMATION)malloc(length);
        if (buffer && GetLogicalProcessorInformation(buffer, &length)) {
            int cores = 0;
            int relationCoreCount = length / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
            for (int i = 0; i < relationCoreCount; i++) {
                if (buffer[i].Relationship == RelationProcessorCore) {
                    cores++;
                }
            }
            systemCores = (cores > 0) ? cores : systemLogicalProcs;
        }
        free(buffer);
    }
    if (systemCores == 0) systemCores = systemLogicalProcs;
}

void InitStaticHardwareInfo() {
    GetProcessorTopology();
    sysHardware.cores = systemCores;
    sysHardware.logicalProcessors = systemLogicalProcs;

    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(memStatus);
    if (GlobalMemoryStatusEx(&memStatus)) {
        sysHardware.ramTotalGB = memStatus.ullTotalPhys / (1024.0 * 1024.0 * 1024.0);
    }
}

void UpdateSystemPerformanceData() {
    sysHardware.totalProcesses = processesCount;
    sysHardware.totalThreads = globalTotalThreads;
    sysHardware.totalHandles = globalTotalHandles;

    unsigned long long systemUptime = GetTickCount64() / 1000;
    DWORD days = static_cast<DWORD>(systemUptime / 86400);
    DWORD hours = static_cast<DWORD>((systemUptime % 86400) / 3600);
    DWORD minutes = static_cast<DWORD>((systemUptime % 3600) / 60);
    DWORD seconds = static_cast<DWORD>(systemUptime % 60);

    char uptimeBuffer[64];
    sprintf_s(uptimeBuffer, "%lu:%02lu:%02lu:%02lu", days, hours, minutes, seconds);
    sysHardware.uptimeStr = uptimeBuffer;

    MEMORYSTATUSEX memStatus;
    memStatus.dwLength = sizeof(memStatus);
    if (GlobalMemoryStatusEx(&memStatus)) {
        sysHardware.ramTotalGB = memStatus.ullTotalPhys / (1024.0 * 1024.0 * 1024.0);
        sysHardware.ramUsedGB = (memStatus.ullTotalPhys - memStatus.ullAvailPhys) / (1024.0 * 1024.0 * 1024.0);
        sysHardware.ramFreeGB = memStatus.ullAvailPhys / (1024.0 * 1024.0 * 1024.0);
    }
}
    
ProcessCategory DetectProcessCategory(DWORD pid, const WCHAR* name) {
    if (_wcsicmp(name, L"svchost.exe") == 0 || _wcsicmp(name, L"csrss.exe") == 0 ||
        _wcsicmp(name, L"explorer.exe") == 0 || _wcsicmp(name, L"services.exe") == 0 ||
        _wcsicmp(name, L"lsass.exe") == 0 || _wcsicmp(name, L"wininit.exe") == 0 ||
        _wcsicmp(name, L"winlogon.exe") == 0 || _wcsicmp(name, L"dwm.exe") == 0) {
        return CAT_WINDOWS;
    }
    if (IsPidInGuiArray(pid)) return CAT_APP;
    return CAT_BACKGROUND;
}

unsigned long long FileTimeToQuadWord(const FILETIME* ft) {
    if (ft == nullptr) return 0;
    ULARGE_INTEGER uli;
    uli.LowPart = ft->dwLowDateTime;
    uli.HighPart = ft->dwHighDateTime;
    return uli.QuadPart;
}

std::string WStringToString(const WCHAR* wstr) {
    if (!wstr) return "";
    int size = WideCharToMultiByte(1251, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
    if (size <= 0) return "";
    std::string result(size - 1, 0);
    WideCharToMultiByte(1251, 0, wstr, -1, &result[0], size, nullptr, nullptr);
    return result;
}

std::string FormatMemory(DWORD bytes) {
    double mb = bytes / (1024.0 * 1024.0);
    char buffer[32];
    sprintf_s(buffer, "%.1f", mb);
    return std::string(buffer);
}
