# Atlas Authentication — C++ Console Example

The minimum viable Atlas integration. A Windows console app that prompts for a license, authenticates against the Atlas server, and prints the resulting session data. About 60 lines of code — read it once and you understand the whole SDK.

**For onboarding (create an Atlas account, get an API key, generate a license), see [`../README.md`](../README.md).** This file covers only what's specific to this example.

---

## What's here

```
Console Example/
├── README.md                            you are here
├── Atlas Auth Example.cpp               the whole example — ~60 lines
├── Atlas Auth Example.sln               Visual Studio 2022 solution
├── Atlas Auth Example.vcxproj           MSBuild project (already wired to ../shared/)
└── Atlas Auth Example.vcxproj.filters   VS solution explorer grouping
```

Links against [`../shared/Atlas Auth.lib`](../shared/) and includes [`../shared/Atlas.h`](../shared/Atlas.h). No other project deps.

---

## Build

1. **Set your API key** in [`../shared/Atlas.h`](../shared/Atlas.h):
   ```cpp
   namespace Atlas { inline std::string API_KEY = "your-key"; }
   ```
2. Open `Atlas Auth Example.sln` in Visual Studio 2022.
3. Configuration: **Release · x64** (Atlas is x64 only).
4. Build: `Ctrl+Shift+B`.

Output: `Atlas Auth Example.exe` in `x64/Release/`.

---

## Run

`F5` (with debugger) or `Ctrl+F5` (without). A console window opens:

```
Atlas Authentication Example

Enter license:
```

Paste your test license key, press Enter. On success:

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

Now check the dashboard's **Logs** tab — the login entry is there with your IP, HWID, latency, result = `ALLOW`. Kick the session from **Sessions → Kick** and the example terminates within 5 seconds via `__fastfail`. Full loop verified.

---

## What the code does

Just the three moves:

1. **`Atlas::Startup()`** — initializes crypto primitives, snapshots the executable pages (baseline for CRC verification), starts the mutual watchdog threads, resolves all API imports via PEB walking. Call once at program start.

2. **`Atlas::Login(license)`** — sends the license through the encrypted transport, validates against the server, binds the hardware fingerprint, stores the session token across 4 non-adjacent memory fragments. Returns `true` on success, `false` on rejection (read `Atlas::Data::GetErrorMessage()` for the reason).

3. **`Atlas::Network::CheckAuthentication()`** and the `Atlas::Data::Get*()` family — the accessors you use in your app.

That's the shape. Full API surface in [`../shared/Atlas.h`](../shared/Atlas.h); reference table in [`../README.md`](../README.md#api-reference).

---

## Prefer a native GUI?

If you want a real windowed login form instead of a console prompt, use [`../ImGui Example/`](../ImGui%20Example/) — same Atlas SDK, native Windows GUI with Dear ImGui + DirectX 11.

---

## License / legal

See [`../README.md`](../README.md#legal-notice).
