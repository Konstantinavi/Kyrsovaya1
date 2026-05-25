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

int FindPidInCpuHistory(DWORD pid) {
    for (int i = 0; i < cpuHistoryCount; i++) if (cpuHistory[i].pid == pid) return i;
    return -1;
}

int AddPidToCpuHistory(DWORD pid) {
    if (cpuHistoryCount >= cpuHistoryCapacity) {
        int newCap = (cpuHistoryCapacity == 0) ? 100 : cpuHistoryCapacity * 2;
        ProcessCpuTime* newArr = new ProcessCpuTime[newCap];
        for (int i = 0; i < cpuHistoryCount; i++) newArr[i] = cpuHistory[i];
        delete[] cpuHistory;
        cpuHistory = newArr;
        cpuHistoryCapacity = newCap;
    }
    cpuHistory[cpuHistoryCount].pid = pid;
    cpuHistory[cpuHistoryCount].lastKernel = 0;
    cpuHistory[cpuHistoryCount].lastUser = 0;
    cpuHistory[cpuHistoryCount].isActive = TRUE;
    return cpuHistoryCount++;
}

double GetProcessCpu(DWORD pid, unsigned long long curKernel, unsigned long long curUser, unsigned long long sysDelta) {
    int idx = FindPidInCpuHistory(pid);
    if (idx != -1) {
        unsigned long long prevTotal = cpuHistory[idx].lastKernel + cpuHistory[idx].lastUser;
        unsigned long long curTotal = curKernel + curUser;
        if (curTotal >= prevTotal && sysDelta > 0) {
            double percent = static_cast<double>((curTotal - prevTotal) * 100) / static_cast<double>(sysDelta);
            cpuHistory[idx].lastKernel = curKernel;
            cpuHistory[idx].lastUser = curUser;
            return (percent > 100) ? 100 : (percent < 0 ? 0 : percent);
        }
    }
    else {
        int newIdx = AddPidToCpuHistory(pid);
        if (newIdx != -1) {
            cpuHistory[newIdx].lastKernel = curKernel;
            cpuHistory[newIdx].lastUser = curUser;
        }
    }
    return 0.0;
}

int FindPidInDiskHistory(DWORD pid) {
    for (int i = 0; i < diskHistoryCount; i++) if (diskHistory[i].pid == pid) return i;
    return -1;
}

int AddPidToDiskHistory(DWORD pid) {
    if (diskHistoryCount >= diskHistoryCapacity) {
        int newCap = (diskHistoryCapacity == 0) ? 100 : diskHistoryCapacity * 2;
        DiskStats* newArr = new DiskStats[newCap];
        for (int i = 0; i < diskHistoryCount; i++) newArr[i] = diskHistory[i];
        delete[] diskHistory;
        diskHistory = newArr;
        diskHistoryCapacity = newCap;
    }
    diskHistory[diskHistoryCount].pid = pid;
    diskHistory[diskHistoryCount].lastRead = 0;
    diskHistory[diskHistoryCount].lastWrite = 0;
    diskHistory[diskHistoryCount].lastTime = GetTickCount64();
    return diskHistoryCount++;
}

double GetProcessDisk(DWORD pid, unsigned long long readBytes, unsigned long long writeBytes) {
    unsigned long long now = GetTickCount64();
    int idx = FindPidInDiskHistory(pid);
    if (idx != -1) {
        unsigned long long deltaTime = now - diskHistory[idx].lastTime;
        if (deltaTime > 0) {
            double mbps = static_cast<double>((readBytes - diskHistory[idx].lastRead) +
                (writeBytes - diskHistory[idx].lastWrite)) / (1048576.0 * (deltaTime / 1000.0));
            diskHistory[idx].lastRead = readBytes;
            diskHistory[idx].lastWrite = writeBytes;
            diskHistory[idx].lastTime = now;
            return (mbps < 0) ? 0 : mbps;
        }
    }
    else {
        int newIdx = AddPidToDiskHistory(pid);
        if (newIdx != -1) {
            diskHistory[newIdx].lastRead = readBytes;
            diskHistory[newIdx].lastWrite = writeBytes;
            diskHistory[newIdx].lastTime = now;
        }
    }
    return 0.0;
}

double GetRealNetworkUsage(ProcessInfo& proc, const IO_COUNTERS& ioCounters, unsigned long long currentTime) {
    unsigned long long currentNetBytes = ioCounters.OtherTransferCount;
    if (proc.lastUpdateTime != 0 && currentTime > proc.lastUpdateTime) {
        unsigned long long timeDelta = currentTime - proc.lastUpdateTime;
        unsigned long long netDiff = currentNetBytes - proc.lastNetBytes;
        double speedMbps = (static_cast<double>(netDiff) * 8.0) / (1000000.0 * (timeDelta / 1000.0));
        proc.lastNetBytes = currentNetBytes;
        return (speedMbps < 0) ? 0 : speedMbps;
    }
    proc.lastNetBytes = currentNetBytes;
    return 0.0;
}
