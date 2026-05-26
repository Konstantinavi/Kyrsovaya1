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
void DrawLine(HANDLE hBuf, INT row, const std::string& text, WORD attributes) {
    if (row < 0 || row >= consoleHeight) return;
    
    COORD coord = { 0, (SHORT)row };
    DWORD written;
    WriteConsoleOutputCharacterA(hBuf, text.c_str(), (DWORD)text.length(), coord, &written);
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
