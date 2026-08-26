# Atlas Authentication - C++ SDK

![Platform](https://img.shields.io/badge/platform-Windows%20x64-0078D6?logo=windows&logoColor=white) ![Language](https://img.shields.io/badge/language-C%2B%2B-00599C?logo=cplusplus&logoColor=white) ![License](https://img.shields.io/badge/license-proprietary-lightgrey)

[atlassecurity.site](https://atlassecurity.site) · [Dashboard](https://atlassecurity.site/dashboard) · [Docs](https://atlassecurity.site/docs) · [Discord](https://discord.gg/EG5dmpFaCF) · [mail@atlassecurity.site](mailto:mail@atlassecurity.site)

Most auth libraries stop caring once login succeeds - the client is trusted for the rest of the session. Atlas doesn't. After `Login` returns, the SDK keeps proving to the server that the process is still the one that logged in: same binary, same memory, same network stack, still alive. If any of that stops being true, the process dies. Built for teams whose licensing keeps getting bypassed and whose binaries keep getting cracked.

Two calls get you there:

```cpp
Atlas::Startup();
Atlas::License::Login(key);
```

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
- [IMPORANT - Atlas Diagnostic Logs](#diagnostic-logs)
- [Support](#support)
- [Legal](#legal)

---

## Repo layout

```
C++ Integration/
├-- Atlas SDK/
│   ├-- Atlas.h                       the API you call
│   └-- Atlas Auth.lib                the static library - link against this
├-- Console Example/
│   ├-- Atlas Auth Example.cpp        the star of the show
│   ├-- Atlas Auth Example.sln
│   └-- Atlas Auth Example.vcxproj
└-- ImGui Example/
    ├-- README.md                     Dear ImGui vendoring step
    ├-- Atlas Auth ImGui Example.cpp  native GUI login → welcome flow
    ├-- Atlas Auth ImGui Example.sln
    ├-- Atlas Auth ImGui Example.vcxproj
    └-- imgui/                        ← you vendor Dear ImGui here
```

`Atlas Auth.lib` is prebuilt and committed. You link against the library - you don't build the SDK source. The Atlas SDK source code is private.

---

## Prerequisites

| | |
|---|---|
| Windows 10 or 11 (x64) | Atlas is Windows-x64 only. |
| [Visual Studio 2022](https://visualstudio.microsoft.com/vs/community/) | Community edition is fine. |
| **Desktop development with C++** workload | Installs MSVC v143, Windows SDK, MSBuild. |
| An Atlas account | [atlassecurity.site](https://atlassecurity.site) - free. |

No Boost, no vcpkg, no CMake, no redistributables. `Atlas Auth.lib` links statically; the resulting `.exe` runs on a stock Windows install.

---

## Get an account, an app, a license

1. Sign up at [atlassecurity.site](https://atlassecurity.site), verify your email.
2. **Applications → New application** - name it as you like, this will be often shown to end users in MessageBoxes or Emails, Copy the **API key** it hands you.
3. **Licenses → Generate** - pick a duration (Weekly / Monthly / Lifetime / custom), a level (`1` for basic, `2+` for tiered), and optionally a note. Copy the key.
4. *(Optional, for the account flow)* **Applications → Account policy** - choose when verification codes fire (never / first login / every N / once per day / new HWID / new HWID or IP / always). Toggle "email required at registration" if you want to force email addresses.

Free tier: 3 applications, 300 licenses across them, 3 file uploads per app.

---

## Console example

Covers all three auth paths.

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

Open the dashboard **Logs** tab - the login is there with IP, HWID, latency, and result `ALLOW`. From **Sessions → Kick**, terminate the session; the example exits within about five seconds.

Pick `[2]` for the account flow - if the server asks for verification, an 8-digit code arrives by email and the example prompts for it inline. Pick `[3]` to register a new account.

Full source: [`Console Example/Atlas Auth Example.cpp`](Console%20Example/Atlas%20Auth%20Example.cpp).

---

## ImGui example

A native Windows GUI: two-panel login → welcome flow, Segoe UI, DirectX 11. Same SDK underneath, different shell - every thread the console example starts is running here too.

If Dear ImGui isn't found for some reason in your source code. Vendor it once:

```
cd "ImGui Example"
git submodule add https://github.com/ocornut/imgui.git imgui
git submodule update --init --recursive
```

Or download the source zip from [github.com/ocornut/imgui/releases](https://github.com/ocornut/imgui/releases) and drop it in `ImGui Example/imgui/` so the pathing looks like:

```
ImGui Example/imgui/
├-- imgui.cpp, imgui_draw.cpp, imgui_tables.cpp, imgui_widgets.cpp, imgui.h
└-- backends/
    ├-- imgui_impl_dx11.cpp   imgui_impl_dx11.h
    └-- imgui_impl_win32.cpp  imgui_impl_win32.h
```

Reuse the API key you already set in `Atlas SDK/Atlas.h`. Open the `.sln`, build **Release · x64**, run with `Ctrl+F5`. A login window appears; sign in and you land on a welcome screen with a session card and **Sign out** / **Recheck session** buttons.

Fonts, styling, and backend detail: see the vendored Dear ImGui docs at `ImGui Example/imgui/docs/` and the build configuration in `Atlas Auth ImGui Example.vcxproj` (DX11 + Win32 backends, vendored `imgui/` source tree).

---

## Integrate into your project

1. Copy [`Atlas SDK/Atlas.h`](Atlas%20SDK/Atlas.h) and [`Atlas SDK/Atlas Auth.lib`](Atlas%20SDK/Atlas%20Auth.lib) into your project (a `vendor/atlas/` folder is conventional).
2. In your Visual Studio project properties, for **Release | x64**:
   - **C/C++ → General → Additional Include Directories** - add the folder holding `Atlas.h`
   - **Linker → General → Additional Library Directories** - same folder
   - **Linker → Input → Additional Dependencies** - add `Atlas Auth.lib;` (the filename contains a space; include it as-is)
3. Set your API key inline in `Atlas.h`, or from your own code before `Startup()`:
   ```cpp
   Atlas::API_KEY = "YOUR_API_KEY";
   Atlas::Startup();
   ```
4. Wire it up - headless path:
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

Full API surface in [`Atlas SDK/Atlas.h`](Atlas%20SDK/Atlas.h).

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
// Headless - you handle the prompts.
Atlas::License::Login(license_key);                   // key only, HWID-bound
Atlas::License::Login(username, password);            // for a license bound to one user
Atlas::License::Register(license_key,                 // bind an existing license to
                         username, password);         // a new user (does NOT sign in)

// Built-in Win32 dialogs - modal, one call.
Atlas::License::Dialog::Login();
Atlas::License::Dialog::Register();
```

**`Login` return value and side effects**

| | |
|---|---|
| Returns `true` | License valid, HWID accepted (or first-seen and now bound), session established. |
| Returns `false` | See `Atlas::Data::GetErrorMessage()` - invalid key, expired, banned, HWID mismatch, executable-hash mismatch, or server unreachable. |
| On success | Starts the heartbeat and integrity threads (see [What Login starts](#what-login-starts)); populates `Atlas::Data`. |
| On failure | No threads started; no partial session state left behind. |

### `Atlas::Account`

Username, password, and email accounts, with 8-digit email verification, password reset, and license redemption. Whether a verification code is required on a given sign-in is controlled per-app in the dashboard.

```cpp
// Headless - inspect r.status to drive your flow.
auto r = Atlas::Account::Login(username, password);
Atlas::Account::Register(username, password, email);    // email optional; needed for reset
Atlas::Account::SubmitVerification(code);               // 8-digit sign-in code
Atlas::Account::ResendVerification();                   // 60 s cooldown
Atlas::Account::ConfirmEmail(code);                     // for a pending registration
Atlas::Account::HasPendingEmailConfirm();
Atlas::Account::Redeem(license_key);                    // add a license to the signed-in account
Atlas::Account::RequestPasswordReset(identifier);       // always returns true (anti-enumeration)
Atlas::Account::CompletePasswordReset(code, new_pass);

// Built-in Win32 dialogs - same protocol.
Atlas::Account::Dialog::Login();
Atlas::Account::Dialog::Register();
Atlas::Account::Dialog::VerifyCode();
Atlas::Account::Dialog::ConfirmEmail();
Atlas::Account::Dialog::ResetPassword();
```

`LoginResult::Status` is one of `Ok`, `WrongCredentials`, `NeedsVerification`, `Banned`, `AccountPaused`, `ServerUnreachable`, `Error`.

- On `Ok` - `r.user_id`, `r.expiry`, `r.level`, `r.note` are populated.
- On `NeedsVerification` - the server emailed an 8-digit code; pass it to `SubmitVerification`. `r.masked_email`, `r.sign_in_ip`, `r.sign_in_country` are populated so you can render something like "we sent a code to a•••@example.com from Riyadh."

### `Atlas::Data`

Session state, valid once `Login` succeeds.

```cpp
// Identity
GetLicense()  GetUsername()  GetEmail()  GetPassword()  GetIP()  GetHWID()  GetDevice()
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

Configuration values set from the dashboard and read at runtime - change them without a rebuild.

```cpp
Atlas::Variables::Fetch("welcome_msg");        // "" if the key doesn't exist
Atlas::Variables::FetchBool("beta_feature");   // "true" / "1" / "yes" → true; else false
Atlas::Variables::FetchInt("max_items");       // 0 if missing or unparseable
```

### `Atlas::Webhook`

Fire-and-forget HTTP POSTs, unrelated to authentication - a convenience for shipping Discord notifications and generic webhooks from your app.

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
Atlas::Dialog::accents.signal = 0xFFE04A2C;              // 0xAARRGGBB color overrides
Atlas::Dialog::copy.verify_title = "Enter code";         // string overrides
Atlas::Dialog::FatalError(title, body, error_code);
```

---

## What Login starts

`Login` doesn't end at the handshake. From that point forward, every assumption gets re-verified for the entire life of the session - nothing is trusted just because it was true a moment ago. This is zero trust applied to the client itself, not just the connection.

- **The server re-authenticates the session continuously, not once.** Every check the client passed at login runs again, on a loop, for as long as the process is alive. Passing once buys you nothing later - you keep proving it.
- **Every message between client and server is signed, fresh, and single-use.** Nothing is replayable. A captured request, however perfectly captured, is worthless the moment it's reused.
- **The server holds full control over every live session, in real time.** It can end, message, or re-verify any session on demand - the client has no ability to resist, delay, or negotiate.
- **The binary and its runtime state are continuously verified against what was there at login.** Any modification, any injected code, any external interference with the running process is treated as a compromise - not logged, not flagged, acted on.
- **Detection never announces itself.** No dialog, no error, no exception, nothing to hook or intercept. The response to a failed check is the process ending - not a message telling an attacker what they tripped.
- **Nothing static ever sits in the client waiting to be stolen.** No reusable secret, no long-lived token, no single value that unlocks the next session if it leaks.

This is the actual model: authentication isn't a gate the client passes through once. It's a relationship the server keeps re-verifying, continuously, until the session ends - on the server's terms, not the client's.

---

## The API-key model

The API key is a **routing identifier** - it tells the server which dashboard account and application a request belongs to. It is not what authenticates a request. That rests on:

1. An X25519 handshake, deriving a fresh HMAC key per session.
2. The Ed25519 signature the server places on its handshake reply, verified against three keys pinned inside `Atlas Auth.lib` (primary, backup, emergency). A nulled server can't produce these signatures.
3. HWID binding - the session key is derived with the HWID mixed in, so a stolen session token doesn't work from a different machine.
4. A per-request nonce - replays are dropped.
5. The executable-hash whitelist, if you've configured one.

> [!IMPORTANT]
> A leaked API key alone doesn't let an attacker impersonate a user - but treat it as sensitive. Rotate it on suspected exposure (**Settings → Reset API Key**) and keep it out of public source.

---

## Troubleshooting

**`LNK2019: unresolved external symbol "Atlas::Startup"`** - `Atlas Auth.lib` isn't linked, or the library search path doesn't include the SDK. See [Integrate into your project](#integrate-into-your-project).

**`error C2039: 'API_KEY': is not a member of 'Atlas'`** - you're compiling against an outdated `Atlas.h`. Replace it with the current version from `Atlas SDK/`.

**Application exits during `Startup()`** - Atlas terminated the process after an integrity check failed. Verify `Atlas::API_KEY` is set correctly, the application still exists in the dashboard, and no debugger is attached (`Ctrl+F5`, not `F5`). See **Dashboard → Logs** for the exact failure reason.

**`Login()` returns `false`** - call `Atlas::Data::GetErrorMessage()` to determine the failure. Common causes include an executable hash mismatch (after rebuilding), an expired or banned license, a banned HWID, or invalid credentials.

**`Account::Login()` returns `NeedsVerification`** - no verified account exists yet. Configure an email sender for the application, verify email delivery in **Dashboard → Logs**, then call `Account::ResendVerification()` if needed (60-second cooldown).

**Process exits unexpectedly after authentication** - a runtime integrity check failed. Common causes include modified code sections, injected modules, hooked imports, failed server signature verification, or an extended heartbeat timeout. See **Dashboard → Logs** for the reported reason.

Please! view the when you have any runtime troubles [Atlas Diagnostic Logs](#diagnostic-logs)
Full FAQ: [atlassecurity.site/docs](https://atlassecurity.site/docs).

---

## Diagnostic logs

> [!IMPORTANT]
> Every session-ending event - a failed integrity check, a lost connection, a server-issued end to the session - is written to disk the moment it occurs, with the exact cause, source file, and line. The `logs\` folder itself always exists, on every machine running an Atlas-built application, end users included.

Press **`Win + R`**, paste:

```
%LOCALAPPDATA%\AtlasAuth
```

Each entry in `logs\` is a complete record of one event:

```
[Atlas Exit Report]
Time:   2026-08-02 8:38:50
Reason: CheckAuthentication: not authenticated or no session
File:   Atlas Auth.cpp
Line:   2258
```

> [!NOTE]
> The rest of `%LOCALAPPDATA%\AtlasAuth` - `installed.flag`, `declined.flag`, `commit.sha`, `manage_autoupdate.bat` - is dev-only. Those files exist to drive the MSBuild auto-update hook and only appear when a dev environment (Visual Studio, VS Code, MSBuild, JetBrains, and similar) is detected. `logs\` is the only part of this folder your end users will ever have. Always remember to check this folder to diagnose any issues, it is your #1 GOTO!

---

## Support

- **Docs** - [atlassecurity.site/docs](https://atlassecurity.site/docs)
- **Discord** - [discord.gg/EG5dmpFaCF](https://discord.gg/EG5dmpFaCF) (fastest response)
- **Email** - [mail@atlassecurity.site](mailto:mail@atlassecurity.site)

Bug reports: include your OS version, Visual Studio version, the failing SDK call, and the dashboard **Logs** entry if there is one.

The SDK's source is not distributed with this repo. If you need a custom build or believe you've found a bug in `Atlas Auth.lib` itself, contact support - don't attempt to reconstruct or patch the library from the header alone.

---

## Legal

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
