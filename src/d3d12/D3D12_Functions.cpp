#include <stdafx.h>

#include "D3D12.h"
#include "Options.h"
#include "Utils.h"

#include <CET.h>
#include <imgui_impl/dx12.h>
#include <imgui_impl/win32.h>
#include <overlay/Localization.h>
#include <overlay/SystemFonts.h>
#include <overlay/Themes.h>
#include <window/window.h>

bool D3D12::ResetState(const bool acDestroyContext)
{
    if (m_initialized)
    {
        std::lock_guard _(m_imguiLock);

        for (auto& drawData : m_imguiDrawDataBuffers)
        {
            for (auto i = 0; i < drawData.CmdListsCount; ++i)
                IM_DELETE(drawData.CmdLists[i]);
            drawData.Clear();
        }

        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();

        if (acDestroyContext)
            ImGui::DestroyContext();
    }

    m_frameContexts.clear();
    m_outSize = {0, 0};

    m_pd3d12Device.Reset();
    m_pd3dRtvDescHeap.Reset();
    m_pd3dSrvDescHeap.Reset();

    m_pCommandQueue.Reset();
    m_pdxgiSwapChain.Reset();

    m_initialized = false;

    return false;
}

bool D3D12::Initialize()
{
    if (m_initialized)
        return true;

    if (!m_pdxgiSwapChain)
        return false;

    const HWND hWnd = m_window.GetWindow();
    if (!hWnd)
    {
        Log::Warn("D3D12::Initialize() - window not yet hooked!");
        return false;
    }

    if (FAILED(m_pdxgiSwapChain->GetDevice(IID_PPV_ARGS(&m_pd3d12Device))))
    {
        Log::Error("D3D12::Initialize() - failed to get device!");
        return ResetState();
    }

    DXGI_SWAP_CHAIN_DESC sdesc;
    m_pdxgiSwapChain->GetDesc(&sdesc);

    if (hWnd != sdesc.OutputWindow)
        Log::Warn(
            "D3D12::Initialize() - output window of current swap chain does not match hooked window! Currently hooked "
            "to {} while swap chain output window is {}.",
            reinterpret_cast<void*>(hWnd), reinterpret_cast<void*>(sdesc.OutputWindow));

    m_outSize = {static_cast<LONG>(sdesc.BufferDesc.Width), static_cast<LONG>(sdesc.BufferDesc.Height)};

    const auto buffersCounts = std::min(sdesc.BufferCount, 3u);
    m_frameContexts.resize(buffersCounts);

    D3D12_DESCRIPTOR_HEAP_DESC srvdesc = {};
    srvdesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
    srvdesc.NumDescriptors = 200; // Same number as is used in scripting/Texture
    srvdesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
    if (FAILED(m_pd3d12Device->CreateDescriptorHeap(&srvdesc, IID_PPV_ARGS(&m_pd3dSrvDescHeap))))
    {
        Log::Error("D3D12::Initialize() - failed to create SRV descriptor heap!");
        return ResetState();
    }

    D3D12_DESCRIPTOR_HEAP_DESC rtvdesc;
    rtvdesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvdesc.NumDescriptors = buffersCounts;
    rtvdesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    rtvdesc.NodeMask = 1;
    if (FAILED(m_pd3d12Device->CreateDescriptorHeap(&rtvdesc, IID_PPV_ARGS(&m_pd3dRtvDescHeap))))
    {
        Log::Error("D3D12::Initialize() - failed to create RTV descriptor heap!");
        return ResetState();
    }

    const SIZE_T rtvDescriptorSize = m_pd3d12Device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_pd3dRtvDescHeap->GetCPUDescriptorHandleForHeapStart();
    for (UINT i = 0; i < buffersCounts; i++)
    {
        auto& context = m_frameContexts[i];
        context.MainRenderTargetDescriptor = rtvHandle;
        m_pdxgiSwapChain->GetBuffer(i, IID_PPV_ARGS(&context.BackBuffer));
        m_pd3d12Device->CreateRenderTargetView(context.BackBuffer.Get(), nullptr, context.MainRenderTargetDescriptor);
        rtvHandle.ptr += rtvDescriptorSize;
    }

    for (auto& context : m_frameContexts)
    {
        if (FAILED(m_pd3d12Device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&context.CommandAllocator))))
        {
            Log::Error("D3D12::Initialize() - failed to create command allocator!");
            return ResetState();
        }

        if (FAILED(m_pd3d12Device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, context.CommandAllocator.Get(), nullptr, IID_PPV_ARGS(&context.CommandList))) ||
            FAILED(context.CommandList->Close()))
        {
            Log::Error("D3D12::Initialize() - failed to create command list!");
            return ResetState();
        }
    }

    if (!InitializeImGui(buffersCounts))
    {
        Log::Error("D3D12::Initialize() - failed to initialize ImGui!");
        return ResetState();
    }

    Log::Info("D3D12::Initialize() - initialization successful!");
    m_initialized = true;

    OnInitialized.Emit();

    return true;
}

void D3D12::ReloadFonts()
{
    if (ImGui::GetCurrentContext() == nullptr)
        return;

    std::lock_guard _(m_imguiLock);

    // TODO - scale also by DPI
    const auto [resx, resy] = m_outSize;
    const auto scaleFromReference = std::min(static_cast<float>(resx) / 1920.0f, static_cast<float>(resy) / 1080.0f);

    auto& io = ImGui::GetIO();
    io.Fonts->Clear();

    ImFontConfig config;
    const auto& fontSettings = m_options.Font;
    config.SizePixels = std::floorf(fontSettings.BaseSize * scaleFromReference);
    config.OversampleH = fontSettings.OversampleHorizontal;
    config.OversampleV = fontSettings.OversampleVertical;
    if (config.OversampleH == 1 && config.OversampleV == 1)
        config.PixelSnapH = true;
    config.MergeMode = false;

    const auto& resolvedLang = CET::Get().GetLocalization().GetResolvedLanguage();
    const ImWchar* cpGlyphRanges = io.Fonts->GetGlyphRangesDefault();
    if (resolvedLang == "zh-cn")
        cpGlyphRanges = io.Fonts->GetGlyphRangesChineseSimplifiedCommon();
    else if (resolvedLang == "zh-tw")
        cpGlyphRanges = io.Fonts->GetGlyphRangesChineseFull();
    else if (resolvedLang == "jp-jp")
        cpGlyphRanges = io.Fonts->GetGlyphRangesJapanese();
    else if (resolvedLang == "kr-kr")
        cpGlyphRanges = io.Fonts->GetGlyphRangesKorean();
    else if (resolvedLang == "ru-ru" || resolvedLang == "ua-ua")
        cpGlyphRanges = io.Fonts->GetGlyphRangesCyrillic();
    else if (resolvedLang == "th-th")
        cpGlyphRanges = io.Fonts->GetGlyphRangesThai();

    // Prefer legacy Font.Path, else a system font family, else ImGui default.
    std::filesystem::path fontPath;
    if (!fontSettings.Path.empty())
    {
        fontPath = GetAbsolutePath(UTF8ToUTF16(fontSettings.Path), m_paths.Fonts(), false);
        if (fontPath.empty())
            Log::Warn("D3D12::ReloadFonts() - Custom font path is invalid! Falling back to system/default font.");
    }

    if (fontPath.empty())
    {
        std::wstring faceName;
        if (!m_options.UI.SystemFontFamily.empty())
            faceName = UTF8ToUTF16(m_options.UI.SystemFontFamily);
        else if (resolvedLang == "zh-cn" || resolvedLang == "zh-tw" || resolvedLang == "jp-jp" || resolvedLang == "kr-kr")
            faceName = L"Microsoft YaHei UI";
        else
            faceName = L"Segoe UI";

        fontPath = SystemFonts::FindFontFile(faceName);
        if (fontPath.empty() && faceName != L"Segoe UI")
            fontPath = SystemFonts::FindFontFile(L"Segoe UI");
    }

    if (!fontPath.empty())
        io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(fontPath.native()).c_str(), config.SizePixels, &config, cpGlyphRanges);
    else
    {
        Log::Warn("D3D12::ReloadFonts() - No usable system/custom font found; using ImGui default font.");
        io.Fonts->AddFontDefault(&config);
    }

    // Merge Material Design icons (bundled).
    config.MergeMode = true;
    config.GlyphMinAdvanceX = config.SizePixels;
    static const ImWchar icon_ranges[] = {ICON_MIN_MD, ICON_MAX_MD, 0};
    const auto cetIconPath = GetAbsolutePath(L"materialdesignicons.ttf", m_paths.Fonts(), false);
    if (!cetIconPath.empty())
        io.Fonts->AddFontFromFileTTF(UTF16ToUTF8(cetIconPath.native()).c_str(), config.SizePixels, &config, icon_ranges);
    else
        Log::Warn("D3D12::ReloadFonts() - Missing materialdesignicons.ttf!");
}

void D3D12::ApplyUIStyleAndFonts()
{
    Themes::Apply(m_styleReference, m_options.UI.Theme);

    {
        std::lock_guard _(m_imguiLock);
        if (ImGui::GetCurrentContext() != nullptr)
        {
            const auto [resx, resy] = m_outSize;
            const auto scaleFromReference = std::min(static_cast<float>(resx) / 1920.0f, static_cast<float>(resy) / 1080.0f);
            ImGui::GetStyle() = m_styleReference;
            ImGui::GetStyle().ScaleAllSizes(scaleFromReference);
        }
    }

    ReloadFonts();

    if (m_initialized && m_pCommandQueue)
    {
        ImGui_ImplDX12_InvalidateDeviceObjects();
        ImGui_ImplDX12_CreateDeviceObjects(m_pCommandQueue.Get());
    }
}

bool D3D12::InitializeImGui(size_t aBuffersCounts)
{
    std::lock_guard _(m_imguiLock);

    // TODO - scale also by DPI
    const auto [resx, resy] = m_outSize;
    const auto scaleFromReference = std::min(static_cast<float>(resx) / 1920.0f, static_cast<float>(resy) / 1080.0f);

    if (ImGui::GetCurrentContext() == nullptr)
    {
        // do this once, do not repeat context creation!
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
    }

    Themes::Apply(m_styleReference, m_options.UI.Theme);

    ImGui::GetStyle() = m_styleReference;
    ImGui::GetStyle().ScaleAllSizes(scaleFromReference);

    ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;

    if (!ImGui_ImplWin32_Init(m_window.GetWindow()))
    {
        Log::Error("D3D12::InitializeImGui() - ImGui_ImplWin32_Init call failed!");
        return false;
    }

    if (!ImGui_ImplDX12_Init(
            m_pd3d12Device.Get(), static_cast<int>(aBuffersCounts), DXGI_FORMAT_R8G8B8A8_UNORM, m_pd3dSrvDescHeap.Get(), m_pd3dSrvDescHeap->GetCPUDescriptorHandleForHeapStart(),
            m_pd3dSrvDescHeap->GetGPUDescriptorHandleForHeapStart()))
    {
        Log::Error("D3D12::InitializeImGui() - ImGui_ImplDX12_Init call failed!");
        ImGui_ImplWin32_Shutdown();
        return false;
    }

    // ReloadFonts also locks; mutex is recursive.
    ReloadFonts();

    if (!ImGui_ImplDX12_CreateDeviceObjects(m_pCommandQueue.Get()))
    {
        Log::Error("D3D12::InitializeImGui() - ImGui_ImplDX12_CreateDeviceObjects call failed!");
        ImGui_ImplDX12_Shutdown();
        ImGui_ImplWin32_Shutdown();
        return false;
    }

    return true;
}

void D3D12::PrepareUpdate()
{
    if (!m_initialized)
        return;

    std::lock_guard _(m_imguiLock);

    ImGui_ImplWin32_NewFrame(m_outSize);
    ImGui::NewFrame();

    CET::Get().GetOverlay().Update();

    CET::Get().GetVM().Draw();

    ImGui::Render();

    auto& drawData = m_imguiDrawDataBuffers[2];

    for (auto i = 0; i < drawData.CmdListsCount; ++i)
        IM_DELETE(drawData.CmdLists[i]);
    drawData.Clear();

    drawData = *ImGui::GetDrawData();

    ImVector<ImDrawList*> copiedDrawLists;
    copiedDrawLists.resize(drawData.CmdListsCount);

    for (auto i = 0; i < drawData.CmdListsCount; ++i)
        copiedDrawLists[i] = drawData.CmdLists[i]->CloneOutput();
    drawData.CmdLists = std::move(copiedDrawLists);

    std::swap(m_imguiDrawDataBuffers[1], m_imguiDrawDataBuffers[2]);
}

void D3D12::Update()
{
    // swap staging ImGui buffer with render ImGui buffer
    {
        std::lock_guard _(m_imguiLock);
        ImGui_ImplDX12_NewFrame(m_pCommandQueue.Get());
        if (m_imguiDrawDataBuffers[1].Valid)
        {
            std::swap(m_imguiDrawDataBuffers[0], m_imguiDrawDataBuffers[1]);
            m_imguiDrawDataBuffers[1].Valid = false;
        }
    }

    if (!m_imguiDrawDataBuffers[0].Valid)
        return;

    assert(m_pdxgiSwapChain);
    auto& frameContext = m_frameContexts[m_pdxgiSwapChain->GetCurrentBackBufferIndex()];
    frameContext.CommandAllocator->Reset();

    D3D12_RESOURCE_BARRIER barrier;
    barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
    barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
    barrier.Transition.pResource = frameContext.BackBuffer.Get();
    barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;

    ID3D12DescriptorHeap* heaps[] = {m_pd3dSrvDescHeap.Get()};

    frameContext.CommandList->Reset(frameContext.CommandAllocator.Get(), nullptr);
    frameContext.CommandList->ResourceBarrier(1, &barrier);
    frameContext.CommandList->SetDescriptorHeaps(1, heaps);
    frameContext.CommandList->OMSetRenderTargets(1, &frameContext.MainRenderTargetDescriptor, FALSE, nullptr);

    ImGui_ImplDX12_RenderDrawData(&m_imguiDrawDataBuffers[0], frameContext.CommandList.Get());

    barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
    barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
    frameContext.CommandList->ResourceBarrier(1, &barrier);
    frameContext.CommandList->Close();

    ID3D12CommandList* commandLists[] = {frameContext.CommandList.Get()};
    m_pCommandQueue->ExecuteCommandLists(1, commandLists);
}
