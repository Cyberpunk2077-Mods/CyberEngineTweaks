#pragma once

#include <filesystem>
#include <string>
#include <vector>

struct SystemFonts
{
    struct Entry
    {
        std::string family; // UTF-8 display name
        std::wstring face;  // native face name for CreateFont
    };

    static std::vector<Entry> Enumerate();
    // Resolve a family name to a .ttf/.otf file path when possible (Windows font link / registry).
    static std::filesystem::path FindFontFile(const std::wstring& aFaceName);
};
