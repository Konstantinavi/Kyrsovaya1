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
