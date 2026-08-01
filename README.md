# Atlas Authentication — C++ SDK

![Platform](https://img.shields.io/badge/platform-Windows%20x64-0078D6?logo=windows&logoColor=white) ![Language](https://img.shields.io/badge/language-C%2B%2B-00599C?logo=cplusplus&logoColor=white) ![License](https://img.shields.io/badge/license-proprietary-lightgrey)

[atlassecurity.site](https://atlassecurity.site) · [Dashboard](https://atlassecurity.site/dashboard) · [Docs](https://atlassecurity.site/docs) · [Discord](https://discord.gg/EG5dmpFaCF) · [mail@atlassecurity.site](mailto:mail@atlassecurity.site)

Most authentication libraries stop working once login succeeds — the client is trusted for the rest of the session. Atlas doesn't make that assumption. After `Login` returns, the SDK keeps proving to the server that the process is still the one that logged in: same binary, same memory, same network stack, still alive. If any of that stops being true, the process is terminated strictly. Atlas is built on a A-Z Zero-trust structure with logically untamperable code, that is how we have created Atlas. For teams whose licensing gets bypassed and whos binaries get cracked.

Two calls get you there:

```cpp
Atlas::Startup();
Atlas::License::Login(key);
```

This repo contains the SDK header, the prebuilt static library, and two runnable examples — a console example and a native DirectX 11 / Dear ImGui example.

---

## Contents

- [Repo layout](#repo-layout)
- [Prerequisites](#prerequisites)
- [Get an account, an app, a license](#get-an-account-an-app-a-license)
- [Console example](#console-example)
- [ImGui example](#imgui-example)
- [Integrate into your project](#integrate-into-your-project)
- [API reference](#api-reference)
  - [Session lifecycle](#session-lifecycle)
  - [`Atlas::License`](#atlaslicense)
  - [`Atlas::Account`](#atlasaccount)
  - [`Atlas::Data`](#atlasdata)
  - [`Atlas::Network`](#atlasnetwork)
  - [`Atlas::Variables`](#atlasvariables)
  - [`Atlas::Webhook`](#atlaswebhook)
  - [`Atlas::Dialog`](#atlasdialog)
- [What Login starts](#what-login-starts)
- [The API-key model](#the-api-key-model)
- [Troubleshooting](#troubleshooting)
- [Support](#support)
- [Legal](#legal)

---

## Repo layout

```
C++ Integration/
├── Atlas SDK/
│   ├── Atlas.h                       the API you call
│   └── Atlas Auth.lib                the static library — link against this
├── Console Example/
│   ├── Atlas Auth Example.cpp        ~150 lines: license, account, and register paths
│   ├── Atlas Auth Example.sln
│   └── Atlas Auth Example.vcxproj
└── ImGui Example/
    ├── README.md                     Dear ImGui vendoring step
    ├── Atlas Auth ImGui Example.cpp  native GUI login → welcome flow
    ├── Atlas Auth ImGui Example.sln
    ├── Atlas Auth ImGui Example.vcxproj
    └── imgui/                        ← you vendor Dear ImGui here (one-time)
```

`Atlas Auth.lib` is prebuilt and committed. The SDK's source isn't part of this repo — you link against the library, you don't build it.

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
2. **Applications → New application** — name it whatever; users never see it. Copy the **API key** it hands you.
3. **Licenses → Generate** — pick a duration (Weekly / Monthly / Lifetime / custom), a level (`1` for basic, `2+` for tiered), and optionally a note. Copy the key.
4. *(Optional, for the account flow)* **Applications → Account policy** — choose when verification codes fire (never / first login / every N / once per day / new HWID / new HWID or IP / always). Toggle "email required at registration" if you want to force email addresses.

Free tier: 3 applications, 300 licenses across them, 3 file uploads per app.

---

## Console example

Covers all three auth paths in about 150 lines.

1. Open [`Atlas SDK/Atlas.h`](Atlas%20SDK/Atlas.h). Replace `"YOUR_API_KEY"` with your key. Save.
2. Open [`Console Example/Atlas Auth Example.sln`](Console%20Example/).
3. Set configuration to **Release · x64**. (32-bit will not link.)
4. Build: `Ctrl+Shift+B`.
5. Run with **`Ctrl+F5`**, not `F5`. `F5` attaches the VS debugger, and the SDK refuses to run with one attached.

The example asks which auth path to try:

```
Atlas Authentication Example

Choose an auth path:
  [1] License key       (classic license authentication)
  [2] Account sign-in   (username + password + email verification)
  [3] Register account  (creates a new account, optional email)

Choice [1/2/3]:
```

Pick `[1]`, paste your license key. On success:

```
--- User Information ---
License:      ATLAS-A9F2K-4RMXM
Expiry:       15-08-2026 14:32:00
IP:           45.11.42.187
HWID:         Atlas-4A9C...
Level:        1
Note:         None
Active Users: 1
Total Users:  3
```

Open the dashboard **Logs** tab — the login is there with IP, HWID, latency, and result `ALLOW`. From **Sessions → Kick**, terminate the session; the example exits within about five seconds.

Pick `[2]` for the account flow — if the server asks for verification, an 8-digit code arrives by email and the example prompts for it inline. Pick `[3]` to register a new account.

Full source: [`Console Example/Atlas Auth Example.cpp`](Console%20Example/Atlas%20Auth%20Example.cpp).

---

## ImGui example

A native Windows GUI: two-panel login → welcome flow, Segoe UI, DirectX 11. Same SDK underneath, different shell — every thread the console example starts is running here too.

Dear ImGui isn't shipped in this repo (MIT license, upstream). Vendor it once:

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

Reuse the API key you already set in `Atlas SDK/Atlas.h`. Open the `.sln`, build **Release · x64**, run with `Ctrl+F5`. A 940×640 login window appears; sign in and you land on a welcome screen with a session card and **Sign out** / **Recheck session** buttons.

Fonts, styling, backend detail: [`ImGui Example/README.md`](ImGui%20Example/README.md).

---

## Integrate into your project

1. Copy [`Atlas SDK/Atlas.h`](Atlas%20SDK/Atlas.h) and [`Atlas SDK/Atlas Auth.lib`](Atlas%20SDK/Atlas%20Auth.lib) into your project (a `vendor/atlas/` folder is conventional).
2. In your Visual Studio project properties, for **Release | x64**:
   - **C/C++ → General → Additional Include Directories** — add the folder holding `Atlas.h`
   - **Linker → General → Additional Library Directories** — same folder
   - **Linker → Input → Additional Dependencies** — add `Atlas Auth.lib;` (the filename contains a space; include it as-is)
3. Set your API key inline in `Atlas.h`, or from your own code before `Startup()`:
   ```cpp
   Atlas::API_KEY = LoadKeyFromSignedRemoteConfig();
   Atlas::Startup();
   ```
4. Wire it up — headless path:
   ```cpp
   #include "Atlas.h"

   int main() {
       Atlas::Startup();

       std::string license = PromptUserForLicense();
       if (!Atlas::License::Login(license)) {
           std::cout << Atlas::Data::GetErrorMessage();
           return 1;
       }

       // authenticated
       return 0;
   }
   ```
   Or with the built-in Win32 dialog:
   ```cpp
   Atlas::Startup();
   if (!Atlas::License::Dialog::Login()) return 1;
   RunMyApplication();
   ```

Once you have a shipping build, compute its SHA-256 and paste it into **Applications → Executable-hash whitelist**. Modified copies are then rejected server-side before the license is even checked. You can whitelist one hash per release and revoke old ones from the same panel.

---

## API reference

Everything callable lives in [`Atlas SDK/Atlas.h`](Atlas%20SDK/Atlas.h).

### Session lifecycle

Every integration touches these four calls, regardless of auth path.

```cpp
Atlas::API_KEY = "YOUR_API_KEY";   // set before Startup, or leave inline in Atlas.h
Atlas::Startup();              // call once at the top of main()
Atlas::Logout();               // end the session, clear all state
Atlas::Exit();                 // hard-terminate the process, uncatchable
```

### `Atlas::License`

License-key sign-in: single-user, hardware-bound, no email or verification code.

```cpp
// Headless — you handle the prompts.
Atlas::License::Login(license_key);                   // key only, HWID-bound
Atlas::License::Login(username, password);            // for a license bound to one user
Atlas::License::Register(license_key,                 // bind an existing license to
                         username, password);         // a new user (does NOT sign in)

// Built-in Win32 dialogs — modal, one call.
Atlas::License::Dialog::Login();
Atlas::License::Dialog::Register();
```

**`Login` return value and side effects**

| | |
|---|---|
| Returns `true` | License valid, HWID accepted (or first-seen and now bound), session established. |
| Returns `false` | See `Atlas::Data::GetErrorMessage()` — invalid key, expired, banned, HWID mismatch, executable-hash mismatch, or server unreachable. |
| On success | Starts the heartbeat and integrity threads (see [What Login starts](#what-login-starts)); populates `Atlas::Data`. |
| On failure | No threads started; no partial session state left behind. |

### `Atlas::Account`

Username, password, and email accounts, with 8-digit email verification, password reset, and license redemption. Whether a verification code is required on a given sign-in is controlled per-app in the dashboard.

```cpp
// Headless — inspect r.status to drive your flow.
auto r = Atlas::Account::Login(username, password);
Atlas::Account::Register(username, password, email);    // email optional; needed for reset
Atlas::Account::SubmitVerification(code);               // 8-digit sign-in code
Atlas::Account::ResendVerification();                   // 60 s cooldown
Atlas::Account::ConfirmEmail(code);                     // for a pending registration
Atlas::Account::HasPendingEmailConfirm();
Atlas::Account::Redeem(license_key);                    // add a license to the signed-in account
Atlas::Account::RequestPasswordReset(identifier);       // always returns true (anti-enumeration)
Atlas::Account::CompletePasswordReset(code, new_pass);

// Built-in Win32 dialogs — same protocol.
Atlas::Account::Dialog::Login();
Atlas::Account::Dialog::Register();
Atlas::Account::Dialog::VerifyCode();
Atlas::Account::Dialog::ConfirmEmail();
Atlas::Account::Dialog::ResetPassword();
```

`LoginResult::Status` is one of `Ok`, `WrongCredentials`, `NeedsVerification`, `Banned`, `AccountPaused`, `ServerUnreachable`, `Error`.

- On `Ok` — `r.expiry`, `r.level`, `r.note` are populated.
- On `NeedsVerification` — the server emailed an 8-digit code; pass it to `SubmitVerification`. `r.masked_email`, `r.sign_in_ip`, `r.sign_in_country` are populated so you can render something like "we sent a code to a•••@example.com from Riyadh."

### `Atlas::Data`

Session state, valid once `Login` succeeds.

```cpp
// Identity
GetLicense()  GetUsername()  GetEmail()  GetIP()  GetHWID()  GetDevice()
GetNote()  GetUserId()  GetLevel()
GetFirstSeenDate()  GetLastSeenDate()

// Expiry
GetExpiry()  GetDaysRemaining()  IsLifetime()  IsExpiringSoon(days = 7)

// Status
IsAuthenticated()  IsBanned()

// App-wide counts
GetActiveUserCount()  GetUserCount()

// Errors
GetErrorMessage()  HasError()  ClearError()
```

### `Atlas::Network`

Server operations that act on the current session.

```cpp
CheckAuthentication();                        // force a fresh server round-trip
Download(int file_id);                        // dashboard-uploaded file → std::vector<uint8_t>
BanUser(reason, duration_minutes);            // duration = 0 → permanent
SubmitLog(text);                              // ≤ 512 chars, appears in dashboard Logs
ChangePassword(old_password, new_password);   // account flow only
Ping();                                       // round-trip ms to the auth server, -1 if unreachable
```

### `Atlas::Variables`

Configuration values set from the dashboard and read at runtime — change them without a rebuild.

```cpp
Atlas::Variables::Fetch("welcome_msg");        // "" if the key doesn't exist
Atlas::Variables::FetchBool("beta_feature");   // "true" / "1" / "yes" → true; else false
Atlas::Variables::FetchInt("max_items");       // 0 if missing or unparseable
```

### `Atlas::Webhook`

Fire-and-forget HTTP POSTs, unrelated to authentication — a convenience for shipping Discord notifications and generic webhooks from your app.

```cpp
Atlas::Webhook::SendDiscord(webhook_url, message);
Atlas::Webhook::SendDiscordEmbed(webhook_url, title, description, color);  // color = 0xRRGGBB
Atlas::Webhook::Send(url, json_payload);
```

### `Atlas::Dialog`

Theming for every built-in Win32 dialog. Set these once before opening any `Atlas::License::Dialog::*` or `Atlas::Account::Dialog::*`.

```cpp
Atlas::Dialog::AppName = "My App";                       // title bar text
Atlas::Dialog::theme   = Atlas::Dialog::Theme::Dark;     // Dark or Light
Atlas::Dialog::parent  = my_main_hwnd;                   // modal parent, null = foreground
Atlas::Dialog::accents.signal = 0xFFE04A2C;              // 0xAARRGGBB colour overrides
Atlas::Dialog::copy.verify_title = "Enter code";         // string overrides
Atlas::Dialog::FatalError(title, body, error_code);
```

---

## What Login starts

On success, `Login` starts background threads. You don't manage any of them directly — they run until `Logout()`, `Exit()`, or a failed check terminates the process.

- **Every 5 seconds** — a heartbeat signed with a per-session HMAC key, sequence-numbered, echoing the server's newest challenge nonce. The server can push messages, kick the session, or terminate the process in its reply.
- **Every 15 seconds** — a deep sweep: `.text` CRC compared against the startup snapshot, full IAT check against the resolved-imports snapshot.
- **Before every heartbeat** — the first bytes of `ws2_32.recv` / `send` / `connect` are inspected for hook signatures. A hooked network function is the standard foundation for a man-in-the-middle on the auth channel, so the SDK terminates before any data crosses it.
- **Continuously** — the executable's page map is compared against the post-login snapshot; PEB, `NtQueryInformationProcess`, `DR0`–`DR7`, and VEH debugger checks run; two independent threads verify each other's liveness through hardware performance counters.

Any failure terminates the process at kernel level. There is no dialog, no exception you can catch, no signal you can hook.

---

## The API-key model

The API key is a **routing identifier** — it tells the server which dashboard account a request belongs to. It is not what authenticates a request. That rests on:

1. An X25519 handshake, deriving a fresh HMAC key per session.
2. The Ed25519 signature the server places on its handshake reply, verified against three keys pinned inside `Atlas Auth.lib` (primary, backup, emergency). A nulled server can't produce these signatures.
3. HWID binding — the session key is derived with the HWID mixed in, so a stolen session token doesn't work from a different machine.
4. A per-request nonce — replays are dropped.
5. The executable-hash whitelist, if you've configured one.

> [!IMPORTANT]
> A leaked API key alone doesn't let an attacker impersonate a user — but treat it as sensitive. Rotate it on suspected exposure (**Settings → Reset API Key**) and keep it out of public source.

---

## Troubleshooting

**`LNK2019: unresolved external symbol "Atlas::Startup"`** — `Atlas Auth.lib` isn't in the linker inputs, or the library search path doesn't include its folder. See step 2 of [Integrate into your project](#integrate-into-your-project).

**`error C2039: 'API_KEY': is not a member of 'Atlas'`** — you're compiling against an old `Atlas.h`. Copy the current one from `Atlas SDK/`.

**Startup terminates the process immediately** — the SDK's kill path fired. Common causes: API key still set to `"YOUR_API_KEY"`; API key belongs to a deleted app; a debugger is attached (test with `Ctrl+F5`, not `F5`). Check the dashboard **Logs** tab for the reason.

**`Login` returns `false`, "Executable hash mismatch"** — you whitelisted a hash and then rebuilt. Update the whitelist, or don't whitelist during active development.

**`Login` returns `false`, "License banned" / "HWID banned"** — check **Bans** in the dashboard.

**`Account::Login` returns `NeedsVerification` but no email arrives** — the app has no email sender configured, or the recipient's inbox rejected it. Check the **Logs** tab for the outbound event; `Account::ResendVerification()` has a 60-second cooldown.

**Process exits silently, no message** — an integrity check tripped the kill path. Dashboard **Logs** shows the reason: `.text` modified, IAT hooked, injected module, server signature verification failed (nulled or MITM'd server), or the heartbeat couldn't reach the server for too long.

Full FAQ: [atlassecurity.site/docs](https://atlassecurity.site/docs).

---

## Support

- **Docs** — [atlassecurity.site/docs](https://atlassecurity.site/docs)
- **Discord** — [discord.gg/EG5dmpFaCF](https://discord.gg/EG5dmpFaCF) (fastest response)
- **Email** — [mail@atlassecurity.site](mailto:mail@atlassecurity.site)

Bug reports: include your OS version, Visual Studio version, the failing SDK call, and the dashboard **Logs** entry if there is one.

The SDK's source is not distributed with this repo. If you need a custom build or believe you've found a bug in `Atlas Auth.lib` itself, contact support — don't attempt to reconstruct or patch the library from the header alone.

---

## Legal

© 2025–2026 Atlas Security Solutions. All rights reserved. Sold by Atlas Security Solutions, Jeddah, Kingdom of Saudi Arabia.

This SDK exists so developers can integrate Atlas Authentication into their software. If that's you, use it freely.

**Prohibited without explicit written authorization:**
- Reverse engineering, decompiling, disassembling, or reconstructing Atlas binaries, protocols, or server infrastructure
- Tampering with, bypassing, or disabling any authentication or anti-tamper control
- Probing or interfering with Atlas servers or databases
- Using knowledge of Atlas internals to build competing platforms or bypass tools

Enforcement: Saudi Arabia Anti-Cybercrime Law (Royal Decree M/17, 1428H, Articles 3–4); U.S. Computer Fraud and Abuse Act (18 U.S.C. § 1030); EU Directive 2013/40/EU; WIPO / TRIPS (180+ signatory nations).

Atlas monitors for unauthorized access, reverse engineering, and protocol analysis. Violations are met with civil action, referral to competent authorities, and pursuit of all available remedies — injunctive relief, asset recovery, and cross-jurisdiction enforcement — without prior notice.

Permission requests and inquiries: [mail@atlassecurity.site](mailto:mail@atlassecurity.site) · [atlassecurity.site/legal](https://atlassecurity.site/legal)