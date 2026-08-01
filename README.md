# Atlas SDK — C++

License authentication and continuous binary protection for Windows x64 C++ applications. The most complete authentication stack shipping for C++ today — verifiable in the header, the library, and this repo.

Two calls — `Atlas::Startup()` and `Atlas::License::Login(key)` — and your process authenticates, keeps verifying itself while it runs, and can be killed live from the dashboard.

---

Most C++ auth libraries stop caring after `Login()` returns `true`. Atlas doesn't. Login is the easy part; everything after it is what you're actually paying for.

- **Continuous integrity.** A per-session HMAC frame every 5 seconds, a `.text` + IAT recheck every 15, an inline-hook scan on `ws2_32.recv/send/connect` before every frame, and two independent threads checking each other with hardware performance counters. Any failure kills the process through a path with no user-mode handler to catch.
- **Hardware identity from 16 sources.** Firmware serials, TPM key hashes, PCI instance paths, per-device EDIDs — each a separate keyed hash. Spoof one, the other fifteen still ban.
- **Cascade bans.** Ban a license and the engine follows the HWID and IP unions across every account the fingerprint has ever touched, and bans the whole set.
- **Rules engine.** Per-app geofence, two-source anti-VPN (ip-api + proxycheck, cached 6 h), executable-hash whitelist. Cheapest checks first; first catch wins. All of it runs before the license table is opened.
- **Real email layer, built in.** 8-digit codes on new-device sign-ins, registration confirmation, password reset. Seller-branded, with IP / city / country / device on every code. No SMTP to configure.
- **Live control.** Kick sessions, ban HWIDs, push runtime variables without a rebuild. Every login lands in your dashboard **Logs** tab with IP, HWID, latency, and result.
- **Built-in Win32 dialogs.** Themed login, register, verify, and password-reset flows. Zero UI code if you don't want to write any.

**Also on the same account: [Atlas Obfuscator](https://atlassecurity.site/obfuscator)** — a Windows PE protector for the binary itself. Control-flow flattening, string encryption, VM-lifted hot paths, anti-debug and anti-dump baked into the output. Sold separately; bundled with Auth in Atlas Complete.

Free forever: 3 apps, 300 licenses across them, 3 file uploads per app. Full security stack, no feature gates. [Plans](https://atlassecurity.site/plans) lifts the caps.

[atlassecurity.site](https://atlassecurity.site) · [Dashboard](https://atlassecurity.site/dashboard) · [Docs](https://atlassecurity.site/docs) · [Legal](https://atlassecurity.site/legal) · [Discord](https://discord.gg/EG5dmpFaCF) · [mail@atlassecurity.site](mailto:mail@atlassecurity.site)

---

## What's in this folder

```
C++ Integration/
├── Atlas SDK/
│   ├── Atlas.h            the header you include
│   └── Atlas Auth.lib     the static library you link
├── Console Example/       plain console: license / account / register
└── ImGui Example/         native GUI with Dear ImGui + DX11
```

`Atlas Auth.lib` is prebuilt. You don't rebuild the SDK to use it.

---

## Prerequisites

- Windows 10 or 11, x64. No Linux, macOS, ARM.
- Visual Studio 2022 with the **Desktop development with C++** workload (MSVC v143 + Windows SDK + MSBuild).
- An Atlas account. Sign up at <https://atlassecurity.site>, then **Applications → New application** to grab an API key and **Licenses → Generate** for a test key (`ATLAS-XXXXX-XXXXX`).

No vcpkg, no CMake, no redistributables. The `.exe` runs on a stock Windows install.

---

## Run the Console example

1. Open `Atlas SDK/Atlas.h`. Replace `"YOUR_API_KEY"` with your key.
2. Open `Console Example/Atlas Auth Example.sln`.
3. Configuration: **Release · x64**. 32-bit will not link.
4. Build (`Ctrl+Shift+B`), run with **`Ctrl+F5`**. `F5` attaches the debugger, which the anti-debug check refuses.

The example asks which auth path to try:

```
Atlas Authentication Example

Choose an auth path:
  [1] License key       (single-user, HWID-bound)
  [2] Account sign-in   (username + password + email verification)
  [3] Register account  (create a new account)
```

Pick `[1]`, paste the license. On success:

```
--- User Information ---
License:      ATLAS-A9F2K-4RMXM
Expiry:       15-08-2026 14:32:00
IP:           45.11.42.187
HWID:         Atlas-4A9C...E1B2
Level:        1
Active Users: 1
Total Users:  3
```

The login is in your dashboard **Logs** tab with IP, HWID, latency, and result `ALLOW`. From **Sessions → Kick** you can terminate the session; the example ends within about five seconds.

Whole example is in `Console Example/Atlas Auth Example.cpp`.

---

## Run the ImGui example

Same SDK, native Windows GUI. Dear ImGui is MIT, upstream, and not shipped here — vendor it once:

```
cd "ImGui Example"
git submodule add https://github.com/ocornut/imgui.git imgui
git submodule update --init --recursive
```

Or download the source zip from <https://github.com/ocornut/imgui/releases> and extract into `ImGui Example/imgui/` so you have:

```
ImGui Example/imgui/
├── imgui.cpp, imgui_draw.cpp, imgui_tables.cpp, imgui_widgets.cpp, imgui.h
└── backends/
    ├── imgui_impl_dx11.cpp / .h
    └── imgui_impl_win32.cpp / .h
```

Reuse the API key. Open the `.sln`, build **Release · x64**, run with `Ctrl+F5`. A 940×640 login window opens; sign in and you land on a welcome panel with the session data and a **Sign out** button. The ImGui frontend is only a rendering layer — every protection thread from the console example is running underneath.

---

## Integrate into your project

1. Copy `Atlas SDK/Atlas.h` and `Atlas SDK/Atlas Auth.lib` into your project. A `vendor/atlas/` folder is a reasonable place.
2. In your Visual Studio project properties for **Release · x64**:
   - **C/C++ → General → Additional Include Directories** — the folder holding `Atlas.h`.
   - **Linker → General → Additional Library Directories** — same folder.
   - **Linker → Input → Additional Dependencies** — add `Atlas Auth.lib;` verbatim (the space in the filename is intentional).
3. Set your API key inline in `Atlas.h` for development, or from your own code before `Startup()`:
   ```cpp
   Atlas::API_KEY = LoadKeyFromSignedRemoteConfig();
   Atlas::Startup();
   ```
4. Wire it up:
   ```cpp
   #include "Atlas.h"

   int main() {
       Atlas::Startup();

       std::string key = PromptUserForLicense();
       if (!Atlas::License::Login(key)) {
           std::cout << Atlas::Data::GetErrorMessage();
           return 1;
       }

       RunMyApplication();   // authenticated
       return 0;
   }
   ```

Once you have a shipping build, compute its SHA-256 and paste it into **Applications → Executable-hash whitelist**. Modified copies get rejected server-side before the license is even checked. Multiple hashes are allowed, one per release.

---

## API reference

Full surface in `Atlas SDK/Atlas.h`. Summary here.

### Session

```cpp
Atlas::API_KEY = "your-key";   // set before Startup, or leave inline in Atlas.h
Atlas::Startup();              // call once, at the top of main()
Atlas::Logout();               // end the session, clear all state
Atlas::Exit();                 // kill the process the hardest way Windows allows
```

### `Atlas::License` — license-key sign-in

```cpp
Atlas::License::Login(key);                             // key only (HWID-bound)
Atlas::License::Login(username, password);              // for a license bound to one user
Atlas::License::Register(key, username, password);      // bind — does NOT sign in

// Built-in Win32 dialogs, modal:
Atlas::License::Dialog::Login();
Atlas::License::Dialog::Register();
```

### `Atlas::Account` — username / password / email accounts

```cpp
auto r = Atlas::Account::Login(username, password);     // inspect r.status
Atlas::Account::Register(username, password, email);    // email optional; needed for reset
Atlas::Account::SubmitVerification(code);               // 8-digit sign-in code
Atlas::Account::ResendVerification();                   // 60s cooldown
Atlas::Account::ConfirmEmail(code);                     // for a pending registration
Atlas::Account::HasPendingEmailConfirm();
Atlas::Account::Redeem(license_key);                    // add a license to the signed-in account
Atlas::Account::RequestPasswordReset(identifier);       // always returns true (anti-enumeration)
Atlas::Account::CompletePasswordReset(code, new_pass);

// Built-in Win32 dialogs, modal:
Atlas::Account::Dialog::Login();
Atlas::Account::Dialog::Register();
Atlas::Account::Dialog::VerifyCode();
Atlas::Account::Dialog::ConfirmEmail();
Atlas::Account::Dialog::ResetPassword();
```

`LoginResult::Status` is one of `Ok`, `WrongCredentials`, `NeedsVerification`, `Banned`, `AccountPaused`, `ServerUnreachable`, `Error`. On `NeedsVerification` the server emailed the user an 8-digit code — pass it back through `SubmitVerification`. On `Ok`, `r.expiry` / `r.level` / `r.note` are populated.

### `Atlas::Data` — session state, valid after sign-in

```cpp
// Identity
GetLicense()  GetUsername()  GetEmail()  GetPassword()  GetIP()  GetHWID()  GetDevice()
GetNote()  GetFirstSeenDate()  GetLastSeenDate()  GetUserId()  GetLevel()

// Expiry
GetExpiry()  GetDaysRemaining()  IsLifetime()  IsExpiringSoon(days = 7)

// Status
IsAuthenticated()  IsBanned()

// App-wide
GetActiveUserCount()  GetUserCount()

// Errors
GetErrorMessage()  HasError()  ClearError()
```

### `Atlas::Network` — server RPCs on the current session

```cpp
CheckAuthentication();                        // force a fresh server round-trip
Download(file_id);                            // dashboard file → std::vector<uint8_t>
BanUser(reason, duration_minutes);            // duration = 0 → permanent
SubmitLog(text);                              // ≤ 512 chars, shows in Logs
ChangePassword(old_password, new_password);
Ping();                                       // ms to auth server, -1 if unreachable
```

### `Atlas::Variables` — server-set config, no rebuild required

```cpp
Atlas::Variables::Fetch("welcome_msg");       // "" if the key doesn't exist
Atlas::Variables::FetchBool("beta_feature");  // "true" / "1" / "yes" → true; else false
Atlas::Variables::FetchInt("max_items");      // 0 if missing or unparseable
```

### `Atlas::Webhook` — fire-and-forget HTTP POSTs (unrelated to Atlas auth)

```cpp
Atlas::Webhook::SendDiscord(webhook_url, message);
Atlas::Webhook::SendDiscordEmbed(webhook_url, title, description, color);  // color = 0xRRGGBB
Atlas::Webhook::Send(url, json_payload);
```

### `Atlas::Dialog` — theming for every built-in Win32 dialog

```cpp
Atlas::Dialog::AppName = "My App";                       // title bar text
Atlas::Dialog::theme   = Atlas::Dialog::Theme::Dark;     // Dark or Light
Atlas::Dialog::parent  = my_main_hwnd;                   // modal parent, null = foreground
Atlas::Dialog::accents.signal = 0xFFE04A2C;              // 0xAARRGGBB colour overrides
Atlas::Dialog::copy.verify_title = "Enter code";         // string overrides
Atlas::Dialog::FatalError(title, body, error_code);
```

---

## The API-key model

The API key is a routing identifier. It tells the server which dashboard account the request belongs to. Authentication of each request rests on five things:

1. The X25519 handshake — derives a per-session HMAC key only your app and the server know.
2. The Ed25519 signature the server places on its handshake reply — verified against three keys pinned inside `Atlas Auth.lib` (primary, backup, emergency). A nulled server cannot produce these signatures.
3. The HWID binding — the session key is derived with the HWID mixed in; a stolen session token doesn't work from a different machine.
4. The per-request nonce — replays are dropped.
5. The executable-hash whitelist, if you configured one.

A leaked API key does not by itself let someone impersonate a user. Still, treat it as sensitive: rotate on suspected exposure (**Settings → Rotate key**), keep it out of public source.

---

## Troubleshooting

**`LNK2019: unresolved external symbol "Atlas::Startup"`**
`Atlas Auth.lib` isn't in the linker inputs or its folder isn't in the library search path. See integration step 2.

**`error C2039: 'API_KEY': is not a member of 'Atlas'`**
Old `Atlas.h`. Copy the current one from `Atlas SDK/`.

**Startup terminates the process immediately.**
API key still `"YOUR_API_KEY"`, API key for a deleted app, or a debugger attached (try `Ctrl+F5`). Dashboard **Logs** has the exact reason.

**`Login` returns `false`, "Executable hash mismatch".**
You whitelisted a hash then rebuilt. Update the whitelist, or don't whitelist during active development.

**`Login` returns `false`, "License banned" / "HWID banned".**
Check **Bans**.

**Process exits silently.**
Integrity check tripped the kill path. Dashboard **Logs** shows the reason — `.text` modified, IAT hooked, injected module, server signature verification failed, session frames couldn't reach the server for too long.

**The `.exe` is large.**
`Atlas Auth.lib` is ~15 MB — that's the whole protection stack. UPX compresses it about 4:1.

Full FAQ: <https://atlassecurity.site/docs>.

---

## Support

<https://atlassecurity.site/docs> · [Discord](https://discord.gg/EG5dmpFaCF) · [mail@atlassecurity.site](mailto:mail@atlassecurity.site)

To rebuild `Atlas Auth.lib` from source (only if you're modifying SDK internals): open `Auth Library/Atlas Auth/Atlas Auth.sln`, build **Release · x64** or **Ship-All · x64**. The output overwrites `C++ Integration/Atlas SDK/Atlas Auth.lib` in place.

---

## Legal

© 2025–2026 Atlas Security Solutions. All rights reserved. Sold by Atlas Security Solutions, Jeddah, Kingdom of Saudi Arabia. This SDK exists so developers can integrate Atlas Authentication into their software — if that's you, use it freely.

Prohibited without written authorization: reverse engineering, decompiling, or reconstructing Atlas binaries, protocols, or server infrastructure; tampering with, bypassing, or disabling any authentication or anti-tamper control; probing or interfering with Atlas servers; using knowledge of Atlas internals to build competing platforms or bypass tools. Atlas monitors for unauthorized access and pursues violations under Saudi Arabia Anti-Cybercrime Law (Royal Decree M/17, 1428H, Articles 3–4), the U.S. Computer Fraud and Abuse Act (18 U.S.C. § 1030), EU Directive 2013/40/EU, and WIPO / TRIPS. Remedies include civil action, injunctive relief, and cross-jurisdiction enforcement without prior notice.

Permission requests and legal inquiries: [mail@atlassecurity.site](mailto:mail@atlassecurity.site) · <https://atlassecurity.site/legal>
