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
