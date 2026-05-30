include 'common.h'

void SuspendSelectedProcess() {
    if (selectedIndex < 0 || selectedIndex >= renderItemsCount || renderItems[selectedIndex].isHeader) return;

    int pIdx = renderItems[selectedIndex].procIndex;
    if (pIdx >= 0 && pIdx < processesCount) {
        DWORD pid = processes[pIdx].pid;

        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
        if (hSnapshot != INVALID_HANDLE_VALUE) {
            THREADENTRY32 te;
            te.dwSize = sizeof(THREADENTRY32);

            if (Thread32First(hSnapshot, &te)) {
                do {
                    if (te.th32OwnerProcessID == pid) {
                        HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te.th32ThreadID);
                        if (hThread != NULL) {
                            SuspendThread(hThread);
                            CloseHandle(hThread);
                        }
                    }
                } while (Thread32Next(hSnapshot, &te));
            }
            CloseHandle(hSnapshot);

            int idx = FindPidInCpuHistory(pid);
            if (idx != -1) cpuHistory[idx].isSuspended = TRUE;
            Sleep(200);
        }
    }
}
void ResumeSelectedProcess() {
    if (selectedIndex < 0 || selectedIndex >= renderItemsCount || renderItems[selectedIndex].isHeader) return;

    int pIdx = renderItems[selectedIndex].procIndex;
    if (pIdx >= 0 && pIdx < processesCount) {
        DWORD pid = processes[pIdx].pid;

        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
        if (hSnapshot != INVALID_HANDLE_VALUE) {
            THREADENTRY32 te;
            te.dwSize = sizeof(THREADENTRY32);

            if (Thread32First(hSnapshot, &te)) {
                do {
                    if (te.th32OwnerProcessID == pid) {
                        HANDLE hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, te.th32ThreadID);
                        if (hThread != NULL) {
                            ResumeThread(hThread);
                            CloseHandle(hThread);
                        }
                    }
                } while (Thread32Next(hSnapshot, &te));
            }
            CloseHandle(hSnapshot);

            int idx = FindPidInCpuHistory(pid);
            if (idx != -1) cpuHistory[idx].isSuspended = FALSE;
            Sleep(200);
        }
    }
}
