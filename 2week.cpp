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
