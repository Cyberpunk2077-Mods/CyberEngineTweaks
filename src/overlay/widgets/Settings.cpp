#include <stdafx.h>

#include "Settings.h"

#include <CET.h>

#include <overlay/Localization.h>
#include <overlay/Themes.h>
#include <Utils.h>

Settings::Settings(Options& aOptions, LuaVM& aVm)
    : Widget("Settings")
    , m_options(aOptions)
    , m_vm(aVm)
{
    m_systemFonts = SystemFonts::Enumerate();
    Load();
}

WidgetResult Settings::OnPopup()
{
    auto& loc = CET::Get().GetLocalization();
    const auto ret = UnsavedChangesPopup(loc.Get("settings.unsaved"), m_openChangesModal, m_madeChanges, [this] { Save(); }, [this] { Load(); });
    m_madeChanges = ret == TChangedCBResult::CHANGED;
    m_popupResult = ret;

    return m_madeChanges ? WidgetResult::ENABLED : WidgetResult::DISABLED;
}

WidgetResult Settings::OnDisable()
{
    if (m_enabled)
    {
        if (m_popupResult == TChangedCBResult::CANCEL)
        {
            m_popupResult = TChangedCBResult::APPLY;
            return WidgetResult::CANCEL;
        }

        if (m_madeChanges)
        {
            m_drawPopup = true;
            return WidgetResult::ENABLED;
        }

        m_enabled = false;
    }

    return m_enabled ? WidgetResult::ENABLED : WidgetResult::DISABLED;
}

void Settings::DrawInterfaceSettings()
{
    auto& loc = CET::Get().GetLocalization();
    const auto& savedUi = m_options.UI;

    ImGui::AlignTextToFramePadding();
    ImVec4 langColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);
    if (m_ui.Language != savedUi.Language)
        langColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Text, langColor);
    ImGui::TextUnformatted(loc.Get("settings.language"));
    ImGui::PopStyleColor();
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        ImGui::SetTooltip("%s", loc.Get("settings.language_tip"));

    ImGui::SameLine();
    ImGui::SetNextItemWidth(-FLT_MIN);

    int languageIndex = 0;
    const auto& languages = Localization::SupportedLanguages();
    if (!m_ui.Language.empty() && m_ui.Language != Localization::kAuto)
    {
        for (int i = 0; i < static_cast<int>(languages.size()); ++i)
        {
            if (m_ui.Language == languages[static_cast<size_t>(i)].code)
            {
                languageIndex = i + 1;
                break;
            }
        }
    }

    const char* preview = languageIndex == 0 ? loc.Get("settings.language_auto") : languages[static_cast<size_t>(languageIndex - 1)].nativeName;
    if (ImGui::BeginCombo("##UI_Language", preview))
    {
        if (ImGui::Selectable(loc.Get("settings.language_auto"), languageIndex == 0))
        {
            m_ui.Language = Localization::kAuto;
            languageIndex = 0;
        }
        for (int i = 0; i < static_cast<int>(languages.size()); ++i)
        {
            const bool selected = languageIndex == i + 1;
            if (ImGui::Selectable(languages[static_cast<size_t>(i)].nativeName, selected))
            {
                m_ui.Language = languages[static_cast<size_t>(i)].code;
                languageIndex = i + 1;
            }
            if (selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    m_madeChanges |= m_ui.Language != savedUi.Language;

    ImGui::AlignTextToFramePadding();
    ImVec4 themeColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);
    if (m_ui.Theme != savedUi.Theme)
        themeColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Text, themeColor);
    ImGui::TextUnformatted(loc.Get("settings.theme"));
    ImGui::PopStyleColor();
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        ImGui::SetTooltip("%s", loc.Get("settings.theme_tip"));

    ImGui::SameLine();
    ImGui::SetNextItemWidth(-FLT_MIN);

    const char* themeNames[] = {Themes::kDefault, Themes::kCyberpunk};
    const char* themeLabels[] = {loc.Get("settings.theme_default"), loc.Get("settings.theme_cyberpunk")};
    int themeIndex = m_ui.Theme == Themes::kCyberpunk ? 1 : 0;
    if (ImGui::BeginCombo("##UI_Theme", themeLabels[themeIndex]))
    {
        for (int i = 0; i < 2; ++i)
        {
            const bool selected = themeIndex == i;
            if (ImGui::Selectable(themeLabels[i], selected))
            {
                m_ui.Theme = themeNames[i];
                themeIndex = i;
            }
            if (selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    m_madeChanges |= m_ui.Theme != savedUi.Theme;

    ImGui::AlignTextToFramePadding();
    ImVec4 fontColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);
    if (m_ui.SystemFontFamily != savedUi.SystemFontFamily)
        fontColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_Text, fontColor);
    ImGui::TextUnformatted(loc.Get("settings.font"));
    ImGui::PopStyleColor();
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        ImGui::SetTooltip("%s", loc.Get("settings.font_tip"));

    ImGui::SameLine();
    ImGui::SetNextItemWidth(-FLT_MIN);

    const char* fontPreview = m_ui.SystemFontFamily.empty() ? loc.Get("settings.font_system") : m_ui.SystemFontFamily.c_str();
    if (ImGui::BeginCombo("##UI_Font", fontPreview))
    {
        if (ImGui::Selectable(loc.Get("settings.font_system"), m_ui.SystemFontFamily.empty()))
            m_ui.SystemFontFamily.clear();
        for (const auto& font : m_systemFonts)
        {
            const bool selected = m_ui.SystemFontFamily == font.family;
            if (ImGui::Selectable(font.family.c_str(), selected))
                m_ui.SystemFontFamily = font.family;
            if (selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndCombo();
    }
    m_madeChanges |= m_ui.SystemFontFamily != savedUi.SystemFontFamily;
}

void Settings::OnUpdate()
{
    auto& loc = CET::Get().GetLocalization();

    const auto frameSize = ImVec2(ImGui::GetContentRegionAvail().x, -(ImGui::GetFrameHeight() + ImGui::GetStyle().ItemSpacing.y + ImGui::GetStyle().FramePadding.y + 2.0f));
    if (ImGui::BeginChild(ImGui::GetID("Settings"), frameSize))
    {
        m_madeChanges = false;
        if (ImGui::CollapsingHeader(loc.Get("settings.interface"), ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::TreePush("##INTERFACE");
            DrawInterfaceSettings();
            ImGui::TreePop();
        }
        if (ImGui::CollapsingHeader(loc.Get("settings.patches"), ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::TreePush("##PATCHES");
            if (ImGui::BeginTable("##SETTINGS_PATCHES", 2, ImGuiTableFlags_Sortable | ImGuiTableFlags_SizingStretchSame, ImVec2(-ImGui::GetStyle().IndentSpacing, 0)))
            {
                const auto& patchesSettings = m_options.Patches;
                UpdateAndDrawSetting(
                    loc.Get("settings.disable_async_compute"), loc.Get("settings.disable_async_compute_tip"), m_patches.AsyncCompute, patchesSettings.AsyncCompute);
                UpdateAndDrawSetting(
                    loc.Get("settings.disable_antialiasing"), loc.Get("settings.disable_antialiasing_tip"), m_patches.Antialiasing, patchesSettings.Antialiasing);
                UpdateAndDrawSetting(
                    loc.Get("settings.disable_vignette"), loc.Get("settings.disable_vignette_tip"), m_patches.DisableVignette, patchesSettings.DisableVignette);
                UpdateAndDrawSetting(
                    loc.Get("settings.disable_boundary_teleport"), loc.Get("settings.disable_boundary_teleport_tip"), m_patches.DisableBoundaryTeleport,
                    patchesSettings.DisableBoundaryTeleport);
                UpdateAndDrawSetting(
                    loc.Get("settings.disable_win7_vsync"), loc.Get("settings.disable_win7_vsync_tip"), m_patches.DisableWin7Vsync, patchesSettings.DisableWin7Vsync);

                ImGui::EndTable();
            }
            ImGui::TreePop();
        }
        if (ImGui::CollapsingHeader(loc.Get("settings.developer"), ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::TreePush("##DEV");
            if (ImGui::BeginTable("##SETTINGS_DEV", 2, ImGuiTableFlags_Sortable | ImGuiTableFlags_SizingStretchSame, ImVec2(-ImGui::GetStyle().IndentSpacing, 0)))
            {
                const auto& developerSettings = m_options.Developer;
                UpdateAndDrawSetting(
                    loc.Get("settings.remove_dead_bindings"), loc.Get("settings.remove_dead_bindings_tip"), m_developer.RemoveDeadBindings,
                    developerSettings.RemoveDeadBindings);
                UpdateAndDrawSetting(
                    loc.Get("settings.enable_imgui_assertions"), loc.Get("settings.enable_imgui_assertions_tip"), m_developer.EnableImGuiAssertions,
                    developerSettings.EnableImGuiAssertions);
                UpdateAndDrawSetting(
                    loc.Get("settings.dump_game_options"), loc.Get("settings.dump_game_options_tip"), m_developer.DumpGameOptions, developerSettings.DumpGameOptions);
                UpdateAndDrawSetting(loc.Get("settings.enable_jit"), loc.Get("settings.enable_jit_tip"), m_developer.EnableJIT, developerSettings.EnableJIT);

                ImGui::EndTable();
            }
            ImGui::TreePop();
        }
    }
    ImGui::EndChild();

    ImGui::Separator();

    const auto itemWidth = GetAlignedItemWidth(3);
    if (ImGui::Button(loc.Get("settings.load"), ImVec2(itemWidth, 0)))
        Load();
    ImGui::SameLine();
    if (ImGui::Button(loc.Get("settings.save"), ImVec2(itemWidth, 0)))
        Save();
    ImGui::SameLine();
    if (ImGui::Button(loc.Get("settings.defaults"), ImVec2(itemWidth, 0)))
        ResetToDefaults();
}

void Settings::Load()
{
    m_options.Load();

    m_patches = m_options.Patches;
    m_developer = m_options.Developer;
    m_ui = m_options.UI;
}

void Settings::Save() const
{
    m_options.Patches = m_patches;
    m_options.Developer = m_developer;

    m_options.UI = m_ui;
    m_options.UI.LanguageConfigured = !m_ui.Language.empty() && m_ui.Language != Localization::kAuto;

    m_options.Save();

    auto& cet = CET::Get();
    cet.GetLocalization().SetLanguage(m_options.UI.Language);
    cet.GetD3D12().ApplyUIStyleAndFonts();
}

void Settings::ResetToDefaults()
{
    m_options.ResetToDefaults();

    m_patches = m_options.Patches;
    m_developer = m_options.Developer;
    m_ui = m_options.UI;

    auto& cet = CET::Get();
    cet.GetLocalization().SetLanguage(m_options.UI.Language);
    cet.GetD3D12().ApplyUIStyleAndFonts();
}

void Settings::UpdateAndDrawSetting(const std::string& acLabel, const std::string& acTooltip, bool& aCurrent, const bool& acSaved)
{
    ImGui::TableNextRow();
    ImGui::TableNextColumn();

    ImVec4 curTextColor = ImGui::GetStyleColorVec4(ImGuiCol_Text);
    if (aCurrent != acSaved)
        curTextColor = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);

    ImGui::AlignTextToFramePadding();

    ImGui::PushStyleColor(ImGuiCol_Text, curTextColor);

    ImGui::PushID(&acLabel);
    ImGui::TextUnformatted(acLabel.c_str());
    ImGui::PopID();

    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled) && !acTooltip.empty())
        ImGui::SetTooltip("%s", acTooltip.c_str());

    ImGui::TableNextColumn();

    ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (ImGui::GetContentRegionAvail().x - ImGui::GetFrameHeight()) / 2);
    ImGui::Checkbox(("##" + acLabel).c_str(), &aCurrent);
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
        ImGui::SetTooltip("%s", acTooltip.c_str());

    ImGui::PopStyleColor();

    m_madeChanges |= aCurrent != acSaved;
}
