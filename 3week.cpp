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
void StartNewProcessPrompt() {
    FlushConsoleInputBuffer(GetStdHandle(STD_INPUT_HANDLE));

    HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleActiveScreenBuffer(hStdOut);

    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hStdOut, &cursorInfo);
    cursorInfo.bVisible = TRUE;
    SetConsoleCursorInfo(hStdOut, &cursorInfo);

    system("cls");
    std::cout << "==================================================\n";
    std::cout << "ЗАПУСК НОВОГО ПРОЦЕССА\n";
    std::cout << "==================================================\n";
    std::cout << "Введите имя программы\n";
    std::cout << "Или нажмите ENTER для отмены и выхода.\n";
    std::cout << "--------------------------------------------------\n";
    std::cout << "-> ";

    std::string userInput;
    std::getline(std::cin, userInput);

    if (userInput.empty() || userInput == "exit" || userInput == "EXIT") {
        std::cout << "\nВозврат в диспетчер задач...\n";
        Sleep(500);
    }
    else {
        std::string exeName = userInput;
        if (exeName.length() < 4 || exeName.substr(exeName.length() - 4) != ".exe") {
            exeName += ".exe";
        }

        char fullPath[MAX_PATH] = { 0 };
        BOOL foundInRegistry = FALSE;

        std::string regPath = "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\" + exeName;
        HKEY hKey;
        if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, regPath.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
            DWORD bufferSize = MAX_PATH;
            if (RegQueryValueExA(hKey, NULL, NULL, NULL, (LPBYTE)fullPath, &bufferSize) == ERROR_SUCCESS) {
                foundInRegistry = TRUE;
            }
            RegCloseKey(hKey);
        }

        if (!foundInRegistry) {
            if (RegOpenKeyExA(HKEY_CURRENT_USER, regPath.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
                DWORD bufferSize = MAX_PATH;
                if (RegQueryValueExA(hKey, NULL, NULL, NULL, (LPBYTE)fullPath, &bufferSize) == ERROR_SUCCESS) {
                    foundInRegistry = TRUE;
                }
                RegCloseKey(hKey);
            }
        }

        STARTUPINFOA si = { sizeof(si) };
        PROCESS_INFORMATION pi;
        BOOL success = FALSE;

        if (foundInRegistry) {
            success = CreateProcessA(fullPath, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
        }
        else {
            char* commandLine = new char[exeName.length() + 1];
            strcpy_s(commandLine, exeName.length() + 1, exeName.c_str());
            success = CreateProcessA(NULL, commandLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);
            delete[] commandLine;
        }

        if (success) {
            std::cout << "\nПроцесс успешно запущен! PID: " << pi.dwProcessId << "\n";
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
        else {
            std::cout << "\nОшибка: Не удалось найти или запустить \"" << userInput << "\".\n";
            std::cout << "Убедитесь, что программа установлена на ПК.\n";
        }
        Sleep(1500);
    }

    cursorInfo.bVisible = FALSE;
    for (int i = 0; i < 2; i++) {
        SetConsoleCursorInfo(hBuffers[i], &cursorInfo);
    }
    SetConsoleActiveScreenBuffer(hBuffers[currentBuffer]);
}
