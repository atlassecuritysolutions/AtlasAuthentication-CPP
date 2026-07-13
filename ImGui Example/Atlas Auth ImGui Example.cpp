// ============================================================================
// Atlas Authentication - Dear ImGui + DirectX 11 Example
//
// Structure taken from Dear ImGui's official example_win32_directx11 loop
// (which everyone links against and which provably works). Atlas SDK calls
// layered on top of the reference loop -- no changes to how ImGui + DX11
// initialize or how the event loop pumps.
//
// Look mirrors atlassecurity.site: cold-steel palette, three real font sizes
// (body / heading / eyebrow) loaded from Segoe UI, split-panel form-on-left
// brand-on-right, filled circle bullets drawn on the draw list. No asterisks,
// no default ImGui bitmap font, no drop-shadows.
//
// Set your API key in ../shared/Atlas.h  (Atlas::API_KEY inline std::string).
// ============================================================================

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>
#include <string>
#include <shellapi.h>

#include "Atlas.h"

#pragma comment(lib, "shell32.lib")

// ── Atlas cold-steel palette (source: atlassecurity.site) ────────────────
static const ImVec4 COL_INK        = ImVec4(0x08 / 255.f, 0x0f / 255.f, 0x1e / 255.f, 1.0f);
static const ImVec4 COL_PANEL      = ImVec4(0x0e / 255.f, 0x1a / 255.f, 0x30 / 255.f, 1.0f);
static const ImVec4 COL_RAISED     = ImVec4(0x14 / 255.f, 0x23 / 255.f, 0x3d / 255.f, 1.0f);
static const ImVec4 COL_RAISED_HI  = ImVec4(0x18 / 255.f, 0x2a / 255.f, 0x45 / 255.f, 1.0f);
static const ImVec4 COL_LINE       = ImVec4(0x22 / 255.f, 0x37 / 255.f, 0x5a / 255.f, 1.0f);
static const ImVec4 COL_LINE_SOFT  = ImVec4(0x17 / 255.f, 0x26 / 255.f, 0x3f / 255.f, 1.0f);
static const ImVec4 COL_HI         = ImVec4(0xf1 / 255.f, 0xf5 / 255.f, 0xfb / 255.f, 1.0f);
static const ImVec4 COL_LO         = ImVec4(0x93 / 255.f, 0xa7 / 255.f, 0xc4 / 255.f, 1.0f);
static const ImVec4 COL_FAINT      = ImVec4(0x56 / 255.f, 0x6d / 255.f, 0x8f / 255.f, 1.0f);
static const ImVec4 COL_SIGNAL     = ImVec4(0x4a / 255.f, 0xa3 / 255.f, 0xec / 255.f, 1.0f);
static const ImVec4 COL_SIGNAL_HI  = ImVec4(0x5f / 255.f, 0xb0 / 255.f, 0xe8 / 255.f, 1.0f);
static const ImVec4 COL_OK         = ImVec4(0x62 / 255.f, 0xcd / 255.f, 0xa0 / 255.f, 1.0f);
static const ImVec4 COL_ALERT      = ImVec4(0xf0 / 255.f, 0x81 / 255.f, 0x7f / 255.f, 1.0f);

// ── DX11 globals (identical to reference) ────────────────────────────────
static ID3D11Device*            g_pd3dDevice = nullptr;
static ID3D11DeviceContext*     g_pd3dDeviceContext = nullptr;
static IDXGISwapChain*          g_pSwapChain = nullptr;
static bool                     g_SwapChainOccluded = false;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView*  g_mainRenderTargetView = nullptr;

// ── Font handles ─────────────────────────────────────────────────────────
// Three sizes for real hierarchy: body 15px, heading 24px, eyebrow 10.5px.
// If Segoe UI isn't found (very rare on Windows), ImGui's default is used
// as a fallback, but the app still runs.
static ImFont* g_FontBody     = nullptr;
static ImFont* g_FontHeading  = nullptr;
static ImFont* g_FontEyebrow  = nullptr;
static ImFont* g_FontBodyBold = nullptr;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// ── App state ────────────────────────────────────────────────────────────
enum class Screen { Login, Welcome };
static Screen  g_screen = Screen::Login;
static char    g_licenseBuf[128] = "";
static std::string g_errorMsg;
static ULONGLONG g_sessionStart = 0;

struct WelcomeInfo {
    std::string license, level, expiry, hwid, ip, note, active, total;
} g_info;

// ── Style + fonts ────────────────────────────────────────────────────────
// Use Segoe UI from the Windows system font folder. It's present on every
// Windows install this SDK supports (7+), so no shipping fonts, no assets/
// dir next to the exe. If a customer's Windows install is nonstandard
// enough to lack Segoe UI we fall back to ImGui's default bitmap font so
// the app still runs -- ugly but functional.
static void LoadFonts(ImGuiIO& io) {
    char winDir[MAX_PATH] = {};
    GetWindowsDirectoryA(winDir, MAX_PATH);
    const std::string regular  = std::string(winDir) + "\\Fonts\\segoeui.ttf";
    const std::string semibold = std::string(winDir) + "\\Fonts\\seguisb.ttf";

    ImFontConfig cfg;
    cfg.OversampleH = 3;
    cfg.OversampleV = 2;
    cfg.PixelSnapH  = false;

    bool haveReg = GetFileAttributesA(regular.c_str())  != INVALID_FILE_ATTRIBUTES;
    bool haveSb  = GetFileAttributesA(semibold.c_str()) != INVALID_FILE_ATTRIBUTES;

    if (haveReg) {
        g_FontBody    = io.Fonts->AddFontFromFileTTF(regular.c_str(), 15.f,  &cfg);
        g_FontEyebrow = io.Fonts->AddFontFromFileTTF(regular.c_str(), 10.5f, &cfg);
    } else {
        g_FontBody    = io.Fonts->AddFontDefault();
        g_FontEyebrow = g_FontBody;
    }
    if (haveSb) {
        g_FontHeading  = io.Fonts->AddFontFromFileTTF(semibold.c_str(), 24.f, &cfg);
        g_FontBodyBold = io.Fonts->AddFontFromFileTTF(semibold.c_str(), 15.f, &cfg);
    } else {
        g_FontHeading  = g_FontBody;
        g_FontBodyBold = g_FontBody;
    }
    io.FontDefault = g_FontBody;
}

static void ApplyAtlasStyle() {
    ImGuiStyle& s = ImGui::GetStyle();
    s.WindowPadding      = ImVec2(0, 0);
    s.FramePadding       = ImVec2(14, 11);
    s.ItemSpacing        = ImVec2(0, 8);
    s.ItemInnerSpacing   = ImVec2(6, 6);
    s.WindowRounding     = 0.f;
    s.ChildRounding      = 0.f;
    s.FrameRounding      = 8.f;
    s.PopupRounding      = 8.f;
    s.WindowBorderSize   = 0.f;
    s.FrameBorderSize    = 1.f;
    s.ChildBorderSize    = 0.f;
    s.ScrollbarSize      = 12.f;

    ImVec4* c = s.Colors;
    c[ImGuiCol_WindowBg]           = COL_INK;
    c[ImGuiCol_ChildBg]            = COL_PANEL;
    c[ImGuiCol_PopupBg]            = COL_PANEL;
    c[ImGuiCol_Border]             = COL_LINE;
    c[ImGuiCol_FrameBg]            = COL_RAISED;
    c[ImGuiCol_FrameBgHovered]     = COL_RAISED_HI;
    c[ImGuiCol_FrameBgActive]      = COL_RAISED_HI;
    c[ImGuiCol_Text]               = COL_HI;
    c[ImGuiCol_TextDisabled]       = COL_FAINT;
    c[ImGuiCol_Button]             = COL_SIGNAL;
    c[ImGuiCol_ButtonHovered]      = COL_SIGNAL_HI;
    c[ImGuiCol_ButtonActive]       = COL_SIGNAL_HI;
    c[ImGuiCol_Separator]          = COL_LINE_SOFT;
}

// Text with a specific colour, no PushStyleColor bookkeeping in the caller.
static void TextC(const ImVec4& col, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    ImGui::PushStyleColor(ImGuiCol_Text, col);
    ImGui::TextV(fmt, args);
    ImGui::PopStyleColor();
    va_end(args);
}
static void TextWrappedC(const ImVec4& col, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    ImGui::PushStyleColor(ImGuiCol_Text, col);
    ImGui::TextWrappedV(fmt, args);
    ImGui::PopStyleColor();
    va_end(args);
}

// Draw a filled square badge with a single-letter glyph inside. Used for the
// brand-mark "A" -- looks like the atlassecurity.site logo mark.
static void DrawBrandMark(const char* letter, const char* name) {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 p = ImGui::GetCursorScreenPos();
    const float box = 24.f;
    // Filled signal-coloured square with rounded corners.
    dl->AddRectFilled(p, ImVec2(p.x + box, p.y + box),
                      ImGui::GetColorU32(COL_SIGNAL), 5.f);
    // Letter centered in the square.
    ImGui::PushFont(g_FontBodyBold);
    ImVec2 sz = ImGui::CalcTextSize(letter);
    dl->AddText(ImVec2(p.x + (box - sz.x) * 0.5f, p.y + (box - sz.y) * 0.5f + 0.5f),
                ImGui::GetColorU32(COL_INK), letter);
    ImGui::PopFont();
    // Reserve the space so following widgets flow past the mark.
    ImGui::Dummy(ImVec2(box, box));
    ImGui::SameLine(0, 12);
    // Name aligned vertically with the box centre.
    ImVec2 nameP = ImGui::GetCursorScreenPos();
    ImGui::SetCursorScreenPos(ImVec2(nameP.x, nameP.y + 3.f));
    ImGui::PushFont(g_FontBodyBold);
    TextC(COL_HI, "%s", name);
    ImGui::PopFont();
}

// Draw a small filled circle at the given cursor position and advance past it.
// Replaces the "text asterisk" bullet that reads like a comment.
static void BulletDot() {
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 p = ImGui::GetCursorScreenPos();
    const float r = 3.f;
    dl->AddCircleFilled(ImVec2(p.x + r + 2.f, p.y + 9.f), r,
                        ImGui::GetColorU32(COL_SIGNAL));
    ImGui::Dummy(ImVec2(r * 2.f + 10.f, 0));
    ImGui::SameLine(0, 0);
}

// A "hyperlink"-style text: underlined on hover, opens in browser on click.
// Real ImGui doesn't have a hyperlink widget; this is the standard idiom.
static void Hyperlink(const char* label, const char* url) {
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImVec2 sz = ImGui::CalcTextSize(label);
    ImGui::PushStyleColor(ImGuiCol_Text, COL_SIGNAL);
    ImGui::TextUnformatted(label);
    ImGui::PopStyleColor();
    bool hovered = ImGui::IsItemHovered();
    if (hovered) {
        ImGui::GetWindowDrawList()->AddLine(
            ImVec2(p.x, p.y + sz.y),
            ImVec2(p.x + sz.x, p.y + sz.y),
            ImGui::GetColorU32(COL_SIGNAL), 1.f);
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
    }
    if (hovered && ImGui::IsMouseClicked(0)) {
        ShellExecuteA(nullptr, "open", url, nullptr, nullptr, SW_SHOWNORMAL);
    }
}

// Fetch user info after successful login
static void FetchWelcomeInfo(const char* licenseInput) {
    g_info.license = licenseInput;
    g_info.level   = Atlas::Data::GetLevel();
    g_info.expiry  = Atlas::Data::GetExpiry();
    g_info.hwid    = Atlas::Data::GetHWID();
    g_info.ip      = Atlas::Data::GetIP();
    g_info.note    = Atlas::Data::GetNote();
    g_info.active  = Atlas::Data::GetActiveUserCount();
    g_info.total   = Atlas::Data::GetUserCount();
}

// ── The Atlas UI - split-panel form-on-left brand-on-right ───────────────
static void RenderAtlasWindow(const ImGuiIO& io) {
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(io.DisplaySize);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Atlas", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove     | ImGuiWindowFlags_NoScrollbar |
                 ImGuiWindowFlags_NoBringToFrontOnFocus);

    const float totalW = io.DisplaySize.x;
    const float totalH = io.DisplaySize.y;
    // Single centred column -- no split, no side panel. The form is the
    // whole example. A max width keeps it readable on wide monitors.
    const float formW  = totalW < 520.f ? totalW : 460.f;
    const float formX  = (totalW - formW) * 0.5f;
    const float padY   = 48.f;

    ImGui::SetCursorPos(ImVec2(0, 0));
    ImGui::PushStyleColor(ImGuiCol_ChildBg, COL_INK);
    ImGui::BeginChild("form_pane", ImVec2(totalW, totalH), 0);
    {
        // Brand-mark, top-left, aligned to the form column.
        ImGui::SetCursorPos(ImVec2(formX, padY));
        DrawBrandMark("A", "Atlas");

        // All form widgets go inside a nested child anchored at (formX, padY+100).
        // Inside this child, cursor-X = 0 means "aligned to the form column",
        // so every following widget lines up without manual SetCursorPosX.
        ImGui::SetCursorPos(ImVec2(formX, padY + 100.f));
        ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0,0,0,0));
        ImGui::BeginChild("form_col", ImVec2(formW, totalH - padY * 2 - 100.f), 0);
        ImGui::PushItemWidth(formW);

        if (g_screen == Screen::Login) {
            ImGui::PushFont(g_FontHeading);
            TextC(COL_HI, "Sign in");
            ImGui::PopFont();

            ImGui::Dummy(ImVec2(0, 6));
            ImGui::PushTextWrapPos(formW);
            TextWrappedC(COL_LO, "Your license is checked against the Atlas server, tied to this machine's hardware, and re-verified every five seconds while the program runs.");
            ImGui::PopTextWrapPos();

            ImGui::Dummy(ImVec2(0, 28));
            ImGui::PushFont(g_FontEyebrow);
            TextC(COL_FAINT, "LICENSE KEY");
            ImGui::PopFont();
            ImGui::Dummy(ImVec2(0, 4));

            ImGui::InputTextWithHint("##license", "ATLAS-XXXXX-XXXXX-XXXXX",
                                      g_licenseBuf, sizeof(g_licenseBuf));

            ImGui::Dummy(ImVec2(0, 14));

            ImGui::PushStyleColor(ImGuiCol_Text, COL_INK);
            ImGui::PushFont(g_FontBodyBold);
            if (ImGui::Button("Authenticate", ImVec2(formW, 44)) ||
                ImGui::IsKeyPressed(ImGuiKey_Enter)) {
                g_errorMsg.clear();
                if (g_licenseBuf[0] == '\0') {
                    g_errorMsg = "Enter a license key.";
                } else if (Atlas::Login(g_licenseBuf)) {
                    FetchWelcomeInfo(g_licenseBuf);
                    g_sessionStart = GetTickCount64();
                    g_screen = Screen::Welcome;
                } else {
                    std::string em = Atlas::Data::GetErrorMessage();
                    g_errorMsg = em.empty() ? "Authentication rejected by the server." : em;
                }
            }
            ImGui::PopFont();
            ImGui::PopStyleColor();

            if (!g_errorMsg.empty()) {
                ImGui::Dummy(ImVec2(0, 14));
                ImGui::PushTextWrapPos(formW);
                TextWrappedC(COL_ALERT, "%s", g_errorMsg.c_str());
                ImGui::PopTextWrapPos();
            }
        }
        else {  // Welcome
            ImGui::PushFont(g_FontHeading);
            TextC(COL_HI, "Signed in");
            ImGui::PopFont();

            ImGui::Dummy(ImVec2(0, 6));
            ImGui::PushTextWrapPos(formW);
            TextWrappedC(COL_LO, "Your session is holding. The Atlas heartbeat keeps re-verifying it every five seconds; you don't need to call anything to keep it alive.");
            ImGui::PopTextWrapPos();

            ImGui::Dummy(ImVec2(0, 20));

            // Info card sized to fit its content exactly (no fixed height,
            // no scrolling, no clipping).
            struct Row { const char* k; const char* v; };
            Row rows[] = {
                { "License", g_info.license.c_str() },
                { "Level",   g_info.level.c_str()   },
                { "Expiry",  g_info.expiry.c_str()  },
                { "HWID",    g_info.hwid.c_str()    },
                { "IP",      g_info.ip.c_str()      },
            };
            const int   rowCount = (int)(sizeof(rows) / sizeof(rows[0]));
            const float rowH     = ImGui::GetTextLineHeight() + 6.f;
            const float cardH    = rowCount * rowH + 24.f;

            ImGui::PushStyleColor(ImGuiCol_ChildBg, COL_RAISED);
            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 10.f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(16, 12));
            ImGui::BeginChild("info_card", ImVec2(formW, cardH),
                              true, ImGuiWindowFlags_NoScrollbar);
            {
                for (auto& r : rows) {
                    ImGui::PushFont(g_FontEyebrow);
                    TextC(COL_FAINT, "%s", r.k);
                    ImGui::PopFont();
                    ImGui::SameLine(96);
                    TextC(COL_HI, "%s", (r.v && r.v[0]) ? r.v : "-");
                }
            }
            ImGui::EndChild();
            ImGui::PopStyleVar(2);
            ImGui::PopStyleColor();

            ImGui::Dummy(ImVec2(0, 16));

            // Ghost buttons -- transparent + line.
            ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0,0,0,0));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, COL_RAISED);
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  COL_RAISED_HI);
            ImGui::PushStyleColor(ImGuiCol_Text,          COL_LO);
            ImGui::PushStyleColor(ImGuiCol_Border,        COL_LINE);
            if (ImGui::Button("Sign out", ImVec2(120, 36))) {
                Atlas::Network::SubmitLog("User signed out from ImGui example");
                Atlas::Logout();
                g_screen = Screen::Login;
                g_licenseBuf[0] = '\0';
                g_errorMsg.clear();
            }
            ImGui::SameLine(0, 8);
            if (ImGui::Button("Recheck session", ImVec2(160, 36))) {
                if (!Atlas::Network::CheckAuthentication()) {
                    g_screen = Screen::Login;
                    g_errorMsg = "Session is no longer valid.";
                    g_licenseBuf[0] = '\0';
                }
            }
            ImGui::PopStyleColor(5);
        }

        ImGui::PopItemWidth();
        ImGui::EndChild();     // form_col
        ImGui::PopStyleColor(); // form_col ChildBg

        // Website link — big, obvious, blue. This is the app footer: users
        // should always know where the real product lives.
        const float linkH = 30.f;
        ImGui::SetCursorPos(ImVec2(formX, totalH - padY - linkH));
        ImGui::PushFont(g_FontBodyBold);
        const char* linkLabel = "atlassecurity.site";
        ImVec2 linkSz = ImGui::CalcTextSize(linkLabel);
        ImVec2 linkPos = ImGui::GetCursorScreenPos();
        // Draw the text + a permanent underline so it reads as a hyperlink.
        ImGui::TextUnformatted(linkLabel);
        ImDrawList* fdl = ImGui::GetWindowDrawList();
        fdl->AddLine(ImVec2(linkPos.x, linkPos.y + linkSz.y + 1.f),
                     ImVec2(linkPos.x + linkSz.x, linkPos.y + linkSz.y + 1.f),
                     ImGui::GetColorU32(COL_SIGNAL), 1.f);
        // Overlay an invisible button so the whole text is clickable.
        ImGui::SetCursorScreenPos(linkPos);
        if (ImGui::InvisibleButton("##site", ImVec2(linkSz.x, linkSz.y + 4.f))) {
            ShellExecuteA(nullptr, "open", "https://atlassecurity.site", nullptr, nullptr, SW_SHOWNORMAL);
        }
        bool linkHovered = ImGui::IsItemHovered();
        if (linkHovered) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        // Repaint the text on top so the colour is right (hover = brighter).
        fdl->AddText(linkPos,
                     ImGui::GetColorU32(linkHovered ? COL_SIGNAL_HI : COL_SIGNAL),
                     linkLabel);
        ImGui::PopFont();
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();

    ImGui::End();
    ImGui::PopStyleVar(2);
}

// ============================================================================
// Main code (structure IDENTICAL to imgui/examples/example_win32_directx11)
// ============================================================================
int APIENTRY WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
{
    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"Atlas", nullptr };
    ::RegisterClassExW(&wc);
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"Atlas", WS_OVERLAPPEDWINDOW,
                                CW_USEDEFAULT, CW_USEDEFAULT, 940, 640,
                                nullptr, nullptr, wc.hInstance, nullptr);

    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;

    LoadFonts(io);
    ApplyAtlasStyle();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    // Start Atlas AFTER DX11 + ImGui have loaded their runtime libraries.
    // Atlas's watchdog snapshots exec pages at Startup and later kills the
    // process if new pages appear. Loading GPU + ImGui runtimes first puts
    // them in the baseline instead of triggering a false-positive kill.
    Atlas::Startup();

    bool done = false;
    while (!done)
    {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;

        if (g_SwapChainOccluded && g_pSwapChain->Present(0, DXGI_PRESENT_TEST) == DXGI_STATUS_OCCLUDED)
        {
            ::Sleep(10);
            continue;
        }
        g_SwapChainOccluded = false;

        if (g_ResizeWidth != 0 && g_ResizeHeight != 0)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, g_ResizeWidth, g_ResizeHeight, DXGI_FORMAT_UNKNOWN, 0);
            g_ResizeWidth = g_ResizeHeight = 0;
            CreateRenderTarget();
        }

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        RenderAtlasWindow(io);

        ImGui::Render();
        const float clear[4] = { COL_INK.x, COL_INK.y, COL_INK.z, COL_INK.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        HRESULT hr = g_pSwapChain->Present(1, 0);
        g_SwapChainOccluded = (hr == DXGI_STATUS_OCCLUDED);
    }

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
    return 0;
}

// ── DX11 helpers (identical to reference) ────────────────────────────────
bool CreateDeviceD3D(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
        createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION,
        &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED)
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr,
            createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION,
            &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam);
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
