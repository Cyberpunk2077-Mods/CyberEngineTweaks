#pragma once

#include <imgui.h>
#include <string>

struct Themes
{
    static constexpr const char* kDefault = "Default";
    static constexpr const char* kCyberpunk = "Cyberpunk";

    // Builds a base style (rounding etc.) then applies named theme colors.
    static void Apply(ImGuiStyle& aStyle, const std::string& aThemeName);
};
