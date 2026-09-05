#include <stdafx.h>

#include "SystemFonts.h"

#include "Utils.h"

#include <shlobj.h>

namespace
{
int CALLBACK EnumFontFamExProc(const LOGFONTW* lpelfe, const TEXTMETRICW*, DWORD, LPARAM lParam)
{
    auto* out = reinterpret_cast<std::vector<SystemFonts::Entry>*>(lParam);
    if (lpelfe == nullptr || out == nullptr)
        return 1;

    // Skip vertical fonts (@...)
    if (lpelfe->lfFaceName[0] == L'@')
        return 1;

    SystemFonts::Entry entry;
    entry.face = lpelfe->lfFaceName;
    entry.family = UTF16ToUTF8(lpelfe->lfFaceName);

    for (const auto& existing : *out)
    {
        if (existing.face == entry.face)
            return 1;
    }
    out->push_back(std::move(entry));
    return 1;
}
} // namespace

std::vector<SystemFonts::Entry> SystemFonts::Enumerate()
{
    std::vector<Entry> fonts;
    HDC hdc = GetDC(nullptr);
    if (!hdc)
        return fonts;

    LOGFONTW logFont{};
    logFont.lfCharSet = DEFAULT_CHARSET;
    EnumFontFamiliesExW(hdc, &logFont, EnumFontFamExProc, reinterpret_cast<LPARAM>(&fonts), 0);
    ReleaseDC(nullptr, hdc);

    std::sort(fonts.begin(), fonts.end(), [](const Entry& a, const Entry& b) { return a.family < b.family; });
    return fonts;
}

std::filesystem::path SystemFonts::FindFontFile(const std::wstring& aFaceName)
{
    // Map face -> file via registry Fonts key (best-effort).
    HKEY key = nullptr;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts", 0, KEY_READ, &key) != ERROR_SUCCESS)
        return {};

    wchar_t valueName[512];
    wchar_t valueData[1024];
    DWORD index = 0;
    std::filesystem::path result;

    wchar_t windowsDir[MAX_PATH]{};
    GetWindowsDirectoryW(windowsDir, MAX_PATH);
    const auto fontsDir = std::filesystem::path(windowsDir) / L"Fonts";

    for (;;)
    {
        DWORD nameLen = static_cast<DWORD>(std::size(valueName));
        DWORD dataLen = static_cast<DWORD>(std::size(valueData) * sizeof(wchar_t));
        DWORD type = 0;
        const auto status = RegEnumValueW(key, index++, valueName, &nameLen, nullptr, &type, reinterpret_cast<LPBYTE>(valueData), &dataLen);
        if (status == ERROR_NO_MORE_ITEMS)
            break;
        if (status != ERROR_SUCCESS || type != REG_SZ)
            continue;

        std::wstring name(valueName, nameLen);
        if (name.find(aFaceName) == std::wstring::npos)
            continue;

        std::filesystem::path file = valueData;
        if (!file.is_absolute())
            file = fontsDir / file;
        if (exists(file))
        {
            result = file;
            break;
        }
    }

    RegCloseKey(key);
    return result;
}
