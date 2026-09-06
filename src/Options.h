#pragma once

#include "Image.h"

struct Paths;

struct PatchesSettings
{
    void Load(const nlohmann::json& aConfig);
    nlohmann::json Save() const;
    void ResetToDefaults();

    [[nodiscard]] auto operator<=>(const PatchesSettings&) const = default;

    bool AsyncCompute{false};
    bool Antialiasing{false};
    bool DisableVignette{false};
    bool DisableBoundaryTeleport{false};
    bool DisableWin7Vsync{false};
};

struct FontSettings
{
    void Load(const nlohmann::json& aConfig);
    nlohmann::json Save() const;
    void ResetToDefaults();

    [[nodiscard]] auto operator<=>(const FontSettings&) const = default;

    std::string Path{};
    std::string Language{"Default"};
    float BaseSize{18.0f};
    int32_t OversampleHorizontal{3};
    int32_t OversampleVertical{1};
};

struct DeveloperSettings
{
    void Load(const nlohmann::json& aConfig);
    nlohmann::json Save() const;
    void ResetToDefaults();

    [[nodiscard]] auto operator<=>(const DeveloperSettings&) const = default;

    bool RemoveDeadBindings{true};
    bool EnableImGuiAssertions{false};
    bool DumpGameOptions{false};
    uint64_t MaxLinesConsoleHistory{1000};
    bool PersistentConsole{true};
    bool EnableJIT{true};
};

struct UISettings
{
    void Load(const nlohmann::json& aConfig);
    nlohmann::json Save() const;
    void ResetToDefaults();

    [[nodiscard]] auto operator<=>(const UISettings&) const = default;

    // "auto" follows the current game language on every launch, with the system language as fallback.
    std::string Language{"auto"};
    bool LanguageConfigured{false};
    std::string Theme{"Default"};
    // Empty = try Segoe UI / system default via FindFontFile; otherwise system font family name.
    std::string SystemFontFamily{};
};

struct Options
{
    Options(Paths& aPaths);
    ~Options() = default;

    void Load();
    void Save() const;
    void ResetToDefaults();

    Image GameImage;
    bool ExeValid{false};

    PatchesSettings Patches{};
    FontSettings Font{};
    DeveloperSettings Developer{};
    UISettings UI{};

private:
    Paths& m_paths;
};
