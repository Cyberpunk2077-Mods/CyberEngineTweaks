#include <stdafx.h>

#include "Localization.h"

#include "Paths.h"
#include "Utils.h"

#include <functional>
#include <shlobj.h>

namespace
{
const Localization::LanguageInfo kLanguages[] = {
    {"ar-ar", "Arabic", "العربية"},
    {"cz-cz", "Czech", "Čeština"},
    {"de-de", "German", "Deutsch"},
    {"en-us", "English", "English"},
    {"es-es", "Spanish", "Español"},
    {"es-mx", "Spanish (Mexico)", "Español (México)"},
    {"fr-fr", "French", "Français"},
    {"hu-hu", "Hungarian", "Magyar"},
    {"it-it", "Italian", "Italiano"},
    {"jp-jp", "Japanese", "日本語"},
    {"kr-kr", "Korean", "한국어"},
    {"pl-pl", "Polish", "Polski"},
    {"pt-br", "Portuguese (Brazil)", "Português (Brasil)"},
    {"ru-ru", "Russian", "Русский"},
    {"th-th", "Thai", "ไทย"},
    {"tr-tr", "Turkish", "Türkçe"},
    {"ua-ua", "Ukrainian", "Українська"},
    {"zh-cn", "Chinese (Simplified)", "简体中文"},
    {"zh-tw", "Chinese (Traditional)", "繁體中文"},
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
    {"overlay.bind_notification", "CET overlay key: %s"},
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
    {"settings.language_tip", "By default, CET follows the game language. A manual selection is saved and used on the next launch."},
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
    {"common.save", "Save"},
    {"common.clear", "Clear"},
    {"common.edit", "Edit"},
    {"common.cancel", "Cancel"},
    {"common.apply", "Apply"},
    {"common.discard", "Discard"},
    {"common.add_new", "Add new"},
    {"common.search", "Search"},
    {"common.regex", "Regex"},
    {"common.success", "Success!"},
    {"common.failed_check_console", "Failed. Check the console!"},
    {"common.unsaved_changes", "Unsaved changes"},
    {"common.unsaved_changes_title", "{} - Unsaved changes"},
    {"common.unsaved_changes_message", "You have some unsaved changes."},
    {"common.unsaved_changes_question", "Do you wish to apply them or discard them?"},
    {"log.clear_output", "Clear output"},
    {"log.auto_scroll", "Auto-scroll"},
    {"bindings.reset_changes", "Reset changes"},
    {"bindings.first_time_title", "CET First Time Setup"},
    {"bindings.first_time_prompt", "Please bind a key combination to toggle the overlay."},
    {"bindings.first_time_limit", "The combination can contain up to 4 keys."},
    {"bindings.tooltip_unavailable", "This tooltip is temporarily unavailable. Please wait a moment."},
    {"bindings.binding", "Binding..."},
    {"bindings.unbind_tip", "Clear this checkbox to unbind the action."},
    {"bindings.hotkeys", "Hotkeys"},
    {"bindings.hotkeys_tip", "Hotkeys run after the assigned key combination is pressed and released. You can assign up to 4 keys."},
    {"bindings.inputs", "Inputs"},
    {"bindings.inputs_tip", "Inputs react while a key is pressed and released. You can assign one key."},
    {"bindings.overlay_key", "Overlay key"},
    {"bindings.overlay_key_tip", "Use this hotkey to show or hide the overlay."},
    {"tweakdb.not_initialized", "TweakDB is not initialized yet"},
    {"tweakdb.rebuilding_cache", "Rebuilding cache..."},
    {"tweakdb.records", "Records"},
    {"tweakdb.queries", "Queries"},
    {"tweakdb.flats", "Flats"},
    {"tweakdb.advanced", "Advanced"},
    {"tweakdb.error_record_not_found", "RECORD NOT FOUND"},
    {"tweakdb.error_flat_not_found", "FLAT NOT FOUND"},
    {"tweakdb.flat_not_found", "'%s' was not found in TweakDB"},
    {"tweakdb.unsupported_type", "Unsupported type: %s"},
    {"tweakdb.roll", "Roll"},
    {"tweakdb.pitch", "Pitch"},
    {"tweakdb.yaw", "Yaw"},
    {"tweakdb.color_not_supported", "'Color' is not supported yet"},
    {"tweakdb.localization_key", "This value is a localization key"},
    {"tweakdb.specific_values", "The game expects specific values."},
    {"tweakdb.grouping_depth", "'Flats' grouping depth"},
    {"tweakdb.dropdown_height", "Combo box dropdown height"},
    {"tweakdb.refresh_all", "Refresh all"},
    {"tweakdb.record_name", "Record name"},
    {"tweakdb.delete_record", "Delete record"},
    {"tweakdb.record_type", "Record type to create"},
    {"tweakdb.create_record", "Create record"},
    {"tweakdb.record_to_clone", "Record to clone"},
    {"tweakdb.clone_record", "Clone record"},
    {"tweakdb.unknown_group", "!Unknown!"},
    {"tweakdb.bad_name", "!Bad name!"},
    {"tweakdb.flat_count", "{} - {} flats"},
    {"tweakdb.array_items", "[%s] %u items"},
    {"imgui.style_editor", "Dear ImGui Style Editor"},
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
                std::function<std::string(const nlohmann::json&)> findOnScreenLanguage = [&](const nlohmann::json& node) -> std::string {
                    if (node.is_object())
                    {
                        if (node.value("name", "") == "OnScreen")
                        {
                            const auto value = node.value("value", "");
                            if (!value.empty())
                                return NormalizeLanguageCode(value);
                        }

                        for (const auto& child : node)
                        {
                            if (auto found = findOnScreenLanguage(child); !found.empty())
                                return found;
                        }
                    }
                    else if (node.is_array())
                    {
                        for (const auto& child : node)
                        {
                            if (auto found = findOnScreenLanguage(child); !found.empty())
                                return found;
                        }
                    }
                    return {};
                };
                if (auto found = findOnScreenLanguage(json); !found.empty())
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
