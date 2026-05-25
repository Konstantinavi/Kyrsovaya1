#pragma once
#include <windows.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <algorithm>
#include <string>

enum ProcessCategory { 
    CAT_APP = 0, 
    CAT_BACKGROUND, 
    CAT_WINDOWS 
};

struct SystemHardware {
    UINT cores;
    UINT logicalProcessors;
    DOUBLE ramTotalGB;
    DOUBLE ramUsedGB;
    DOUBLE ramFreeGB;
    INT totalProcesses;
    INT totalThreads;
    INT totalHandles;
    std::string uptimeStr;
};

struct ProcessInfo {
    DWORD pid;
    WCHAR* name;
    DOUBLE cpuUsage;
    DWORD memoryUsage;
    DOUBLE diskUsage;
    DOUBLE netUsage;
    ProcessCategory category;
    ULONGLONG lastReadBytes;
    ULONGLONG lastWriteBytes;
    ULONGLONG lastNetBytes;
    ULONGLONG lastUpdateTime;
};

struct RenderItem {
    BOOL isHeader;
    std::string text;
    INT procIndex;
};

struct ProcessCpuTime { 
    DWORD pid; 
    ULONGLONG lastKernel; 
    ULONGLONG lastUser; 
    BOOL isActive; 
};

struct DiskStats { 
    DWORD pid; 
    ULONGLONG lastRead; 
    ULONGLONG lastWrite; 
    ULONGLONG lastTime; 
};
