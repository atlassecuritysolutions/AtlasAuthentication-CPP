# Atlas Authentication — C++ SDK

[atlassecurity.site](https://atlassecurity.site) · [Docs](https://atlassecurity.site/docs) · [Plans](https://atlassecurity.site/plans) · [Discord](https://discord.gg/EG5dmpFaCF) · [mail@atlassecurity.site](mailto:mail@atlassecurity.site)

Hardware-bound license authentication and software protection for Windows x64 C++ applications. Two calls — `Atlas::Startup()` and `Atlas::Login(license)` — and your binary is authenticated, continuously protected, and killable in real time from the web dashboard.

This repo contains the SDK header, the prebuilt static library, and two runnable examples: a console app and a native DirectX 11 / Dear ImGui GUI.

---

## Contents

- [What ships in the box](#what-ships-in-the-box)
- [Repo layout](#repo-layout)
- [Prerequisites](#prerequisites)
- [Get an account, an app, a license](#get-an-account-an-app-a-license)
- [Console example](#console-example)
- [ImGui example](#imgui-example)
- [Integrate into your project](#integrate-into-your-project)
- [API reference](#api-reference)
- [What happens after `Login`](#what-happens-after-login)
- [The API-key model](#the-api-key-model)
- [Troubleshooting](#troubleshooting)
- [Pricing](#pricing)
- [Support](#support)
- [Legal](#legal)

---

## What ships in the box

- Ephemeral X25519 handshake and Ed25519-signed server reply on every connection — server impersonation is refused by construction.
- 5-second heartbeat with a rotating token, `.text` + IAT checks every 15 s, continuous inline-hook scan on `ws2_32.recv/send/connect`.
- Debugger, hardware breakpoint, injected-module, and manual-map detection.
- Mutual watchdog on two threads driven by hardware performance counters.
- On integrity failure, the process is ended via kernel `__fastfail()` — no dialog, no exception handler, nothing catchable.

---

## Repo layout

```
C++ Integration/
├── shared/
│   ├── Atlas.h                          the SDK header — the API you call
│   └── Atlas Auth.lib                   the static library — you link against this
├── Console Example/
│   ├── Atlas Auth Example.cpp           ~60 lines: Startup, Login, print session
│   ├── Atlas Auth Example.sln
│   └── Atlas Auth Example.vcxproj
└── ImGui Example/
    ├── README.md                        Dear ImGui vendoring step
    ├── Atlas Auth ImGui Example.cpp     native GUI login → welcome flow
    ├── Atlas Auth ImGui Example.sln
    ├── Atlas Auth ImGui Example.vcxproj
    └── imgui/                           ← you vendor Dear ImGui here (one-time)
```

`Atlas Auth.lib` is prebuilt and committed. You don't rebuild the SDK to use it.

---

## Prerequisites

| | |
|---|---|
| Windows 10 or 11 (x64) | Atlas is Windows-x64 only. No Linux, macOS, ARM. |
| [Visual Studio 2022](https://visualstudio.microsoft.com/vs/community/) | Community edition is fine. |
| **Desktop development with C++** workload | Installs MSVC v143, Windows SDK, MSBuild. |
| An Atlas account | [atlassecurity.site](https://atlassecurity.site) — free. |

No Boost, no vcpkg, no CMake, no redistributables. `Atlas Auth.lib` links statically; the resulting `.exe` runs on a stock Windows install.

---

## Get an account, an app, a license

1. Sign up at [atlassecurity.site](https://atlassecurity.site), verify your email.
2. **Applications → New application** — name it whatever, users never see it. Copy the **API key** it hands you.
3. **Licenses → Generate** — pick a duration (Weekly / Monthly / Lifetime / custom), a level (`1` for basic, `2+` for tiered), and optionally a note. Copy the key — format is `ATLAS-XXXXX-XXXXX`.

Free tier is 3 applications, 300 licenses across them, 3 file uploads per app.

---

## Console example

The minimum viable integration. Read it once and you know the SDK.

1. Open [`shared/Atlas.h`](shared/Atlas.h). Replace `"YOUR_API_KEY"` with your key. Save.
2. Open [`Console Example/Atlas Auth Example.sln`](Console%20Example/).
3. Set configuration to **Release · x64**. (32-bit will not link.)
4. Build: `Ctrl+Shift+B`.
5. Run: **`Ctrl+F5`**, not `F5`. `F5` attaches the VS debugger, which the anti-debug watchdog refuses.

Paste your license key when prompted. On success:

```
--- User Information ---
License: ATLAS-A9F2K-4RMXM
Expiry:  15-08-2026 14:32:00
IP:      45.11.42.187
HWID:    Atlas-4A9C...E1B2
Level:   1
Note:
Active Users: 1
Total Users:  3
```

Open the dashboard **Logs** tab — your login is there with IP, HWID, latency, and result `ALLOW`. Try **Sessions → Kick** — the example dies within 5 seconds via `__fastfail`. That's the loop.

The whole example lives in [`Console Example/Atlas Auth Example.cpp`](Console%20Example/Atlas%20Auth%20Example.cpp).

---

## ImGui example

A native Windows GUI: two-panel login → welcome flow, cold-steel design, Segoe UI, DirectX 11. Same SDK underneath, only the shell differs.

Dear ImGui is not shipped in this repo (MIT, upstream). Vendor it once:

```
cd "ImGui Example"
git submodule add https://github.com/ocornut/imgui.git imgui
git submodule update --init --recursive
```

Or download the source zip from [github.com/ocornut/imgui/releases](https://github.com/ocornut/imgui/releases) and drop it in `ImGui Example/imgui/` so the tree looks like:

```
ImGui Example/imgui/
├── imgui.cpp, imgui_draw.cpp, imgui_tables.cpp, imgui_widgets.cpp, imgui.h
└── backends/
    ├── imgui_impl_dx11.cpp   imgui_impl_dx11.h
    └── imgui_impl_win32.cpp  imgui_impl_win32.h
```

Reuse the same API key you set in `shared/Atlas.h`. Open the .sln, build **Release · x64**, run with `Ctrl+F5`. A 940×640 login window appears. Sign in and you land on a welcome screen with the full session card and **Sign out** / **Recheck session** buttons.

Fonts, styling, backend detail: [`ImGui Example/README.md`](ImGui%20Example/README.md).

---

## Integrate into your project

1. Copy [`shared/Atlas.h`](shared/Atlas.h) and [`shared/Atlas Auth.lib`](shared/Atlas%20Auth.lib) into your project (a `vendor/atlas/` folder is conventional).
2. In your Visual Studio project properties for **Release | x64**:
   - **C/C++ → General → Additional Include Directories** — add the folder holding `Atlas.h`
   - **Linker → General → Additional Library Directories** — same folder
   - **Linker → Input → Additional Dependencies** — add `Atlas Auth.lib;` (the filename literally has a space)
3. Set your API key inline in `Atlas.h`, or (recommended for shipping) from your own code before `Startup()`:
   ```cpp
   Atlas::API_KEY = LoadKeyFromSignedRemoteConfig();
   Atlas::Startup();
   ```
4. Wire the SDK:
   ```cpp
   #include "Atlas.h"

   int main() {
       Atlas::Startup();

       std::string license = PromptUserForLicense();
       if (!Atlas::Login(license)) {
           std::cout << Atlas::Data::GetErrorMessage();
           return 1;
       }

       RunMyApplication();  // authenticated
       return 0;
   }
   ```

From `Startup()` onward the SDK's own threads run the heartbeat, integrity checks, and watchdogs. You manage none of it.

Once you have a shipping build, compute its SHA-256 and paste it into **Applications → Executable-hash whitelist**. From that moment, modified copies are rejected server-side before the license is even checked. You can whitelist multiple hashes (one per release) and revoke old ones from the same panel.

---

## API reference

Everything callable lives in [`shared/Atlas.h`](shared/Atlas.h).

### Core

```cpp
void Atlas::Startup();                                   // call once at program start
bool Atlas::Login(const std::string& license_key);       // true on success, false on rejection
void Atlas::Logout();                                    // gentle sign-out; process stays alive
```

### `Atlas::Data` — session state (valid after `Login`)

```cpp
GetLicense()   GetHWID()    GetIP()     GetExpiry()   GetLevel()   GetNote()
GetFirstSeenDate()          GetLastSeenDate()
GetActiveUserCount()        GetUserCount()
IsAuthenticated()           IsBanned()
GetDaysRemaining()          IsLifetime()          IsExpiringSoon(days = 7)
GetErrorMessage()           HasError()            ClearError()
```

### `Atlas::Network` — server operations

```cpp
CheckAuthentication();               // force a fresh server round-trip
Download(int file_id);               // dashboard-uploaded file → std::vector<uint8_t>
BanUser(reason, minutes);            // minutes = 0 → permanent
SubmitLog(text);                     // custom log entry, ≤ 512 chars, shows in Logs
```

### `Atlas::Variables` — server-set config, no rebuild required

```cpp
Fetch("welcome_msg");        // string
FetchBool("beta_feature");   // bool
FetchInt("max_items");       // int
```

### `Atlas::Helper` — utilities

```cpp
SendDiscord(webhook, msg);
SendDiscordEmbed(webhook, title, desc, color);
SendWebhook(url, json);
Exit();        // kernel-level fastfail
Ping();        // round-trip ms to the auth server, -1 if unreachable
```

---

## What happens after `Login`

Runs inside the SDK's own threads. You write none of it.

- **5 s heartbeat** — signed with the per-session HMAC key, sequence-numbered, echoes the server's newest challenge nonce. Server can push messages, kick the session, or issue a hard terminate in the reply.
- **15 s deep sweep** — `.text` CRC vs the startup snapshot (catches NOPs, code caves, jump injection). Full IAT check (catches inline hooks, manual mapping).
- **Before every heartbeat** — first bytes of `ws2_32.recv/send/connect` scanned for hook signatures. A hooked network function is the foundation of a MitM on the auth channel; Atlas kills the process before any data crosses it.
- **Continuous** — executable-page-map diff vs the post-login snapshot (catches reflective DLL, manual map, shellcode); PEB + `NtQueryInformationProcess` + `DR0–DR7` + VEH debugger checks; mutual watchdog on two threads using hardware performance counters.

On any failure: `__fastfail()` from kernel. No dialog. No exception handler. No soft signal to intercept.

---

## The API-key model

The API key is a **routing identifier** — it says "send this request to my dashboard account." What actually authenticates every request:

1. The X25519 handshake — derives a per-session HMAC key only your app and the server know.
2. The Ed25519 signature the server puts on its handshake reply — verified against three keys pinned inside `Atlas Auth.lib` (primary, backup, emergency). A nulled server cannot produce these signatures.
3. The HWID binding — the session key is derived with the HWID mixed in; a stolen session token won't work from a different machine.
4. The per-request nonce — replays are dropped.
5. The executable-hash whitelist (if you configured one).

A leaked API key does not, by itself, let an attacker impersonate a user. Still, treat it as sensitive: rotate on suspected exposure (**Settings → Rotate key**) and keep it out of public source.

---

## Troubleshooting

**`LNK2019: unresolved external symbol "Atlas::Startup"`** — `Atlas Auth.lib` isn't in the linker inputs, or the library search path doesn't include its folder. See step 2 of integration.

**`error C2039: 'API_KEY': is not a member of 'Atlas'`** — you're compiling against an old `Atlas.h`. Copy the current one from `shared/`.

**Startup terminates the process immediately** — the SDK's kill path fired. Common causes: API key still `"YOUR_API_KEY"`; API key for a deleted app; debugger attached (test with `Ctrl+F5`). Check the dashboard **Logs** tab for the exact reason.

**`Login` returns `false`, "Executable hash mismatch"** — you whitelisted a hash then rebuilt. Update the whitelist, or don't whitelist during active development.

**`Login` returns `false`, "License banned" / "HWID banned"** — check **Bans**.

**Process exits silently, no message** — `__fastfail()` fired on an integrity check. Dashboard **Logs** shows the reason: `.text` modified, IAT hooked, injected module, server signature verification failed (nulled/MitM'd server), heartbeat couldn't reach the server for too long.

**The `.exe` is large** — `Atlas Auth.lib` is a ~15 MB static library; that's the whole protection stack. Compresses ~4:1 with UPX. Ship as-is unless you're size-constrained.

Full FAQ: [atlassecurity.site/docs](https://atlassecurity.site/docs).

---

## Pricing

**Free forever** — 3 applications, 300 licenses across them, 3 file uploads per app. Full security stack, no feature gates.

**Auth Premium** removes the caps:

| Term | Price | Save |
|---|---|---|
| Monthly | $19 | — |
| 6 months | $99 | 13% |
| 1 year | $149 | 35% |

**Atlas Complete** — Authentication + Obfuscator premium bundled: $39/month or $299/year. PayPal or crypto, instant activation. Full plan matrix at [atlassecurity.site/plans](https://atlassecurity.site/plans).

---

## Support

- **Docs** — [atlassecurity.site/docs](https://atlassecurity.site/docs)
- **Discord** — [discord.gg/EG5dmpFaCF](https://discord.gg/EG5dmpFaCF) (fastest response)
- **Email** — [mail@atlassecurity.site](mailto:mail@atlassecurity.site)

Bug reports: include the OS version, Visual Studio version, the failing SDK call, and the dashboard **Logs** entry if there is one.

To rebuild `Atlas Auth.lib` from source (only if you're modifying SDK internals): open `Auth Library/Atlas Auth/Atlas Auth.sln`, build **Release · x64** or **Ship-All · x64**, and the output overwrites `C++ Integration/shared/Atlas Auth.lib` in place.

---

## Legal

© 2025–2026 Atlas Security Solutions. All rights reserved. Sold by Atlas Security Solutions, Jeddah, Kingdom of Saudi Arabia.

This SDK exists so developers can integrate Atlas Authentication into their software. If that's you, use it freely.

**Prohibited without explicit written authorization:** reverse engineering, decompiling, disassembling, or reconstructing Atlas binaries, protocols, or server infrastructure; tampering with, bypassing, or disabling any authentication or anti-tamper control; probing or interfering with Atlas servers or databases; using knowledge of Atlas internals to build competing platforms or bypass tools.

Enforcement: Saudi Arabia Anti-Cybercrime Law (Royal Decree M/17, 1428H, Articles 3–4); U.S. Computer Fraud and Abuse Act (18 U.S.C. § 1030); EU Directive 2013/40/EU; WIPO / TRIPS (180+ signatory nations).

Atlas monitors for unauthorized access, reverse engineering, and protocol analysis. Violations are met with civil action, referral to competent authorities, and pursuit of all available remedies — injunctive relief, asset recovery, and cross-jurisdiction enforcement — without prior notice.

Permission requests and legal inquiries: [mail@atlassecurity.site](mailto:mail@atlassecurity.site) · [atlassecurity.site/legal](https://atlassecurity.site/legal)
