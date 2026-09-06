#include <stdafx.h>

#include "ImGuiDebug.h"

#include <CET.h>

ImGuiDebug::ImGuiDebug()
    : Widget("overlay.imgui_debug", true)
{
}

void ImGuiDebug::OnUpdate()
{
    // create Metrics/Debugger window. display Dear ImGui internals: windows, draw commands, various internal state,
    // etc.
    ImGui::ShowMetricsWindow();

    // create Debug Log window. display a simplified log of important dear imgui events.
    ImGui::ShowDebugLogWindow();

    // create Stack Tool window. hover items with mouse to query information about the source of their unique ID.
    ImGui::ShowStackToolWindow();

    // create About window. display Dear ImGui version, credits and build/system information.
    ImGui::ShowAboutWindow();

    // add style editor block (not a window). you can pass in a reference ImGuiStyle structure to compare to, revert to
    // and save to (else it uses the default style)
    ImGui::Begin(CET::Get().GetLocalization().Get("imgui.style_editor"));
    ImGui::ShowStyleEditor(nullptr);
    ImGui::End();

    // add basic help/info block (not a window): how to manipulate ImGui as a end-user (mouse/keyboard controls).
    // ImGui::ShowUserGuide();
}
