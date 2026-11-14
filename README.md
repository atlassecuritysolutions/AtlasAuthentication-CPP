# Atlas Auth for C++

![Platform](https://img.shields.io/badge/platform-Windows%20x64-0078D6?logo=windows&logoColor=white) ![Language](https://img.shields.io/badge/language-C%2B%2B17-00599C?logo=cplusplus&logoColor=white) ![License](https://img.shields.io/badge/license-MIT%20source%20%C2%B7%20proprietary%20lib-lightgrey)

[atlas.se.net](https://atlas.se.net) · [Dashboard](https://atlas.se.net/dashboard) · [Docs](https://atlas.se.net/docs) · [Discord](https://discord.gg/EG5dmpFaCF) · [mail@atlas.se.net](mailto:mail@atlas.se.net)

<p align="center"><a href="https://atlas.se.net"><img src="https://atlas.se.net/og-image.png" alt="Atlas. Licensing and binary protection for Windows software." width="100%"></a></p>

## Sell software that can't be shared or read.

**Authorization that holds under active attack.**

Most licensing checks run once, at sign-in, and trust the client after that. Atlas keeps checking. A key locks to the first PC that signs in, and from then on the library proves to the server, on its own threads, that the process is still the one that signed in. Every sign-in, refusal and live session shows up in your dashboard, and you can message, end or ban any of them while the app runs.

This is the C++ SDK: a static library and one header. No DLL, no redistributables, no servers to run.

## What you control

- **Licenses or accounts.** Keys that lock to a machine, or usernames and passwords, per app.
- **Emailed codes.** An 8-digit sign-in code on the policy you pick, from never to every sign-in.
- **Live sessions.** Message, disconnect or kill any session. One ban covers the key, the IP and the hardware.
- **Rules.** Block by country, VPN or device count. Alert rules that act on what they see, with Discord and webhook delivery.
- **Variables, files and entitlements.** Change a value, serve a file or meter a credit without shipping a build.
- **Updates.** Upload a build, promote it, and roll it out to 10% first. Clients verify the SHA-256, swap the file and relaunch.

## Start in minutes

Requirements: Windows 10 or 11 x64, Visual Studio 2022 with **Desktop development with C++**, C++17 or newer.

1. **Create an app.** Sign up at [atlas.se.net](https://atlas.se.net), create an application in the dashboard and copy its API key.
2. **Add the SDK.** Copy `Atlas SDK/Atlas.h` and `Atlas SDK/Atlas Auth.lib` into your project. For **Release | x64**, add their folder to the include and library directories and `Atlas Auth.lib;` to the linker's additional dependencies.
3. **Sign in.**

```cpp
#include <Atlas.h>
#include <iostream>

int main() {
    Atlas::API_KEY = "YOUR_API_KEY";    // Dashboard > Applications
    Atlas::Startup();                   // once, first

    if (!Atlas::License::Login("ATLAS-A9F2K-4RMXM")) {
        std::cout << Atlas::Data::GetErrorMessage();    // says why
        return 1;
    }
    // signed in
}
```

Free to start: three applications, 300 users per app, three file uploads per app and the full protection engine. No card. The Console project in this repo is already wired to the same two files.

## Everything else is in the docs

Every call, status code and edge case is documented, with examples in all three languages.

| You want | Read |
|---|---|
| The whole picture in one page | [Overview](https://atlas.se.net/docs?p=sdk/overview) |
| License keys | [License](https://atlas.se.net/docs?p=sdk/license) |
| Accounts, emailed codes, password reset | [Account](https://atlas.se.net/docs?p=sdk/account) |
| Session control, file downloads, bans | [Network](https://atlas.se.net/docs?p=sdk/network) |
| Dashboard values and entitlements | [Variables](https://atlas.se.net/docs?p=sdk/variables) · [Entitlements](https://atlas.se.net/docs?p=sdk/entitlements) |
| Restyling the built-in windows | [Dialog](https://atlas.se.net/docs?p=sdk/dialog) |

## Shipping

- **Hash whitelist.** Drop your `.exe` into **Settings → Builds** in the dashboard. A modified copy is refused before its license is read. [More](https://atlas.se.net/docs?p=concepts/hash-whitelist).
- **Running-build proof (1.1.2 and later).** Each sign-in and heartbeat also proves the loaded code matches the file you uploaded. Set it to log or enforce per app, in the same place.
- **Updates for your users.** Upload under **Versions**, promote, and older clients replace themselves on their next launch. [More](https://atlas.se.net/docs?p=guides/auto-update).
- **Updates for the SDK.** On a development machine the library keeps `Atlas Auth.lib` and `Atlas.h` current from signed releases and keeps your API key. Its state lives in `%USERPROFILE%\.atlassecurity`.

When a process ends unexpectedly, the cause is already on disk: `%USERPROFILE%\.atlassecurity\logs\atlas_exit_*.log` holds the reason, file and line. [Reading the log](https://atlas.se.net/docs?p=diagnostics/logs).

## Atlas Obfuscator

The other half of Atlas protects the compiled file itself. Upload an `.exe` or `.dll` (x64 or x86), choose options and download the protected build. No source and no SDK. [See it work](https://atlas.se.net/obfuscator), with real captures from protected builds.

## Support

[Docs](https://atlas.se.net/docs) · [Discord](https://discord.gg/EG5dmpFaCF) (fastest) · [mail@atlas.se.net](mailto:mail@atlas.se.net). For a bug report, include your Windows and Visual Studio versions, the failing call, and the **Dashboard → Logs** entry or the newest `atlas_exit_*.log`.

<p align="center"><a href="https://atlas.se.net"><img src="https://atlas.se.net/closing-banner.png" alt="Start free. Upgrade when you reach a ceiling. Both products have a free tier with the full protection engine. No card to start." width="100%"></a></p>

## License

The header (`Atlas.h`) and the example code in this repository are released under the MIT License - see `LICENSE`. Everything below applies to `Atlas Auth.lib`, to Atlas services, and to Atlas internals.

© 2025–2026 Atlas Security. All rights reserved.
Sold by Atlas Security - Jeddah, Kingdom of Saudi Arabia.

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

Authorized inquiries only: [mail@atlas.se.net](mailto:mail@atlas.se.net) · [atlas.se.net/legal](https://atlas.se.net/legal)
