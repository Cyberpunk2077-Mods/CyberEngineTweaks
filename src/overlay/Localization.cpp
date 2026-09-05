#include <stdafx.h>

#include "Localization.h"

#include "Paths.h"
#include "Utils.h"

#include <functional>
#include <shlobj.h>

namespace
{
const Localization::LanguageInfo kLanguages[] = {
    {"ar-ar", "العربية"},
    {"cz-cz", "Čeština"},
    {"de-de", "Deutsch"},
    {"en-us", "English"},
    {"es-es", "Español"},
    {"es-mx", "Español (México)"},
    {"fr-fr", "Français"},
    {"hu-hu", "Magyar"},
    {"it-it", "Italiano"},
    {"jp-jp", "日本語"},
    {"kr-kr", "한국어"},
    {"pl-pl", "Polski"},
    {"pt-br", "Português (Brasil)"},
    {"ru-ru", "Русский"},
    {"th-th", "ไทย"},
    {"tr-tr", "Türkçe"},
    {"ua-ua", "Українська"},
    {"zh-cn", "简体中文"},
    {"zh-tw", "繁體中文"},
};

// Built-in English strings so CET works even if locale files are missing.
const std::pair<const char*, const char*> kEnglishDefaults[] = {
    {"overlay.title", "Cyber Engine Tweaks"},
    {"overlay.console", "Console"},
    {"overlay.bindings", "Bindings"},
    {"overlay.settings", "Settings"},
    {"overlay.tweakdb", "TweakDB Editor"},
    {"overlay.game_log", "Game Log"},
    {"overlay.imgui_debug", "ImGui Debug"},
    {"overlay.reload_mods", "Reload all mods"},
    {"settings.patches", "Patches"},
    {"settings.disable_async_compute", "Disable Async Compute"},
    {"settings.disable_async_compute_tip",
     "Disables async compute, this can give a boost on older GPUs like Nvidia 10xx series for example (requires restart to take effect)."},
    {"settings.disable_antialiasing", "Disable Anti-aliasing"},
    {"settings.disable_antialiasing_tip", "Completely disables anti-aliasing (requires restart to take effect)."},
    {"settings.disable_vignette", "Disable Vignette"},
    {"settings.disable_vignette_tip", "Disables vignetting along screen borders (requires restart to take effect)."},
    {"settings.disable_boundary_teleport", "Disable Boundary Teleport"},
    {"settings.disable_boundary_teleport_tip", "Allows players to access out-of-bounds locations (requires restart to take effect)."},
    {"settings.disable_win7_vsync", "Disable V-Sync (Windows 7 only)"},
    {"settings.disable_win7_vsync_tip", "Disables VSync on Windows 7 to bypass the 60 FPS limit (requires restart to take effect)."},
    {"settings.developer", "CET Development Settings"},
    {"settings.remove_dead_bindings", "Remove Dead Bindings"},
    {"settings.remove_dead_bindings_tip",
     "Removes all bindings which are no longer valid (disabling this could be useful when debugging mod issues)."},
    {"settings.enable_imgui_assertions", "Enable ImGui Assertions"},
    {"settings.enable_imgui_assertions_tip",
     "Enables all ImGui assertions, assertions will get logged into log file of whoever triggered the assertion (useful when debugging ImGui issues, should also be used to check mods before shipping!)."},
    {"settings.dump_game_options", "Dump Game Options"},
    {"settings.dump_game_options_tip", "Dumps all game options into main log file (requires restart to take effect)."},
    {"settings.enable_jit", "Enable JIT for Lua"},
    {"settings.enable_jit_tip",
     "Enables JIT compiler for Lua VM, which may majorly speed up the mods. Disable it in case you experience issues as a troubleshooting step (requires restart to take effect)."},
    {"settings.interface", "Interface"},
    {"settings.language", "Language"},
    {"settings.language_auto", "Match game language"},
    {"settings.language_tip", "First open follows the game language. Changing here is saved and reused on next launch."},
    {"settings.theme", "Theme"},
    {"settings.theme_default", "Default"},
    {"settings.theme_cyberpunk", "Cyberpunk"},
    {"settings.theme_tip", "Overlay visual theme. Does not affect mods that set their own ImGui style."},
    {"settings.font", "Font"},
    {"settings.font_system", "System font"},
    {"settings.font_size", "Font size"},
    {"settings.font_tip", "Uses an installed system font. Material icons remain bundled with CET."},
    {"settings.load", "Load"},
    {"settings.save", "Save"},
    {"settings.defaults", "Defaults"},
    {"settings.unsaved", "Settings"},
};
} // namespace

Localization::Localization(Paths& aPaths)
    : m_paths(aPaths)
{
    EnsureEnglishFallback();
    m_strings = m_english;
}

const std::vector<Localization::LanguageInfo>& Localization::SupportedLanguages()
{
    static const std::vector<LanguageInfo> languages(std::begin(kLanguages), std::end(kLanguages));
    return languages;
}

void Localization::EnsureEnglishFallback()
{
    m_english.clear();
    for (const auto& [key, value] : kEnglishDefaults)
        m_english.emplace(key, value);
}

void Localization::LoadFile(const std::string& aCode, std::unordered_map<std::string, std::string>& aOut) const
{
    const auto path = m_paths.CETRoot() / L"locales" / (UTF8ToUTF16(aCode) + L".json");
    if (!exists(path))
        return;

    std::ifstream file(path);
    if (!file)
        return;

    auto json = nlohmann::json::parse(file, nullptr, false);
    if (!json.is_object())
        return;

    for (auto it = json.begin(); it != json.end(); ++it)
    {
        if (it.value().is_string())
            aOut[it.key()] = it.value().get<std::string>();
    }
}

void Localization::Load()
{
    EnsureEnglishFallback();
    m_strings = m_english;

    auto code = m_language;
    if (IsAuto())
        code = DetectGameOrSystemLanguage();

    m_resolved = NormalizeLanguageCode(code);
    if (m_resolved != "en-us")
    {
        std::unordered_map<std::string, std::string> localized;
        LoadFile(m_resolved, localized);
        for (auto& [key, value] : localized)
            m_strings[key] = std::move(value);
    }
    else
    {
        LoadFile("en-us", m_strings); // optional overrides on top of built-ins
        for (const auto& [key, value] : m_english)
            m_strings.try_emplace(key, value);
    }
}

void Localization::SetLanguage(const std::string& aCode)
{
    m_language = aCode.empty() ? kAuto : aCode;
    Load();
}

const char* Localization::Get(const char* aKey) const
{
    if (aKey == nullptr)
        return "";

    if (const auto it = m_strings.find(aKey); it != m_strings.end())
        return it->second.c_str();
    if (const auto it = m_english.find(aKey); it != m_english.end())
        return it->second.c_str();
    return aKey;
}

std::string Localization::NormalizeLanguageCode(std::string aCode)
{
    for (char& c : aCode)
    {
        if (c == '_')
            c = '-';
        else
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }

    // Common aliases from AMM / Windows
    if (aCode == "en" || aCode == "eng")
        return "en-us";
    if (aCode == "zh" || aCode == "zh-hans" || aCode == "zh_cn")
        return "zh-cn";
    if (aCode == "zh-hant" || aCode == "zh_tw" || aCode == "zh-hk")
        return "zh-tw";
    if (aCode == "ua" || aCode == "uk" || aCode == "uk-ua")
        return "ua-ua";
    if (aCode == "jp" || aCode == "ja" || aCode == "ja-jp")
        return "jp-jp";
    if (aCode == "kr" || aCode == "ko" || aCode == "ko-kr")
        return "kr-kr";
    if (aCode == "pt" || aCode == "pt-pt")
        return "pt-br";
    if (aCode == "cs" || aCode == "cs-cz")
        return "cz-cz";

    for (const auto& lang : SupportedLanguages())
    {
        if (aCode == lang.code)
            return lang.code;
    }
    return "en-us";
}

std::string Localization::DetectGameOrSystemLanguage()
{
    // Prefer game options file when present (Interface OnScreen language).
    // Fallback: Windows UI language.
    try
    {
        wchar_t localAppData[MAX_PATH]{};
        if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_LOCAL_APPDATA, nullptr, 0, localAppData)))
        {
            const auto optionsPath = std::filesystem::path(localAppData) / L"CD Projekt Red" / L"Cyberpunk 2077" / L"UserSettings.json";
            if (exists(optionsPath))
            {
                std::ifstream file(optionsPath);
                auto json = nlohmann::json::parse(file, nullptr, false);
                // Best-effort scan for language-like string values
                std::function<std::string(const nlohmann::json&)> findLang = [&](const nlohmann::json& node) -> std::string {
                    if (node.is_string())
                    {
                        auto value = NormalizeLanguageCode(node.get<std::string>());
                        for (const auto& lang : SupportedLanguages())
                        {
                            if (value == lang.code)
                                return value;
                        }
                    }
                    else if (node.is_object())
                    {
                        for (auto it = node.begin(); it != node.end(); ++it)
                        {
                            const auto key = it.key();
                            if (key.find("lang") != std::string::npos || key.find("Lang") != std::string::npos || key == "OnScreen")
                            {
                                auto nested = findLang(it.value());
                                if (!nested.empty() && nested != "en-us")
                                    return nested;
                            }
                            auto nested = findLang(it.value());
                            if (!nested.empty() && nested != "en-us" && (key.find("lang") != std::string::npos || key.find("OnScreen") != std::string::npos))
                                return nested;
                        }
                    }
                    else if (node.is_array())
                    {
                        for (const auto& child : node)
                        {
                            auto nested = findLang(child);
                            if (!nested.empty() && nested != "en-us")
                                return nested;
                        }
                    }
                    return {};
                };
                if (auto found = findLang(json); !found.empty())
                    return found;
            }
        }
    }
    catch (...)
    {
    }

    const LANGID langId = GetUserDefaultUILanguage();
    const WORD primary = PRIMARYLANGID(langId);
    const WORD sub = SUBLANGID(langId);
    switch (primary)
    {
    case LANG_ARABIC: return "ar-ar";
    case LANG_CZECH: return "cz-cz";
    case LANG_GERMAN: return "de-de";
    case LANG_SPANISH: return sub == SUBLANG_SPANISH_MEXICAN ? "es-mx" : "es-es";
    case LANG_FRENCH: return "fr-fr";
    case LANG_HUNGARIAN: return "hu-hu";
    case LANG_ITALIAN: return "it-it";
    case LANG_JAPANESE: return "jp-jp";
    case LANG_KOREAN: return "kr-kr";
    case LANG_POLISH: return "pl-pl";
    case LANG_PORTUGUESE: return "pt-br";
    case LANG_RUSSIAN: return "ru-ru";
    case LANG_THAI: return "th-th";
    case LANG_TURKISH: return "tr-tr";
    case LANG_UKRAINIAN: return "ua-ua";
    case LANG_CHINESE:
        return sub == SUBLANG_CHINESE_SIMPLIFIED ? "zh-cn" : "zh-tw";
    default: return "en-us";
    }
}
