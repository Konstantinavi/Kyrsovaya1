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
