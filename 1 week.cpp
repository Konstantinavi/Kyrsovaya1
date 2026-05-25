void ClearGuiArray() {
    delete[] guiPids;
    guiPids = nullptr;
    guiPidsCount = 0;
    guiPidsCapacity = 0;
}

void EnsureGuiPidsCapacity(INT needed) {
    if (needed <= guiPidsCapacity) return;
    INT newCapacity = (guiPidsCapacity == 0) ? 100 : guiPidsCapacity * 2;
    while (newCapacity < needed) newCapacity *= 2;
    DWORD* newArray = new DWORD[newCapacity];
    for (INT i = 0; i < guiPidsCount; i++) newArray[i] = guiPids[i];
    delete[] guiPids;
    guiPids = newArray;
    guiPidsCapacity = newCapacity;
}

void AddPidToGuiArray(DWORD pid) {
    for (INT i = 0; i < guiPidsCount; i++) if (guiPids[i] == pid) return;
    EnsureGuiPidsCapacity(guiPidsCount + 1);
    guiPids[guiPidsCount++] = pid;
}

BOOL IsPidInGuiArray(DWORD pid) {
    for (INT i = 0; i < guiPidsCount; i++) if (guiPids[i] == pid) return TRUE;
    return FALSE;
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
    if (IsWindowVisible(hwnd) && GetWindow(hwnd, GW_OWNER) == NULL) {
        DWORD pid;
        GetWindowThreadProcessId(hwnd, &pid);
        AddPidToGuiArray(pid);
    }
    return TRUE;
}

void RefreshGuiProcessesList() {
    ClearGuiArray();
    EnumWindows(EnumWindowsProc, 0);
}
void ClearProcessesData() {
    if (processes != nullptr) {
        for (INT i = 0; i < processesCount; i++) {
            delete[] processes[i].name;
            processes[i].name = nullptr;
        }
    }
    processesCount = 0;
}

void EnsureProcessesCapacity(INT needed) {
    if (needed <= processesCapacity) return;
    INT newCapacity = (processesCapacity == 0) ? 100 : processesCapacity * 2;
    while (newCapacity < needed) newCapacity *= 2;

    ProcessInfo* newArray = new ProcessInfo[newCapacity]();
    for (INT i = 0; i < processesCount; i++) {
        newArray[i] = processes[i];
    }
    delete[] processes;
    processes = newArray;
    processesCapacity = newCapacity;
}

void EnsureRenderItemsCapacity(INT needed) {
    if (needed <= renderItemsCapacity) return;
    INT newCapacity = (renderItemsCapacity == 0) ? 100 : renderItemsCapacity * 2;
    while (newCapacity < needed) newCapacity *= 2;
    RenderItem* newArray = new RenderItem[newCapacity]();
    for (INT i = 0; i < renderItemsCount; i++) newArray[i] = renderItems[i];
    delete[] renderItems;
    renderItems = newArray;
    renderItemsCapacity = newCapacity;
}

void FreeProcessesArray() {
    if (processes != nullptr) {
        for (INT i = 0; i < processesCount; i++) {
            delete[] processes[i].name;
        }
        delete[] processes;
        processes = nullptr;
        processesCount = 0;
        processesCapacity = 0;
    }
}
INT GetProcesses() {
    RefreshGuiProcessesList();
    ClearProcessesData();

    FILETIME sysIdle, sysKernel, sysUser;
    static ULONGLONG lastSysKernel = 0, lastSysUser = 0;
    ULONGLONG globalTimeDelta = 0;

    if (GetSystemTimes(&sysIdle, &sysKernel, &sysUser)) {
        ULONGLONG currentSysKernel = FileTimeToQuadWord(&sysKernel);
        ULONGLONG currentSysUser = FileTimeToQuadWord(&sysUser);
        globalTimeDelta = (currentSysKernel - lastSysKernel) + (currentSysUser - lastSysUser);

        ULONGLONG currentSysIdle = FileTimeToQuadWord(&sysIdle);
        static ULONGLONG lastSysIdle = 0;
        if (lastSysIdle != 0 && globalTimeDelta > 0) {
            ULONGLONG idleDelta = currentSysIdle - lastSysIdle;
            globalCpuLoad = 100.0 - (idleDelta * 100.0 / globalTimeDelta);
            if (globalCpuLoad < 0) globalCpuLoad = 0;
            if (globalCpuLoad > 100) globalCpuLoad = 100;
        }
        lastSysIdle = currentSysIdle;
        lastSysKernel = currentSysKernel;
        lastSysUser = currentSysUser;
    }
    if (globalTimeDelta == 0) globalTimeDelta = 1;

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) return 0;

    PROCESSENTRY32W pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32W);
    globalNetSpeed = 0.0;
    globalTotalThreads = 0;
    globalTotalHandles = 0;

    if (Process32FirstW(hSnapshot, &pe32)) {
        do {
            if (pe32.th32ProcessID == 0) continue;
            EnsureProcessesCapacity(processesCount + 1);

            ProcessInfo& proc = processes[processesCount];
            proc.pid = pe32.th32ProcessID;

            SIZE_T nameLength = wcslen(pe32.szExeFile) + 1;
            proc.name = new WCHAR[nameLength];
            wcscpy_s(proc.name, nameLength, pe32.szExeFile);

            proc.category = DetectProcessCategory(pe32.th32ProcessID, pe32.szExeFile);
            proc.cpuUsage = 0.0;
            proc.memoryUsage = 0;
            proc.diskUsage = 0.0;
            proc.netUsage = 0.0;
            proc.lastUpdateTime = GetTickCount64();

            globalTotalThreads += pe32.cntThreads;

            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);
            if (hProcess != NULL) {
                PROCESS_MEMORY_COUNTERS pmc;
                if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
                    proc.memoryUsage = static_cast<DWORD>(pmc.WorkingSetSize);
                }
                FILETIME creationTime, exitTime, kernelTime, userTime;
                if (GetProcessTimes(hProcess, &creationTime, &exitTime, &kernelTime, &userTime)) {
                    ULONGLONG procKernel = FileTimeToQuadWord(&kernelTime);
                    ULONGLONG procUser = FileTimeToQuadWord(&userTime);
                    proc.cpuUsage = GetProcessCpu(pe32.th32ProcessID, procKernel, procUser, globalTimeDelta);
                }
                IO_COUNTERS io;
                if (GetProcessIoCounters(hProcess, &io)) {
                    proc.diskUsage = GetProcessDisk(pe32.th32ProcessID, io.ReadTransferCount, io.WriteTransferCount);
                    proc.netUsage = GetRealNetworkUsage(proc, io, proc.lastUpdateTime);
                    globalNetSpeed += proc.netUsage;
                }
                DWORD handleCount = 0;
                if (GetProcessHandleCount(hProcess, &handleCount)) {
                    globalTotalHandles += handleCount;
                }
                CloseHandle(hProcess);
            }
            processesCount++;
        } while (Process32NextW(hSnapshot, &pe32));
    }
    CloseHandle(hSnapshot);

    globalSystemUptime = GetTickCount64() / 1000;

    for (INT i = 0; i < processesCount - 1; i++) {
        for (INT j = i + 1; j < processesCount; j++) {
            if (processes[i].category > processes[j].category ||
                (processes[i].category == processes[j].category && processes[i].pid > processes[j].pid)) {
                std::swap(processes[i], processes[j]);
            }
        }
    }
    return processesCount;
}
