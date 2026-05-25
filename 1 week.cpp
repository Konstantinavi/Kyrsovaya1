#include 'common.h'

ProcessCategory DetectProcessCategory(DWORD pid, const WCHAR* name) {
    if (_wcsicmp(name, L"svchost.exe") == 0 || _wcsicmp(name, L"csrss.exe") == 0 ||
        _wcsicmp(name, L"explorer.exe") == 0 || _wcsicmp(name, L"services.exe") == 0 ||
        _wcsicmp(name, L"lsass.exe") == 0 || _wcsicmp(name, L"wininit.exe") == 0 ||
        _wcsicmp(name, L"winlogon.exe") == 0 || _wcsicmp(name, L"dwm.exe") == 0) {
        return CAT_WINDOWS;
    }
    if (IsPidInGuiArray(pid)) return CAT_APP;
    return CAT_BACKGROUND;
}

unsigned long long FileTimeToQuadWord(const FILETIME* ft) {
    if (ft == nullptr) return 0;
    ULARGE_INTEGER uli;
    uli.LowPart = ft->dwLowDateTime;
    uli.HighPart = ft->dwHighDateTime;
    return uli.QuadPart;
}

std::string WStringToString(const WCHAR* wstr) {
    if (!wstr) return "";
    int size = WideCharToMultiByte(1251, 0, wstr, -1, nullptr, 0, nullptr, nullptr);
    if (size <= 0) return "";
    std::string result(size - 1, 0);
    WideCharToMultiByte(1251, 0, wstr, -1, &result[0], size, nullptr, nullptr);
    return result;
}

std::string FormatMemory(DWORD bytes) {
    double mb = bytes / (1024.0 * 1024.0);
    char buffer[32];
    sprintf_s(buffer, "%.1f", mb);
    return std::string(buffer);
}
