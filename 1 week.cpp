#include 'common.h'

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
