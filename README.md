# Atlas Authentication - C++ SDK

![Platform](https://img.shields.io/badge/platform-Windows%20x64-0078D6?logo=windows&logoColor=white) ![Language](https://img.shields.io/badge/language-C%2B%2B-00599C?logo=cplusplus&logoColor=white) ![License](https://img.shields.io/badge/license-MIT%20source%20%C2%B7%20proprietary%20lib-lightgrey)

[atlassecurity.site](https://atlassecurity.site) · [Dashboard](https://atlassecurity.site/dashboard) · [Docs](https://atlassecurity.site/docs) · [Discord](https://discord.gg/EG5dmpFaCF) · [mail@atlassecurity.site](mailto:mail@atlassecurity.site)

**Authorization that holds under active attack.**

Most auth libraries stop caring once login succeeds - the client is trusted for the rest of the session. Atlas doesn't. After `Login` returns, the SDK keeps proving to the server that the process is still the one that logged in: same binary, same memory, same network stack, still alive. If any of that stops being true, the process ends. Built for teams whose licensing keeps getting bypassed and whose binaries keep getting cracked.

## Contents

- [Install](#install)
- [Quick start](#quick-start)
- [Get an account, an app, and a license](#get-an-account-an-app-and-a-license)
- [Examples](#examples)
- [API at a glance](#api-at-a-glance)
- [How a session is protected](#how-a-session-is-protected)
- [The API-key model](#the-api-key-model)
- [Executable hash whitelist](#executable-hash-whitelist)
- [Packaging](#packaging)
- [Auto-update](#auto-update)
- [Troubleshooting](#troubleshooting)
- [Diagnostic logs](#diagnostic-logs)
- [Support](#support)
- [License](#license)

## Install

### Requirements

| | |
|---|---|
| Windows 10 or 11 (x64) | Atlas is Windows-x64 only. |
| [Visual Studio 2022](https://visualstudio.microsoft.com/vs/community/) | Community edition is fine. |
| **Desktop development with C++** workload | Installs MSVC v143, the Windows SDK and MSBuild. |
| C++17 or newer | The header uses `inline` variables. The examples build with C++20. |
| An Atlas account | [atlassecurity.site](https://atlassecurity.site) - free. |

### Add it to your project

1. Copy `Atlas SDK/Atlas.h` and `Atlas SDK/Atlas Auth.lib` into your project (`vendor/atlas/` is conventional).
2. In your Visual Studio project properties, for **Release | x64**:
   - **C/C++ → General → Additional Include Directories** - the folder holding `Atlas.h`
   - **Linker → General → Additional Library Directories** - the same folder
   - **Linker → Input → Additional Dependencies** - add `Atlas Auth.lib;` (the filename contains a space; include it as-is)
3. Set your API key in code before `Startup()` (`Atlas::API_KEY = "..."`), or inline in `Atlas.h`.

`Atlas Auth.lib` links statically. No DLL to ship, no Boost, no vcpkg, no redistributables - the resulting `.exe` runs on a stock Windows install.

### What's in this repo

```
Atlas SDK/
  Atlas.h                       the API you call
  Atlas Auth.lib                the static library - link against it
Console Example/                headless CLI: license, account and register paths
ImGui Example/                  native GUI, login → welcome flow (Dear ImGui + DirectX 11)
```

`Atlas Auth.lib` is prebuilt. You link against it; you don't build the SDK. The SDK source is private.

## Quick start

```cpp
#include <Atlas.h>
#include <iostream>

int main() {
    Atlas::API_KEY = "YOUR_API_KEY";
    Atlas::Startup();

    if (!Atlas::License::Login("ATLAS-A9F2K-4RMXM")) {
        std::cout << Atlas::Data::GetErrorMessage();
        return 1;
    }
    // authenticated
    return 0;
}
```

> **Run without a debugger attached.** An attached debugger is treated as tampering: the process ends shortly after `Startup()`, and the event can be reported against the license or HWID as a ban. Run with **`Ctrl+F5`**, not `F5`.

## Get an account, an app, and a license

1. Sign up at [atlassecurity.site](https://atlassecurity.site) and verify your email.
2. **Dashboard → Applications → New application.** Name it - end users see the name in dialogs and emails. Copy the **API key**. You can view it again later under **Applications → Manage → View API Key**.
3. **Dashboard → Users → License users → Generate.** Pick a duration (or no expiry), a level (`1` for basic, `2+` for tiered) and an optional note. Copy the key.
4. *Account flow only:* accounts live under **Users → Account users**. **Settings → Security → Account policy** sets when 8-digit verification codes fire (never, first login, every N logins, once per H hours, new device, new device or IP, always) and whether registration requires an email or a license key.

Free tier: 3 applications, 300 licenses per app, 3 file uploads per app.

## Examples

### Console example

Covers all three auth paths.

1. Open `Atlas SDK/Atlas.h` and replace `"YOUR_API_KEY"` with your key.
2. Open `Console Example/Atlas Auth Example.sln`.
3. Set the configuration to **Release · x64** (32-bit will not link) and build with `Ctrl+Shift+B`.
4. Run with **`Ctrl+F5`**, not `F5`. `F5` attaches the Visual Studio debugger, and the SDK treats an attached debugger as tampering.

The example asks which path to try:

```
Atlas Authentication Example

Choose an auth path:
  [1] License key       (classic, HWID-bound)
  [2] Account sign-in   (username + password + email verification)
  [3] Register account  (creates a new account, optional email (Configured in dashboard))

Choice [1/2/3]:
```

Pick `[1]` and paste a license key. On success:

```
--- User Information ---
License:      ATLAS-A9F2K-4RMXM
Expiry:       15-08-2026
IP:           203.0.113.42
HWID:         Atlas-4A9C...E1B2
Level:        1
Note:         None
Active Users: 1
Total Users:  3
```

Open **Dashboard → Logs** - the login is there with its IP, HWID and result. From **Monitor → Kill**, end the session; the example exits within a few seconds. Pick `[2]` for the account flow - if the server asks for verification, an 8-digit code arrives by email and the example prompts for it inline. Pick `[3]` to register a new account.

### ImGui example

A native Windows GUI: a two-panel login → welcome flow in Segoe UI on DirectX 11. Same SDK underneath, different shell - every thread the console example starts is running here too.

`Startup()` runs after the window, the D3D11 device and ImGui exist. Atlas snapshots the process's executable pages at `Startup()` and ends the process if new ones appear later, so loading the GPU runtimes first keeps them in the baseline.

Reuse the API key you set in `Atlas SDK/Atlas.h`. Open `ImGui Example/Atlas Auth ImGui Example.sln`, build **Release · x64** and run with `Ctrl+F5`. A login window appears; sign in and you land on a welcome screen with a session card and **Sign out** / **Recheck session** buttons.

If `ImGui Example/imgui/` is missing, vendor Dear ImGui once:

```
cd "ImGui Example"
git submodule add https://github.com/ocornut/imgui.git imgui
git submodule update --init --recursive
```

or download the source zip from [github.com/ocornut/imgui/releases](https://github.com/ocornut/imgui/releases) into `ImGui Example/imgui/`. Build settings are in `Atlas Auth ImGui Example.vcxproj` (C++20, DX11 + Win32 backends).

## API at a glance

```cpp
// Session
Atlas::API_KEY = "YOUR_API_KEY";   // set before Startup, or inline in Atlas.h
Atlas::Startup();                  // once, at the top of main() - in a GUI app, after the window and graphics device exist
Atlas::Logout();                   // end the session, clear all state
Atlas::Exit();                     // hard-terminate the process, uncatchable

// License
Atlas::License::Login(license_key);                          // key only, HWID-bound
Atlas::License::Login(username, password);                   // for a license bound to one user
Atlas::License::Register(license_key, username, password);   // binds a license; does NOT sign in
Atlas::License::Dialog::Login();  Atlas::License::Dialog::Register();   // built-in Win32 dialogs

// Account
auto r = Atlas::Account::Login(username, password);          // inspect r.status
Atlas::Account::Register(username, password, email);         // email optional; needed for reset
Atlas::Account::SubmitVerification(code);                    // 8-digit sign-in code
Atlas::Account::ResendVerification();                        // 60 s cooldown
Atlas::Account::ConfirmEmail(code);                          // for a pending registration
Atlas::Account::HasPendingEmailConfirm();
Atlas::Account::Redeem(license_key);                         // apply a key to the signed-in account
Atlas::Account::RequestPasswordReset(identifier);            // true whether or not it matched
Atlas::Account::CompletePasswordReset(code, new_password);
Atlas::Account::Dialog::Login();   // also Login(u, p), Register(), VerifyCode(), ConfirmEmail(), ResetPassword()

// Network
Atlas::Network::CheckAuthentication();                       // force a fresh server round-trip
Atlas::Network::Download(file_id);                           // dashboard-uploaded file → std::vector<uint8_t>
Atlas::Network::BanUser(reason, duration_minutes);           // 0 = permanent
Atlas::Network::SubmitLog(text);                             // ≤ 512 chars, shows in Dashboard → Logs
Atlas::Network::ChangePassword(old_password, new_password);  // account sessions only
Atlas::Network::Ping();                                      // ms to reach the auth server, -1 if unreachable

// Data - valid once Login succeeds
GetLicense()  GetUsername()  GetEmail()  GetPassword()  GetIP()  GetHWID()  GetDevice()
GetNote()  GetUserId()  GetLevel()  GetFirstSeenDate()  GetLastSeenDate()
GetExpiry()  GetDaysRemaining()  IsLifetime()  IsExpiringSoon(days_threshold = 7)
IsAuthenticated()  IsBanned()  GetActiveUserCount()  GetUserCount()
GetErrorMessage()  HasError()  ClearError()

// Variables - set on the dashboard, read at runtime, no rebuild
Atlas::Variables::Fetch(key);  Atlas::Variables::FetchBool(key);  Atlas::Variables::FetchInt(key);

// Entitlements (releases after 1.0.3) - named features and counters the seller gives a license or account
Atlas::Entitlements::Has(key);                  // held, not expired, and for a counter something left
Atlas::Entitlements::Remaining(key);            // -1 no limit, 0 none, else what is left
Atlas::Entitlements::Consume(key, amount = 1);  // spend from a counter; false if refused (GetErrorMessage() says why)
Atlas::Entitlements::List();  Atlas::Entitlements::Refresh();   // keys held; re-read now (the list is cached ~20 s)

// Webhook - fire-and-forget POSTs
Atlas::Webhook::SendDiscord(url, message);
Atlas::Webhook::SendDiscordEmbed(url, title, description, color);   // color = 0xRRGGBB
Atlas::Webhook::Send(url, json_payload);

// Dialog theming (C++ only)
Atlas::Dialog::AppName = "My App";  Atlas::Dialog::theme = Atlas::Dialog::Theme::Dark;
Atlas::Dialog::parent = my_main_hwnd;  Atlas::Dialog::accents.signal = 0xFFE04A2C;
Atlas::Dialog::copy.verify_title = "Enter code";
Atlas::Dialog::FatalError(title, body, error_code);
```

`Account.Login` returns a result. Branch on `status` first:

| Status | Meaning | Do |
|---|---|---|
| `Ok` | Signed in. `user_id`, `expiry`, `level`, `note` are populated. | Continue. |
| `NeedsVerification` | The server emailed an 8-digit code. `masked_email`, `sign_in_ip`, `sign_in_country` are populated. | Prompt for the code, then `SubmitVerification(code)`. |
| `WrongCredentials` | Unknown username or password. | Show the message; let the user retry. |
| `Banned` | The account is banned. | Show the message. Lift the ban under **Bans** in the dashboard. |
| `AccountPaused` | Paused by the seller. | Show the message. The seller resumes it from the dashboard. |
| `ServerUnreachable` | Network or DNS failure. | Back off and retry. |
| `Error` | Anything else. | Show `error_message`. |

**Good to know**

- `GetExpiry()` is a `DD-MM-YYYY` date (valid through the end of that day) or `Never`. `GetDaysRemaining()` is `-1` with no expiry, `0` when expired or under 24 hours are left, otherwise whole days.
- `GetNote()` is `None` when no note is set.
- On account sessions `GetLicense()` returns `user:<username>`, not a key. Show `GetUsername()` instead.
- `GetPassword()` returns the password used at sign-in, in cleartext, for the life of the session.
- `SubmitLog` queues the line (512 characters max); the next heartbeat sends it.
- `Login` returns `false` on failure. Read `GetErrorMessage()` for the reason: invalid key, expired, banned, HWID mismatch, executable-hash mismatch, or server unreachable. On failure no threads start and no session state is left behind.

Full reference with signatures and examples in all three languages: [SDK reference](https://atlassecurity.site/docs?p=sdk/lifecycle).

## How a session is protected

`Login` doesn't end at the handshake. From that point forward, every assumption gets re-verified for the entire life of the session - nothing is trusted just because it was true a moment ago. This is zero trust applied to the client itself, not just the connection.

- **The server re-authenticates the session continuously, not once.** Every check the client passed at login runs again, on a loop, for as long as the process is alive. Passing once buys you nothing later - you keep proving it.
- **Every message between client and server is signed, fresh, and single-use.** Nothing is replayable. A captured request, however perfectly captured, is worthless the moment it's reused.
- **The server holds full control over every live session, in real time.** It can end, message, or re-verify any session on demand - the client has no ability to resist, delay, or negotiate.
- **The binary and its runtime state are continuously verified against what was there at login.** Any modification, any injected code, any external interference with the running process is treated as a compromise - not logged, not flagged, acted on.
- **Detection never announces itself.** No dialog, no error, no exception, nothing to hook or intercept. The response to a failed check is the process ending - not a message telling an attacker what they tripped.
- **Nothing static ever sits in the client waiting to be stolen.** No reusable secret, no long-lived token, no single value that unlocks the next session if it leaks.

Heartbeat: license sessions hold a persistent socket and beat every 3-7 s; account sessions poll every 5 s. From the dashboard you can message a live session (**Monitor → Message**), end it (**Monitor → Kill**) or ban the user (**Bans**).

## The API-key model

The API key is a **routing identifier** - it tells the server which dashboard account and application a request belongs to. It is not what authenticates a request. That rests on:

1. An X25519 handshake, deriving a fresh HMAC key per session.
2. The Ed25519 signature the server places on its handshake reply, verified against three keys pinned inside `Atlas Auth.lib` (primary, backup, emergency). A nulled server can't produce these signatures.
3. HWID binding - the session key is derived with the HWID mixed in, so a stolen session token doesn't work from a different machine.
4. A per-request nonce - replays are dropped.
5. The executable-hash whitelist, if you've added any.

> **Important:** a leaked API key alone doesn't let an attacker impersonate a user - but treat it as sensitive. Rotate it on suspected exposure (**Dashboard → Settings → Security → Reset API key**) and keep it out of public source.

## Executable hash whitelist

Once you have a shipping build, drop the `.exe` into **Dashboard → Settings → Security → Authorized binary hashes → Authorize a binary**. The dashboard computes the SHA-256 in your browser - the file never leaves it. Modified copies are then rejected server-side before the license is even checked. Authorize one hash per release; remove old ones from the same panel. Building in CI? Choose **Paste hash** and paste the output of `Get-FileHash "myapp.exe" -Algorithm SHA256`.

The hash is of your own `.exe` - whatever Atlas's startup fingerprint reads. Rebuilds produce new hashes; authorize each one before you cut a release.

More: [Executable Hash Whitelist](https://atlassecurity.site/docs?p=concepts/hash-whitelist).

## Packaging

The C++ SDK links statically. Build **Release · x64**, ship the `.exe`, and authorize its hash. Nothing else goes beside it.

## Auto-update

Ship a new build without your users doing anything: upload it under **Dashboard → Settings → Security → Versions**, promote it, and every client on an older authorized version replaces its own `.exe` on its next launch. `Startup()` checks before any login, verifies the SHA-256 of the download, swaps the file and relaunches - about two seconds. Rollback is promoting an earlier version. It replaces one file, so it suits single-file builds (a C++ `.exe`). Details: [Auto-Update](https://atlassecurity.site/docs?p=guides/auto-update).

## Troubleshooting

**`LNK2019: unresolved external symbol "Atlas::Startup"`** - `Atlas Auth.lib` isn't linked, or the library search path doesn't include the SDK folder. See [Add it to your project](#add-it-to-your-project).

**`error C2039: 'API_KEY': is not a member of 'Atlas'`** - you're compiling against an outdated `Atlas.h`. Replace it with the current version from `Atlas SDK/`.

**The app exits shortly after `Startup()`** - Atlas ended the process after a check failed. Verify `Atlas::API_KEY` is set, the application still exists in the dashboard, and no debugger is attached (`Ctrl+F5`, not `F5`). In a GUI app, call `Startup()` after the window and graphics device exist. See [Diagnostic logs](#diagnostic-logs) for the exact reason.

**`Login()` returns `false`** - call `Atlas::Data::GetErrorMessage()`. Common causes: an executable-hash mismatch after a rebuild, an expired or banned license, a banned HWID, invalid credentials.

**`Account::Login()` returns `NeedsVerification`** - not a failure. The server emailed an 8-digit code; prompt for it and call `Account::SubmitVerification(code)`. If it didn't arrive, check delivery in **Dashboard → Logs**, then call `Account::ResendVerification()` (60-second cooldown).

**The process exits unexpectedly after authentication** - a runtime integrity check failed: modified code sections, injected modules, hooked imports, a failed server signature check, or an extended heartbeat timeout. See [Diagnostic logs](#diagnostic-logs).

## Diagnostic logs

Every session-ending event - a failed integrity check, a lost connection, a server-issued end to the session - is written to disk the moment it occurs, with the exact cause, source file and line. The `logs\` folder always exists on every machine running an Atlas-built application, end users included.

Press **`Win + R`** and paste:

```
%LOCALAPPDATA%\AtlasAuth
```

Each `atlas_exit_<timestamp>.log` in `logs\` is a complete record of one event:

```
[Atlas Exit Report]
Time:   2026-08-02 08:38:50
Reason: CheckAuthentication: not authenticated or no session
File:   Atlas Auth.cpp
Line:   2258
```

> The rest of that folder is dev-only: `installed.flag`, `declined.flag`, `commit.sha` and `manage_autoupdate.bat`, which drive the MSBuild auto-update hook and appear only when a development environment is detected. End users only ever have `logs\` and, after a tamper trip, `pending_bans.dat`. Check `logs\` first whenever a process ends unexpectedly.

The reasons, with what causes each: [Diagnostic Logs](https://atlassecurity.site/docs?p=diagnostics/logs).

## Support

- **Docs** - [atlassecurity.site/docs](https://atlassecurity.site/docs)
- **Discord** - [discord.gg/EG5dmpFaCF](https://discord.gg/EG5dmpFaCF) (fastest response)
- **Email** - [mail@atlassecurity.site](mailto:mail@atlassecurity.site)

Bug reports: include your OS version, Visual Studio version, the failing SDK call, and the **Dashboard → Logs** entry or the newest `atlas_exit_*.log` if there is one.

The SDK source is not distributed with this repo. If you need a custom build, or believe you've found a bug in `Atlas Auth.lib` itself, contact support - don't attempt to reconstruct or patch the library from the header alone.

## License

The header (`Atlas.h`) and the example code in this repository are released under the MIT License - see `LICENSE`. Everything below applies to `Atlas Auth.lib`, to Atlas services, and to Atlas internals.

© 2025–2026 Atlas Security Solutions. All rights reserved.
Sold by Atlas Security Solutions - Jeddah, Kingdom of Saudi Arabia.

This SDK is licensed, not sold, for one purpose: integrating Atlas Authentication into your own software. That is the entire grant. Nothing here implies any broader right.

**Not permitted, under any circumstance, without Atlas's prior written consent:**
- Reverse engineering, decompiling, disassembling, or otherwise deriving source code, protocols, or algorithms from Atlas binaries, clients, or infrastructure
- Circumventing, disabling, or interfering with any authentication or anti-tamper mechanism
- Accessing, probing, or testing Atlas servers, databases, or infrastructure outside normal SDK operation
- Using knowledge of Atlas internals to build, assist, or distribute a competing product or a bypass tool

A violation terminates this license the moment it occurs. No warning. No cure period.

This agreement is governed by the laws of the Kingdom of Saudi Arabia, including the Anti-Cyber Crime Law (Royal Decree No. M/17, 1428H), Articles 3 and 5. Unauthorized access to Atlas infrastructure is independently a criminal matter in most jurisdictions Atlas operates in, including under the U.S. Computer Fraud and Abuse Act (18 U.S.C. § 1030) and EU Directive 2013/40/EU. Atlas is not confined to one jurisdiction's remedies and will pursue violators wherever they are found.

Atlas monitors for unauthorized access and reverse-engineering activity as a matter of course. Confirmed violations are referred for civil action, criminal referral where warranted, and pursuit of injunctive relief, damages, and cross-border enforcement - without prior notice.

All rights not expressly granted are reserved.

Authorized inquiries only: [mail@atlassecurity.site](mailto:mail@atlassecurity.site) · [atlassecurity.site/legal](https://atlassecurity.site/legal)
