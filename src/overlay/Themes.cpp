#include <stdafx.h>

#include "Themes.h"

namespace
{
void ApplyCyberpunkColors(ImGuiStyle& style)
{
    // Inspired by AMM Cyberpunk theme (yellow/cyan on dark magenta frame).
    auto* c = style.Colors;
    c[ImGuiCol_Text] = ImVec4(224 / 255.f, 58 / 255.f, 68 / 255.f, 1.f);
    c[ImGuiCol_TextDisabled] = ImVec4(0.48f, 0.39f, 0.40f, 1.f);
    c[ImGuiCol_WindowBg] = ImVec4(0.06f, 0.05f, 0.07f, 0.94f);
    c[ImGuiCol_ChildBg] = ImVec4(0.08f, 0.06f, 0.08f, 0.80f);
    c[ImGuiCol_PopupBg] = ImVec4(0.10f, 0.07f, 0.09f, 0.96f);
    c[ImGuiCol_Border] = ImVec4(97 / 255.f, 219 / 255.f, 223 / 255.f, 0.45f);
    c[ImGuiCol_FrameBg] = ImVec4(0.48f, 0.39f, 0.40f, 0.55f);
    c[ImGuiCol_FrameBgHovered] = ImVec4(195 / 255.f, 77 / 255.f, 69 / 255.f, 0.70f);
    c[ImGuiCol_FrameBgActive] = ImVec4(195 / 255.f, 77 / 255.f, 69 / 255.f, 0.85f);
    c[ImGuiCol_TitleBg] = ImVec4(0.12f, 0.06f, 0.08f, 1.f);
    c[ImGuiCol_TitleBgActive] = ImVec4(157 / 255.f, 30 / 255.f, 36 / 255.f, 0.90f);
    c[ImGuiCol_TitleBgCollapsed] = ImVec4(0.10f, 0.05f, 0.06f, 0.75f);
    c[ImGuiCol_MenuBarBg] = ImVec4(0.12f, 0.08f, 0.09f, 1.f);
    c[ImGuiCol_ScrollbarBg] = ImVec4(0.08f, 0.06f, 0.07f, 0.60f);
    c[ImGuiCol_ScrollbarGrab] = ImVec4(97 / 255.f, 219 / 255.f, 223 / 255.f, 0.45f);
    c[ImGuiCol_ScrollbarGrabHovered] = ImVec4(97 / 255.f, 219 / 255.f, 223 / 255.f, 0.70f);
    c[ImGuiCol_ScrollbarGrabActive] = ImVec4(97 / 255.f, 219 / 255.f, 223 / 255.f, 0.90f);
    c[ImGuiCol_CheckMark] = ImVec4(97 / 255.f, 219 / 255.f, 223 / 255.f, 1.f);
    c[ImGuiCol_SliderGrab] = ImVec4(195 / 255.f, 77 / 255.f, 69 / 255.f, 0.85f);
    c[ImGuiCol_SliderGrabActive] = ImVec4(224 / 255.f, 58 / 255.f, 68 / 255.f, 1.f);
    c[ImGuiCol_Button] = ImVec4(157 / 255.f, 30 / 255.f, 36 / 255.f, 0.65f);
    c[ImGuiCol_ButtonHovered] = ImVec4(195 / 255.f, 77 / 255.f, 69 / 255.f, 0.85f);
    c[ImGuiCol_ButtonActive] = ImVec4(224 / 255.f, 58 / 255.f, 68 / 255.f, 1.f);
    c[ImGuiCol_Header] = ImVec4(157 / 255.f, 30 / 255.f, 36 / 255.f, 0.55f);
    c[ImGuiCol_HeaderHovered] = ImVec4(195 / 255.f, 77 / 255.f, 69 / 255.f, 0.75f);
    c[ImGuiCol_HeaderActive] = ImVec4(224 / 255.f, 58 / 255.f, 68 / 255.f, 0.90f);
    c[ImGuiCol_Separator] = ImVec4(97 / 255.f, 219 / 255.f, 223 / 255.f, 0.35f);
    c[ImGuiCol_Tab] = ImVec4(0.20f, 0.10f, 0.12f, 0.85f);
    c[ImGuiCol_TabHovered] = ImVec4(195 / 255.f, 77 / 255.f, 69 / 255.f, 0.85f);
    c[ImGuiCol_TabActive] = ImVec4(157 / 255.f, 30 / 255.f, 36 / 255.f, 0.90f);
    c[ImGuiCol_PlotLines] = ImVec4(97 / 255.f, 219 / 255.f, 223 / 255.f, 1.f);
    c[ImGuiCol_PlotHistogram] = ImVec4(224 / 255.f, 58 / 255.f, 68 / 255.f, 1.f);
    c[ImGuiCol_TextSelectedBg] = ImVec4(97 / 255.f, 219 / 255.f, 223 / 255.f, 0.35f);
}
} // namespace

void Themes::Apply(ImGuiStyle& aStyle, const std::string& aThemeName)
{
    ImGui::StyleColorsDark(&aStyle);
    aStyle.WindowRounding = 6.0f;
    aStyle.WindowTitleAlign.x = 0.5f;
    aStyle.ChildRounding = 6.0f;
    aStyle.PopupRounding = 6.0f;
    aStyle.FrameRounding = 6.0f;
    aStyle.ScrollbarRounding = 12.0f;
    aStyle.GrabRounding = 12.0f;
    aStyle.TabRounding = 6.0f;

    if (aThemeName == kCyberpunk)
        ApplyCyberpunkColors(aStyle);
}
