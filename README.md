# Atlas Authentication — C++ SDK

**Website** · [atlassecurity.site](https://atlassecurity.site) &nbsp;|&nbsp; **Docs** · [atlassecurity.site/docs](https://atlassecurity.site/docs) &nbsp;|&nbsp; **Plans** · [atlassecurity.site/plans](https://atlassecurity.site/plans) &nbsp;|&nbsp; **Discord** · [discord.gg/EG5dmpFaCF](https://discord.gg/EG5dmpFaCF) &nbsp;|&nbsp; **Email** · [mail@atlassecurity.site](mailto:mail@atlassecurity.site)

The official C++ SDK for [Atlas Authentication](https://atlassecurity.site) — a hardware-bound license authentication and software protection platform for Windows x64. This repo ships the static library, the SDK header, and two runnable examples: a minimal console app and a native GUI login flow built on Dear ImGui + DirectX 11.

Two SDK calls — `Atlas::Startup()` and `Atlas::Login(license)` — and your compiled binary is authenticated, continuously protected, and killable in real time from a web dashboard.

---

## Contents

- [What Atlas is](#what-atlas-is)
- [What's in this repo](#whats-in-this-repo)
- [Prerequisites](#prerequisites)
- [Step 1 — Create your Atlas account](#step-1--create-your-atlas-account)
- [Step 2 — Register your application](#step-2--register-your-application)
- [Step 3 — Generate a license key](#step-3--generate-a-license-key)
- [Step 4a — Build and run the Console example](#step-4a--build-and-run-the-console-example)
- [Step 4b — Build and run the ImGui example](#step-4b--build-and-run-the-imgui-example)
- [Step 5 — Add Atlas to *your* project](#step-5--add-atlas-to-your-project)
- [API reference](#api-reference)
- [What runs automatically after `Login`](#what-runs-automatically-after-login)
- [Dashboard — the operator side](#dashboard--the-operator-side)
- [Troubleshooting](#troubleshooting)
- [Support](#support)
- [Legal notice](#legal-notice)

---

## What Atlas is

Atlas is a **hardware-bound license authentication and software protection platform** for Windows x64 software. Same category as KeyAuth / Auth.gg / sentinel — but done properly, with active protection that runs *during* the session, not just at login.

**Two SDK calls in — full protection stack out:**

- Heartbeat every 5 seconds with a fresh nonce and a rotating session token
- `.text` CRC and IAT integrity check every 15 seconds against a startup snapshot
- Continuous inline-hook scan on `ws2_32.recv/send/connect` before every heartbeat
- Injected-module detection, debugger detection, hardware breakpoint (`DR0–DR7`) inspection
- Mutual watchdog on two threads using hardware performance counters
- On any failure, the process ends via `__fastfail()` at kernel level — no dialog, no exception handler to catch it, no soft signal to intercept

Free tier for life, no credit card, full security stack included (limited to 3 apps · 300 licenses · 3 files per app). Premium removes all caps and starts at **$9/week** or **$19/month** ([see plans](https://atlassecurity.site/plans)).

---

## What's in this repo

```
C++ Integration/
├── README.md                            you are here
├── shared/
│   ├── Atlas.h                            the SDK header — the API surface you call
│   └── Atlas Auth.lib                     the static library — you link against this
├── Console Example/
│   ├── Atlas Auth Example.cpp             ~60 lines: Startup, Login, print session
│   ├── Atlas Auth Example.sln             Visual Studio 2022 solution
│   └── Atlas Auth Example.vcxproj         MSBuild project (already wired for the .lib)
└── ImGui Example/
    ├── README.md                          per-example setup (Dear ImGui vendor step)
    ├── Atlas Auth ImGui Example.cpp       native GUI login → welcome flow
    ├── Atlas Auth ImGui Example.sln
    ├── Atlas Auth ImGui Example.vcxproj
    └── imgui/                             ← you vendor Dear ImGui here (one-time)
```

**Prebuilt.** `Atlas Auth.lib` is committed. You do not need to rebuild the SDK from source to use it. If you want to (because you modified the SDK), see [Rebuilding `Atlas Auth.lib` from source](#rebuilding-atlas-authlib-from-source) at the bottom.

---

## Prerequisites

Everything on this list is one download and covers both examples.

| Requirement | Why | How to get it |
|---|---|---|
| **Windows 10 or 11 (x64)** | Atlas is Windows-x64 only. No Linux, no macOS, no ARM. | — |
| **Visual Studio 2022** | Builds the examples. Community edition is fine. | [visualstudio.microsoft.com](https://visualstudio.microsoft.com/vs/community/) |
| **Desktop development with C++** workload | Installs MSVC v143, Windows 10/11 SDK, MSBuild | Toggle it on in the Visual Studio Installer |
| **Windows 10/11 SDK** (10.0.x) | System headers, `d3d11.lib`, `wininet.lib`, `ws2_32.lib` | Bundled with the workload above |
| **An Atlas account** | Gives you an API key and license keys to test with | [atlassecurity.site](https://atlassecurity.site) — free |

That's the full list. No Boost, no vcpkg, no CMake, no runtime redistributables. `Atlas Auth.lib` links statically; the resulting `.exe` runs on a stock Windows install.

---

## Step 1 — Create your Atlas account

1. Go to [atlassecurity.site](https://atlassecurity.site) and click **Get started**.
2. Sign up with email or Discord.
3. Verify your email (the dashboard is locked until you do).
4. You're in.

Free tier — no credit card. You get 3 applications, 300 licenses across them, and 3 file uploads per app. That's plenty to build, test, and ship a small product.

---

## Step 2 — Register your application

In the dashboard, open **Applications → New application**. Give it a name — this is internal, users never see it.

Atlas creates the record and shows you:

- **API key** — copy this now, you'll paste it into `Atlas.h` in step 4
- **Application ID** — internal, the SDK reads it via the API key
- **Executable-hash whitelist** — optional at this stage; you'll add your compiled `.exe`'s SHA-256 here later so a modified copy is rejected server-side

The API key is a **routing identifier**, not a bearer secret — it says "send this request to my dashboard account." What actually authenticates is the HMAC over the frame body, the HWID binding, and the fresh per-request nonce. See [The API-key model](#the-api-key-model) for the full contract.

---

## Step 3 — Generate a license key

**Licenses → Generate.** Pick:

- **Duration** — Weekly, Monthly, Lifetime, or custom days
- **Level** — `1` for basic, `2+` for tiered access; your code reads this via `Atlas::Data::GetLevel()`
- **Note** *(optional)* — anything you want, shown in the user's session via `GetNote()`

Copy the generated key. It looks like `ATLAS-A9F2-K4RM-XM7K`. This is what your end users will type into your app.

For testing, give the key to yourself — you'll type it into the example in the next step.

---

## Step 4a — Build and run the Console example

The console example is the minimum viable Atlas integration. Read it once and you understand the whole SDK.

### 4a.1 — Paste your API key

Open [`shared/Atlas.h`](shared/Atlas.h) in Visual Studio (or any editor). Near the top:

```cpp
namespace Atlas {
    // Get your API key from atlassecurity.site/dashboard after creating an application
    inline std::string API_KEY = "YOUR_API_KEY";
```

Replace `"YOUR_API_KEY"` with the key you copied in step 2. Save.

### 4a.2 — Open the solution

Double-click [`Console Example/Atlas Auth Example.sln`](Console%20Example/) — Visual Studio 2022 opens the project.

### 4a.3 — Set configuration to Release · x64

Top toolbar: change **Debug** → **Release**, and confirm the platform is **x64**. Atlas is x64 only; a 32-bit build will not link.

### 4a.4 — Build

`Ctrl+Shift+B` (or **Build → Build Solution**). Output at the bottom should end with `1 succeeded, 0 failed`. The compiled `.exe` lands in `Console Example/x64/Release/Atlas Auth Example.exe`.

### 4a.5 — Run

Hit `F5` to run under the debugger, or `Ctrl+F5` to run without one. A console window opens:

```
Atlas Authentication Example

Enter license:
```

Paste the license key from step 3, press Enter. If everything's wired correctly, you'll see:

```
Attempting to connect to server...

--- User Information ---
License: ATLAS-A9F2-K4RM-XM7K
Expiry: 15-08-2026 14:32:00
IP: 45.11.42.187
HWID: 4A9C...E1B2
Level: 1
Note:
Active Users: 1
Total Users: 3
```

**Now check the dashboard.** Open **Logs** — you should see the login entry with your IP, HWID, latency, and result = `ALLOW`. Try **Sessions → Kick session** — the example terminates within 5 seconds via `__fastfail`. That's the full loop.

### 4a.6 — Read the code

The whole example is [`Console Example/Atlas Auth Example.cpp`](Console%20Example/Atlas%20Auth%20Example.cpp). Read it top to bottom — it's ~60 lines, no fluff, and every SDK call it makes is one you'll use in your own app.

---

## Step 4b — Build and run the ImGui example

The ImGui example is a native Windows GUI: two-panel login → welcome flow, cold-steel design that mirrors the Atlas dashboard, Segoe UI, DirectX 11 rendering. Same Atlas SDK underneath — this only adds the UI shell.

### 4b.1 — Vendor Dear ImGui (one-time)

Dear ImGui isn't shipped in this repo (it's MIT, upstream, and we don't want to fork it). Choose one:

**Option A — git submodule (recommended for reproducible builds):**
```
cd "ImGui Example"
git submodule add https://github.com/ocornut/imgui.git imgui
git submodule update --init --recursive
```

**Option B — download the release zip:**
Grab the latest source from [github.com/ocornut/imgui/releases](https://github.com/ocornut/imgui/releases). Extract into `ImGui Example/imgui/` so the file tree looks like:

```
ImGui Example/
├── Atlas Auth ImGui Example.cpp
├── Atlas Auth ImGui Example.sln
├── Atlas Auth ImGui Example.vcxproj
└── imgui/                          ← you added this
    ├── imgui.cpp
    ├── imgui.h
    ├── imgui_draw.cpp
    ├── imgui_tables.cpp
    ├── imgui_widgets.cpp
    └── backends/
        ├── imgui_impl_dx11.cpp / .h
        └── imgui_impl_win32.cpp / .h
```

### 4b.2 — API key, build, run

Same as the console example:

1. Set your API key in `shared/Atlas.h` (already done in step 4a — reuse the same one)
2. Open `Atlas Auth ImGui Example.sln`
3. Configuration: **Release · x64**
4. Build (`Ctrl+Shift+B`)
5. Run (`F5` or `Ctrl+F5`)

You'll see a 940×640 window with a cold-steel login form. Type your license, click **Sign in**. On success you land on a welcome screen with license/HWID/expiry/IP/level/note, a session uptime clock, and **Sign out** / **Recheck session** buttons.

That's it. Same protection stack runs regardless of the UI you put on top.

More detail (fonts, styling, backend files) lives in [`ImGui Example/README.md`](ImGui%20Example/README.md).

---

## Step 5 — Add Atlas to *your* project

You've verified the examples work. Now the real integration.

### 5.1 — Copy the SDK files

Copy these two files into your project:

- [`shared/Atlas.h`](shared/Atlas.h) — the SDK header
- [`shared/Atlas Auth.lib`](shared/Atlas%20Auth.lib) — the static library (~15 MB)

A common layout inside your project:

```
YourProject/
├── src/
│   └── main.cpp
├── vendor/
│   └── atlas/
│       ├── Atlas.h              ← copied from shared/
│       └── Atlas Auth.lib       ← copied from shared/
└── YourProject.vcxproj
```

### 5.2 — Point Visual Studio at those files

In your Visual Studio project **Properties** (for the `Release | x64` configuration):

- **C/C++ → General → Additional Include Directories** — add `$(ProjectDir)vendor\atlas` (or wherever you put `Atlas.h`)
- **Linker → General → Additional Library Directories** — add the same path
- **Linker → Input → Additional Dependencies** — add `Atlas Auth.lib;` (with the space, the filename is literally `Atlas Auth.lib`)

That's the whole build wiring. No preprocessor macros, no runtime dependencies. `Atlas Auth.lib` links every Windows system dep it needs internally.

### 5.3 — Set your API key

Two options:

**Option A — inline in `Atlas.h`** (what the examples do; fine for a first integration):
```cpp
namespace Atlas {
    inline std::string API_KEY = "your-actual-key-here";
    // ...
```

**Option B — set it from your own code before `Startup()`** (recommended for shipping — keeps the key out of a shared header):
```cpp
// somewhere in your app's startup, before Atlas::Startup()
Atlas::API_KEY = LoadKeyFromSignedRemoteConfig();
Atlas::Startup();
```

### 5.4 — Call the SDK

```cpp
#include "Atlas.h"

int main() {
    Atlas::Startup();                       // initialize protection stack

    std::string license = PromptUserForLicense();
    if (!Atlas::Login(license)) {
        std::cout << Atlas::Data::GetErrorMessage();
        return 1;
    }

    // authenticated — your app code runs here
    RunMyApplication();
    return 0;
}
```

Full flow, exactly this simple. From `Startup()` onwards the SDK's own threads are running heartbeats, integrity checks, and watchdogs — you don't manage any of that.

### 5.5 — Register your compiled `.exe`'s hash

Once you have a shipping build, compute its SHA-256 and paste it into the dashboard's **Applications → Executable-hash whitelist**. From that moment, patched or modified copies are rejected server-side *before* the license is checked.

You can whitelist multiple hashes — one per release build. Old versions can be revoked from the same panel.

---

## API reference

Everything callable is declared in [`shared/Atlas.h`](shared/Atlas.h). Quick tour:

### Core

```cpp
void Atlas::Startup();
    // Initialize the protection stack. Call ONCE at program start,
    // before any other Atlas function. Idempotent.

bool Atlas::Login(const std::string& license_key);
    // Authenticate against the server. Returns true on success,
    // false on rejection. Read Data::GetErrorMessage() for the reason.

void Atlas::Logout();
    // Gentle sign-out. Tears down the session server-side, closes the
    // socket, zeroes credentials. Process stays alive. Use this when
    // your user clicks a "Sign out" button.
```

### Data — user & session info (available after `Login`)

```cpp
Atlas::Data::GetLicense()             // "ATLAS-XXXX-XXXX-XXXX"
Atlas::Data::GetHWID()                // hardware fingerprint hash
Atlas::Data::GetIP()                  // detected by the server
Atlas::Data::GetExpiry()              // "15-08-2026 14:32:00" or "Lifetime"
Atlas::Data::GetLevel()               // "1", "VIP", whatever you set
Atlas::Data::GetNote()                // custom note from the dashboard
Atlas::Data::GetFirstSeenDate()
Atlas::Data::GetLastSeenDate()
Atlas::Data::GetActiveUserCount()     // currently authenticated users
Atlas::Data::GetUserCount()           // total registered users on this app

Atlas::Data::IsAuthenticated()        // logged in, session active
Atlas::Data::IsBanned()               // user account is banned

Atlas::Data::GetDaysRemaining()       // -1 if lifetime, 0 if expired
Atlas::Data::IsLifetime()
Atlas::Data::IsExpiringSoon(days = 7)

Atlas::Data::GetErrorMessage()        // last error, "" if none
Atlas::Data::HasError()
Atlas::Data::ClearError()
```

### Network — server operations

```cpp
Atlas::Network::CheckAuthentication();
    // Force a server round-trip to verify the session is still valid.
    // The SDK's own 5s heartbeat already does this — CheckAuthentication()
    // is for on-demand checkpoints (e.g. before revealing gated UI).

Atlas::Network::Download(int file_id);
    // Fetch a file uploaded via the dashboard. Returns raw bytes.
    // Empty vector on failure.

Atlas::Network::BanUser(reason, minutes);
    // Ban the current user. Requires the API key to have ban permissions
    // in the dashboard. minutes = 0 means permanent.

Atlas::Network::SubmitLog(text);
    // Custom log entry, appears in the dashboard Logs tab. Max 512 chars.
```

### Variables — server-set app config

```cpp
Atlas::Variables::Fetch("welcome_msg");     // string
Atlas::Variables::FetchBool("beta_feature"); // bool
Atlas::Variables::FetchInt("max_items");     // int
```

Change values in the dashboard, next `Fetch` returns the new value. Kill-switch for features without a rebuild.

### Helper — utility (not Atlas-specific)

```cpp
Atlas::Helper::SendDiscord(webhook_url, msg);
Atlas::Helper::SendDiscordEmbed(url, title, desc, color);
Atlas::Helper::SendWebhook(url, json_payload);
Atlas::Helper::Exit();                // kernel-level fastfail
Atlas::Helper::Ping();                // ms round-trip, -1 if unreachable
```

---

## What runs automatically after `Login`

You don't write any of this. It happens inside the SDK on its own threads.

- **Every 5 seconds — heartbeat.** Signed, sequence-numbered, echoes the server's newest challenge nonce. Server can push messages, kick sessions, or issue a hard terminate in the reply. The client cannot resist a server kill.
- **Every 15 seconds — deep sweep.** `.text` section CRC vs the startup snapshot (catches NOP patches, code caves, jump injections). Full IAT verification (catches hook injection, manual mapping).
- **Continuous — inline-hook scan.** First bytes of `ws2_32.recv/send/connect` checked for hook signatures (JMP rel32, CALL, INT3) before every heartbeat cycle. A hooked network function is the foundation of a MitM on the auth channel — Atlas kills the process before any data crosses it.
- **Continuous — injected-module detection.** Executable page map compared against the post-login snapshot. New executable pages (reflective DLL, manual map, shellcode) trigger termination.
- **Continuous — debugger detection.** PEB flags, `NtQueryInformationProcess`, hardware breakpoints on `DR0–DR7`, VEH front-of-chain interception. Multiple independent methods; any positive terminates.
- **Continuous — mutual watchdog.** Two threads monitor each other using hardware performance counters (not system time — that's hookable). Either goes silent → both processes fastfail.

**On any failure:** `__fastfail()` from kernel. No dialog. No exception handler. No soft signal to intercept. The process just stops.

---

## Dashboard — the operator side

The web dashboard at [atlassecurity.site](https://atlassecurity.site) is where you manage everything. The important tabs:

- **Applications** — create apps, set the executable-hash whitelist, rotate API keys
- **Licenses** — generate, extend, ban, batch-import
- **Sessions** — see who's live right now, send them a message, kick them
- **Logs** — every auth event with timestamp / IP / geolocation / device / HWID / result / latency; filter by result (ALLOW / DENY / BAN), by license, by IP, by date
- **Bans** — issue by license, HWID, IP, or individual hardware component (NIC MAC, TPM hash, disk serial)
- **Analytics** — auth response time, uptime, load, live geographic heatmap of active connections
- **Variables** — set the server-side values `Atlas::Variables::Fetch*` reads

### Bans — how they actually work

Bans propagate within one heartbeat cycle (≤ 5 seconds of the user's next check-in). Ban vectors, each independent:

- **License key** — key invalidated server-side, rejected at next heartbeat
- **Full HWID** — the combined fingerprint hash is banned
- **Per-component** — ban an individual NIC MAC, TPM hash, or volume serial. The user cannot spoof just that component without shifting the full fingerprint (which they can't do without new hardware).
- **IP address** — global across all your applications
- **Deep ban** — cascade all known fingerprint variants associated with the user

---

## Troubleshooting

### `LNK2019: unresolved external symbol "Atlas::Startup"`

You didn't add `Atlas Auth.lib` to the linker inputs, or the library search path doesn't include the folder that contains it. See [5.2](#52--point-visual-studio-at-those-files).

### `error C2039: 'API_KEY': is not a member of 'Atlas'`

You're compiling against an older `Atlas.h`. Copy the current one from `shared/`.

### The app builds but `Atlas::Startup()` immediately terminates it

The SDK's kill path fired during initialization. Common causes:

- API key is still `"YOUR_API_KEY"` — set it in `Atlas.h`.
- API key is malformed or for a deleted application.
- A debugger is attached to the compiled `.exe` (VS debugger, x64dbg). Atlas detects this and refuses to run. Test with `Ctrl+F5` (run without debugger) — production users won't have a debugger attached, so this isn't a bug.

The dashboard's **Logs** tab shows the reason. Check there first.

### `Atlas::Login` returns `false`, `GetErrorMessage()` says "Executable hash mismatch"

You added your `.exe` hash to the whitelist, then rebuilt — the rebuilt binary has a new hash. Compute the new SHA-256, update the whitelist. In active development, either don't set a whitelist yet, or add each dev build's hash as you go.

### `Login` returns `false`, `GetErrorMessage()` says "License banned" (or "HWID banned")

The license or the hardware was banned in the dashboard. Check **Bans** to confirm.

### The process just exits with no message, no error

`__fastfail()` fired. This is expected on integrity failures. The dashboard's **Logs** tab records the reason. Common triggers:

- Debugger attached (see above)
- `.text` section modified (a memory scanner wrote to your code, or a patch tool tried to NOP a check)
- An IAT hook was detected (an injector's `SetWindowsHookEx` or `MinHook` install)
- The 5-second heartbeat couldn't reach the server for too long

### The `.exe` is huge

`Atlas Auth.lib` is a ~15 MB static library. That's the whole protection stack. It compresses ~4:1 with UPX or the equivalent, but the exe grows most in the `.rdata` section — this is intentional (embedded lookup tables, hashed API imports). Ship as-is unless you're shipping to a size-constrained target.

### More questions

Full FAQ at [atlassecurity.site/docs](https://atlassecurity.site/docs), or ask on Discord.

---

## The API-key model

Your API key is a **routing identifier**, not a bearer secret. On every request the server derives per-request keys from the fingerprint + exe hash + timestamp; the API key just says "send this to the right dashboard account." What actually authenticates the request:

- HMAC signature over the frame body
- HWID binding on the license record
- Fresh nonce that can't be replayed
- Executable hash matching the whitelist (if you've configured one)

**A leaked API key does not, by itself, let an attacker impersonate a user** — they still need the license, and the license binds to hardware. But you should still treat it as sensitive metadata: rotate on suspected exposure (dashboard → Settings → Rotate key), and don't post it in public source code (put it in a config file that's `.gitignore`d, or fetch it at runtime from your own signed config endpoint).

---

## Rebuilding `Atlas Auth.lib` from source

**You don't need this for normal integration** — the `.lib` is committed. Skip this section unless you're modifying the SDK internals or verifying the reproducible build.

The `.lib` is built from the SDK's C++ sources in Visual Studio 2022:

1. Open `Auth Library/Atlas Auth/Atlas Auth.sln` (that repo is separate; see the [Atlas SDK sources](https://atlassecurity.site/docs) doc).
2. Configuration: **Release · x64** (or **Ship-All · x64** if you also want to produce `Atlas.dll` for the JS/Python/C# bindings from the same build).
3. Build.
4. Output lands at `C++ Integration/shared/Atlas Auth.lib` — this repo's copy is overwritten in place.

Full build documentation with troubleshooting: [`BUILD.md`](../BUILD.md) in the SDK sources repo.

---

## Pricing

Free tier for life — 3 applications, 300 licenses across them, 3 file uploads per app. Full security stack, no feature gates.

**Premium** removes the caps: [Weekly $9](https://atlassecurity.site/plans) · Monthly $19 · 6-month $79 (save 31%) · Yearly $99 (save 57%). PayPal or cryptocurrency. Activates instantly.

---

## Support

- **Docs** — [atlassecurity.site/docs](https://atlassecurity.site/docs) — full reference, architecture, protocol
- **Discord** — [discord.gg/EG5dmpFaCF](https://discord.gg/EG5dmpFaCF) — fastest response
- **Email** — [mail@atlassecurity.site](mailto:mail@atlassecurity.site) — for anything you'd rather not post publicly
- **Bug reports** — include the OS version, Visual Studio version, the failing SDK call, and the dashboard **Logs** entry (if there is one)

---

## Legal notice

© 2025–2026 Atlas Security Solutions. All rights reserved. Sold by Atlas Security Solutions, Jeddah, Kingdom of Saudi Arabia.

This SDK exists for one purpose: to let developers integrate Atlas Authentication into their software. If you're a developer building an application and using this code to license and protect it through Atlas — you are exactly who this is for. Use it freely.

**The following acts are strictly prohibited without explicit written authorization** and apply to those who seek to abuse, exploit, or undermine the Atlas platform. Atlas reserves all rights to pursue legal action:

- Reverse engineering, decompiling, disassembling, or reconstructing the Atlas platform, its compiled binaries, network protocols, or server infrastructure
- Tampering with, bypassing, disabling, or circumventing any authentication check, anti-tamper control, or security mechanism within the Atlas system
- Accessing, probing, or interfering with Atlas servers, databases, or infrastructure without authorization
- Using knowledge of Atlas internals to build, assist, or contribute to competing platforms or security-bypass tools

**Applicable law and enforcement:**

- **Saudi Arabia:** Anti-Cybercrime Law (Royal Decree No. M/17, 1428H) — Articles 3 and 4
- **United States:** Computer Fraud and Abuse Act (18 U.S.C. § 1030)
- **European Union:** Directive 2013/40/EU on Attacks Against Information Systems
- **International:** WIPO Copyright Treaty and the TRIPS Agreement (180+ signatory nations)

Atlas Security Solutions actively monitors for unauthorized access, reverse-engineering attempts, and protocol analysis. Any violation will be met with immediate civil action, referral to competent national authorities, and pursuit of all available legal remedies — including injunctive relief, asset recovery, and cross-jurisdiction enforcement — without prior notice.

For permission requests or legal inquiries: [mail@atlassecurity.site](mailto:mail@atlassecurity.site) · [atlassecurity.site/legal](https://atlassecurity.site/legal)
