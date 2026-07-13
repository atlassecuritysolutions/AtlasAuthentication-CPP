# Atlas Authentication — ImGui Example

A native Windows GUI login flow written with Dear ImGui + DirectX 11.

Cold-steel palette from [atlassecurity.site](https://atlassecurity.site), real Segoe UI typography, split-panel form-on-left / brand-on-right, login → welcome flow with live session uptime. Same Atlas SDK as the [Console example](../Console%20Example/) — this one only adds the UI shell.

**For onboarding (create an Atlas account, get an API key, generate a license), see [`C++ Integration/README.md`](../README.md).** This file covers only what's specific to this example.

---

## What's here

```
ImGui Example/
├── README.md                              you are here
├── Atlas Auth ImGui Example.cpp           the whole example — login screen + welcome screen
├── Atlas Auth ImGui Example.sln           Visual Studio 2022 solution
├── Atlas Auth ImGui Example.vcxproj       MSBuild project (references imgui/ + backends/)
└── imgui/                                 ← YOU vendor Dear ImGui here (one-time)
```

The example links against `../shared/Atlas Auth.lib` and includes `../shared/Atlas.h` — same shared SDK as the console example. Nothing about the auth stack changes because you're rendering with ImGui.

---

## One-time setup — vendor Dear ImGui

Dear ImGui isn't shipped in this repo (it's MIT, upstream — no reason to fork it). Choose one:

### Option A — git submodule (recommended)

```
cd "ImGui Example"
git submodule add https://github.com/ocornut/imgui.git imgui
git submodule update --init --recursive
```

### Option B — download the release zip

Grab the latest source from [github.com/ocornut/imgui/releases](https://github.com/ocornut/imgui/releases). Extract into `ImGui Example/imgui/` so the tree looks like:

```
ImGui Example/
├── Atlas Auth ImGui Example.cpp
├── Atlas Auth ImGui Example.sln
├── Atlas Auth ImGui Example.vcxproj
├── README.md
└── imgui/                          ← added
    ├── imgui.cpp
    ├── imgui.h
    ├── imgui_draw.cpp
    ├── imgui_tables.cpp
    ├── imgui_widgets.cpp
    └── backends/
        ├── imgui_impl_dx11.cpp / .h
        └── imgui_impl_win32.cpp / .h
```

The .vcxproj references these files by name — if any are missing, the build will fail at link time with an obvious "unresolved external" for the ImGui symbols.

---

## Build

1. **Set your API key** in [`../shared/Atlas.h`](../shared/Atlas.h) (or leave the value you set for the console example — it's the same header):
   ```cpp
   namespace Atlas { inline std::string API_KEY = "your-key"; }
   ```
2. Open `Atlas Auth ImGui Example.sln` in Visual Studio 2022.
3. Configuration: **Release · x64** (Atlas is x64 only).
4. Build: `Ctrl+Shift+B`.

Output: `Atlas Auth ImGui Example.exe` in `x64/Release/`.

---

## Run

`F5` (with debugger) or `Ctrl+F5` (without). A 940×640 window opens with a cold-steel login screen:

- **Left panel** — license input, "Sign in" button, error banner on failed login
- **Right panel** — Atlas brand, live session uptime clock (updates every second)

Type your license key, click **Sign in**. On success the window transitions to a welcome screen with your session data — license, HWID, expiry, IP, level, note, active user count — and **Sign out** / **Recheck session** buttons.

Behind the UI, exactly the same protection stack that the console example runs:

- 5-second heartbeat
- 15-second `.text` + IAT sweep
- Continuous inline-hook / injection / debugger scan
- Watchdog mutual liveness
- `__fastfail()` on any integrity failure

The ImGui frontend is a rendering layer; the Atlas SDK doesn't care what's on top of it.

---

## Fonts and styling

The example loads Segoe UI directly from `%WINDIR%\Fonts\` — no font files shipped alongside the exe, no assets directory. Three sizes for real hierarchy:

- Body — 15px
- Heading — 24px
- Eyebrow (labels, meta) — 10.5px

If Segoe UI isn't present (very rare on Windows 10+), the code falls back to ImGui's built-in bitmap font. Ugly but functional.

The palette matches [atlassecurity.site](https://atlassecurity.site) pixel-for-pixel — ink `#080f1e`, panel `#0e1a30`, signal blue `#4aa3ec`, alert red `#f0817f`, ok green `#62cda0`. Defined as `ImVec4` constants at the top of `Atlas Auth ImGui Example.cpp` — swap them for your own brand colors if you fork this as a template.

---

## Dependencies

- **Dear ImGui** — MIT-licensed, vendored under `./imgui/` (see setup above)
- **`d3d11.lib`, `dxgi.lib`, `d3dcompiler.lib`** — shipped with the Windows 10/11 SDK
- **`../shared/Atlas Auth.lib`** — the Atlas SDK static library
- **`../shared/Atlas.h`** — the SDK header

Windows 10+ has DX11 built in. No runtime installs required on the target machine.

---

## Prefer the console version?

If you want zero UI dependency, use [`../Console Example/`](../Console%20Example/) instead — same Atlas SDK, ~60 lines of code, links the same shared library. Both examples are self-contained; pick whichever fits your app and delete the other.

---

## License / legal

See [`../README.md`](../README.md#legal-notice).
