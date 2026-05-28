Переменные:
int selectedIndex = 1;
int scrollOffset = 0;

const int visibleRows = 30;
const int consoleWidth = 120;
const int consoleHeight = 50;

Код:
DWORD GetSelectedProcessPid() {
    if (selectedIndex >= 0 && selectedIndex < renderItemsCount && !renderItems[selectedIndex].isHeader) {
        int pIdx = renderItems[selectedIndex].procIndex;
        if (pIdx >= 0 && pIdx < processesCount) {
            return processes[pIdx].pid;
        }
    }
    return 0;
}
void KillSelectedProcess() {
    if (selectedIndex < 0 || selectedIndex >= renderItemsCount) return;
    if (renderItems[selectedIndex].isHeader) return;

    int pIdx = renderItems[selectedIndex].procIndex;
    if (pIdx >= 0 && pIdx < processesCount) {
        HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processes[pIdx].pid);
        if (hProcess != NULL) {
            TerminateProcess(hProcess, 0);
            CloseHandle(hProcess);
            Sleep(200);
        }
    }
}
DWORD GetSelectedProcessPid() {
    if (selectedIndex >= 0 && selectedIndex < renderItemsCount && !renderItems[selectedIndex].isHeader) {
        int pIdx = renderItems[selectedIndex].procIndex;
        if (pIdx >= 0 && pIdx < processesCount) {
            return processes[pIdx].pid;
        }
    }
    return 0;
}
void RestoreSelectionByPid(DWORD targetPid) {
    if (targetPid == 0) return;
    BOOL found = FALSE;
    for (int i = 0; i < renderItemsCount; i++) {
        if (!renderItems[i].isHeader) {
            int pIdx = renderItems[i].procIndex;
            if (pIdx >= 0 && pIdx < processesCount && processes[pIdx].pid == targetPid) {
                selectedIndex = i;
                found = TRUE;
                break;
            }
        }
    }
    if (!found) {
        if (selectedIndex >= renderItemsCount) {
            selectedIndex = renderItemsCount - 1;
        }
        if (selectedIndex < 1) {
            selectedIndex = 1;
        }
        if (selectedIndex < renderItemsCount && renderItems[selectedIndex].isHeader && selectedIndex < renderItemsCount - 1) {
            selectedIndex++;
        }
    }
}

void UpdateScrollLimits() {
    if (selectedIndex >= renderItemsCount) selectedIndex = renderItemsCount - 1;
    if (selectedIndex < 0) selectedIndex = 0;

    if (selectedIndex < scrollOffset) {
        scrollOffset = selectedIndex;
    }
    if (scrollOffset + visibleRows > renderItemsCount) {
        scrollOffset = renderItemsCount - visibleRows;
    }
    if (scrollOffset < 0) {
        scrollOffset = 0;
    }
    if (selectedIndex >= scrollOffset + visibleRows) {
        scrollOffset = selectedIndex - visibleRows + 1;
    }
}
