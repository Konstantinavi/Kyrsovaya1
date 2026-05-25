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

ProcessCpuTime* cpuHistory = nullptr;  
int cpuHistoryCount = 0;
int cpuHistoryCapacity = 0;

DiskStats* diskHistory = nullptr;  
int diskHistoryCount = 0;
int diskHistoryCapacity = 0;

DWORD* guiPids = nullptr;     
int guiPidsCapacity = 0;
int guiPidsCount = 0;

ProcessInfo* processes = nullptr;  
int processesCount = 0;
int processesCapacity = 0;

DOUBLE globalCpuLoad = 0.0;       
DOUBLE globalNetSpeed = 0.0;
DWORD globalTotalThreads = 0;
DWORD globalTotalHandles = 0;
ULONGLONG globalSystemUptime = 0;  

int systemCores = 0;  
int systemLogicalProcs = 0;
SystemHardware sysHardware;

void ClearGuiArray();
void EnsureGuiPidsCapacity(INT needed);
void AddPidToGuiArray(DWORD pid);
BOOL IsPidInGuiArray(DWORD pid);
BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam);
void RefreshGuiProcessesList();
void ClearProcessesData();
void EnsureProcessesCapacity(INT needed);
void EnsureRenderItemsCapacity(INT needed);
void FreeProcessesArray();
INT GetProcesses();
