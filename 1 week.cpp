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
