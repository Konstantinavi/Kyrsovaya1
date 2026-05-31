#include "common.h"
RenderItem* renderItems = nullptr;
int renderItemsCount = 0;
int renderItemsCapacity = 0;
HANDLE hBuffers[2];
int currentBuffer = 0;

void InitDoubleBuffer(int width, int height) {
    for (int i = 0; i < 2; ++i) {
        hBuffers[i] = CreateConsoleScreenBuffer(
            GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE,
            NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
        COORD bufferSize = { static_cast<SHORT>(width), static_cast<SHORT>(height) };
        SetConsoleScreenBufferSize(hBuffers[i], bufferSize);
        SMALL_RECT windowSize = { 0, 0, static_cast<SHORT>(width - 1), static_cast<SHORT>(height - 1) };
        SetConsoleWindowInfo(hBuffers[i], TRUE, &windowSize);
        CONSOLE_CURSOR_INFO cursorInfo;
        GetConsoleCursorInfo(hBuffers[i], &cursorInfo);
        cursorInfo.bVisible = FALSE;
        SetConsoleCursorInfo(hBuffers[i], &cursorInfo);
    }
    SetConsoleActiveScreenBuffer(hBuffers[currentBuffer]);
}
void DrawLine(HANDLE hBuf, int row, const std::string& text, WORD attributes = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE) {
    if (row < 0 || row >= consoleHeight) return;
    std::string formatText = text;
    if (formatText.length() < static_cast<size_t>(consoleWidth)) {
        formatText.append(consoleWidth - formatText.length(), ' ');
    }
    else if (formatText.length() > static_cast<size_t>(consoleWidth)) {
        formatText = formatText.substr(0, consoleWidth);
    }
    COORD coord = { 0, static_cast<SHORT>(row) };
    DWORD written;
    WriteConsoleOutputCharacterA(hBuf, formatText.c_str(), static_cast<DWORD>(formatText.length()), coord, &written);
    FillConsoleOutputAttribute(hBuf, attributes, consoleWidth, coord, &written);
}
void BuildRenderItems() {
    renderItemsCount = 0;
    int appsCount = 0, bgCount = 0, winCount = 0;
    for (int i = 0; i < processesCount; i++) {
        if (processes[i].category == CAT_APP) appsCount++;
        else if (processes[i].category == CAT_BACKGROUND) bgCount++;
        else winCount++;
    }
    EnsureRenderItemsCapacity(renderItemsCount + 1);
    renderItems[renderItemsCount].isHeader = TRUE;
    renderItems[renderItemsCount].text = "--- Приложения (" + std::to_string(appsCount) + ") ---";
    renderItems[renderItemsCount].procIndex = -1;
    renderItemsCount++;
    for (int i = 0; i < processesCount; i++) {
        if (processes[i].category == CAT_APP) {
            EnsureRenderItemsCapacity(renderItemsCount + 1);
            renderItems[renderItemsCount].isHeader = FALSE;
            renderItems[renderItemsCount].procIndex = i;
            renderItemsCount++;
        }
    }
    EnsureRenderItemsCapacity(renderItemsCount + 1);
    renderItems[renderItemsCount].isHeader = TRUE;
    renderItems[renderItemsCount].text = "--- Фоновые процессы (" + std::to_string(bgCount) + ") ---";
    renderItems[renderItemsCount].procIndex = -1;
    renderItemsCount++;
    for (int i = 0; i < processesCount; i++) {
        if (processes[i].category == CAT_BACKGROUND) {
            EnsureRenderItemsCapacity(renderItemsCount + 1);
            renderItems[renderItemsCount].isHeader = FALSE;
            renderItems[renderItemsCount].procIndex = i;
            renderItemsCount++;
        }
    }
    EnsureRenderItemsCapacity(renderItemsCount + 1);
    renderItems[renderItemsCount].isHeader = TRUE;
    renderItems[renderItemsCount].text = "--- Процессы Windows (" + std::to_string(winCount) + ") ---";
    renderItems[renderItemsCount].procIndex = -1;
    renderItemsCount++;
    for (int i = 0; i < processesCount; i++) {
        if (processes[i].category == CAT_WINDOWS) {
            EnsureRenderItemsCapacity(renderItemsCount + 1);
            renderItems[renderItemsCount].isHeader = FALSE;
            renderItems[renderItemsCount].procIndex = i;
            renderItemsCount++;
        }
    }
}
void RenderScreen() {
    HANDLE hBuf = hBuffers[currentBuffer];
    int row = 0;
    WORD monoAttr = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    WORD monoIntensity = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
    DrawLine(hBuf, row++, "=================================== ДИСПЕТЧЕР ЗАДАЧ ===================================", monoAttr);
    DrawLine(hBuf, row++, "", monoAttr);
    char header[128];
    sprintf_s(header, "    %-7s %-18s %-7s %-12s %-11s %-11s | ПРОИЗВОДИТЕЛЬНОСТЬ:",
        "PID", "Имя процесса", "ЦП(%)", "Память(МБ)", "Диск(МБ/с)", "Сеть(Мб/с)");
    DrawLine(hBuf, row++, header, monoIntensity);
    DrawLine(hBuf, row++, std::string(consoleWidth, '-'), monoAttr);
    BuildRenderItems();
    UpdateSystemPerformanceData();
    std::string rightWidget[36];
    char buf[64];
    for (int i = 0; i < 36; i++) rightWidget[i] = "";
    rightWidget[0] = "ПРОЦЕССОР";
    sprintf_s(buf, "Загрузка ЦП:  %.1f%%", globalCpuLoad); rightWidget[1] = buf;
    sprintf_s(buf, "Ядра / Поток: %u / %u", sysHardware.cores, sysHardware.logicalProcessors); rightWidget[2] = buf;
    sprintf_s(buf, "Процессы:     %d", sysHardware.totalProcesses); rightWidget[3] = buf;
    sprintf_s(buf, "Потоки:       %d", sysHardware.totalThreads); rightWidget[4] = buf;
    sprintf_s(buf, "Дескрипторы:  %d", sysHardware.totalHandles); rightWidget[5] = buf;
    rightWidget[6] = "Uptime:       " + sysHardware.uptimeStr;
    rightWidget[7] = "--------------------";
    rightWidget[8] = "ПАМЯТЬ (RAM)";
    sprintf_s(buf, "Использовано: %.1f ГБ", sysHardware.ramUsedGB); rightWidget[9] = buf;
    sprintf_s(buf, "Доступно:     %.1f ГБ", sysHardware.ramFreeGB); rightWidget[10] = buf;
    sprintf_s(buf, "Всего в ПК:   %.1f ГБ", sysHardware.ramTotalGB); rightWidget[11] = buf;
    rightWidget[12] = "--------------------";
    rightWidget[13] = "ДИСК (C:)";
    double globalDiskTotalGB = 0.0, globalDiskFreeGB = 0.0;
    ULARGE_INTEGER fAvail, tBytes, tFree;
    if (GetDiskFreeSpaceExW(L"C:\\", &fAvail, &tBytes, &tFree)) {
        globalDiskTotalGB = tBytes.QuadPart / (1024.0 * 1024.0 * 1024.0);
        globalDiskFreeGB = tFree.QuadPart / (1024.0 * 1024.0 * 1024.0);
    }
    sprintf_s(buf, "Свободно:     %.1f ГБ", globalDiskFreeGB); rightWidget[14] = buf;
    sprintf_s(buf, "Емкость:      %.1f ГБ", globalDiskTotalGB); rightWidget[15] = buf;
    rightWidget[16] = "--------------------";
    rightWidget[17] = "СЕТЬ (АКТИВНОСТЬ)";
    sprintf_s(buf, "Общий поток:  %.2f Мбит/с", globalNetSpeed); rightWidget[18] = buf;
    rightWidget[19] = "--------------------";
    int appsCount = 0, bgCount = 0, winCount = 0;
    for (int i = 0; i < processesCount; i++) {
        if (processes[i].category == CAT_APP) appsCount++;
        else if (processes[i].category == CAT_BACKGROUND) bgCount++;
        else winCount++;
    }
    for (int lineIdx = 0; lineIdx < visibleRows; ++lineIdx) {
        int itemIdx = scrollOffset + lineIdx;
        std::string leftPart = "";
        if (itemIdx < renderItemsCount) {
            if (renderItems[itemIdx].isHeader) {
                leftPart = renderItems[itemIdx].text;
            }
            else {
                int pIdx = renderItems[itemIdx].procIndex;
                if (pIdx >= 0 && pIdx < processesCount) {
                    const ProcessInfo& proc = processes[pIdx];
                    char line[256];
                    char cpuStr[16], diskStr[16], netStr[16];
                    sprintf_s(cpuStr, "%.1f", proc.cpuUsage);
                    sprintf_s(diskStr, "%.1f", proc.diskUsage);
                    sprintf_s(netStr, "%.1f", proc.netUsage);

                    std::string name = WStringToString(proc.name);
                    if (name.length() > 18) name = name.substr(0, 15) + "...";

                    const char* prefix = (itemIdx == selectedIndex) ? "[X] " : "    ";
                    sprintf_s(line, "%s%-7d %-18s %-7s %-12s %-11s %-11s",
                        prefix, proc.pid, name.c_str(), cpuStr,
                        FormatMemory(proc.memoryUsage).c_str(), diskStr, netStr);
                    leftPart = line;
                }
            }
        }
        if (leftPart.length() < 80) leftPart.append(80 - leftPart.length(), ' ');
        else leftPart = leftPart.substr(0, 80);
        std::string rightPart = (lineIdx >= 0 && lineIdx < 36) ? rightWidget[lineIdx] : "";
        std::string fullLine = leftPart + " | " + rightPart;
        WORD lineAttr = monoAttr;
        if (lineIdx == 0 || lineIdx == 8 || lineIdx == 13 || lineIdx == 17) {
            lineAttr = monoIntensity;
        }

        DrawLine(hBuf, row++, fullLine, lineAttr);
    }
    row = 4 + visibleRows;
    DrawLine(hBuf, row++, std::string(consoleWidth, '='), monoAttr);

    if (selectedIndex >= 0 && selectedIndex < renderItemsCount && !renderItems[selectedIndex].isHeader) {
        int pIdx = renderItems[selectedIndex].procIndex;
        if (pIdx >= 0 && pIdx < processesCount) {
            char selectedInfo[512];

            // Проверяем флаг из нашей истории
            std::string statusStr = "АКТИВЕН";
            int idx = FindPidInCpuHistory(processes[pIdx].pid);
            if (idx != -1 && cpuHistory[idx].isSuspended) {
                statusStr = "ЗАМОРОЖЕН";
            }

            sprintf_s(selectedInfo, "ВЫБРАН ПРОЦЕСС: %s (PID: %d) | СТАТУС: [%s] | CPU: %.1f%% | Память: %s МБ | Диск: %.1f МБ/с",
                WStringToString(processes[pIdx].name).c_str(),
                processes[pIdx].pid,
                statusStr.c_str(),
                processes[pIdx].cpuUsage,
                FormatMemory(processes[pIdx].memoryUsage).c_str(),
                processes[pIdx].diskUsage);

            DrawLine(hBuf, row++, selectedInfo, monoIntensity);
        }
        else {
            DrawLine(hBuf, row++, "Ошибка получения данных процесса", monoAttr);
        }
    }
    char stats[256];
    sprintf_s(stats, "Всего процессов: %d | Приложений: %d | Фоновых: %d | Системных: %d",
        processesCount, appsCount, bgCount, winCount);
    DrawLine(hBuf, row++, stats, monoAttr);
    DrawLine(hBuf, row++, std::string(consoleWidth, '-'), monoAttr);
    DrawLine(hBuf, row++, "УПРАВЛЕНИЕ: ↑/↓ - Выбор | PgUp/PgDn - Скролл | K - Завершить процесс | Q/ESC - Выход", monoAttr);
    while (row < consoleHeight) DrawLine(hBuf, row++, "", monoAttr);
    SetConsoleActiveScreenBuffer(hBuffers[currentBuffer]);
    currentBuffer = (currentBuffer == 0) ? 1 : 0;
}
