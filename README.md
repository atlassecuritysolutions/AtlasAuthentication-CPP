# Atlas Authentication — C++ SDK Example

**Website** · [atlassecurity.site](https://atlassecurity.site) &nbsp;|&nbsp; **Docs** · [atlassecurity.site/docs](https://atlassecurity.site/docs) &nbsp;|&nbsp; **Plans** · [atlassecurity.site/plans](https://atlassecurity.site/plans) &nbsp;|&nbsp; **Discord** · &nbsp;|&nbsp; **GitHub** ·

---

This repository contains the example integration for the Atlas Authentication SDK. It demonstrates a minimal Windows console application that performs a full authenticated session — license validation, hardware binding, and live session tracking — using two function calls.

[![Plans](https://atlassecurity.site/readme-plans.png)](https://atlassecurity.site/plans)

Free tier for life includes the full security stack — HWID binding, anti-debug, encrypted transport, proof-of-work — with limits of 3 apps, 300 licenses and 3 files per app. Premium removes all caps starting at $9.

---

[![Docs](https://atlassecurity.site/readme-docs.png)](https://atlassecurity.site/docs)

Full SDK reference, platform architecture, and integration guide at [atlassecurity.site/docs](https://atlassecurity.site/docs).

---

## What this example does

[![Example running — successful login](https://atlassecurity.site/readme-cmd.png)](https://atlassecurity.site/docs)

Prompts for a license key, connects to the Atlas server, and on success prints the session data — expiry, IP, HWID, level, note, active user count. The entire auth stack is active from that point: heartbeat, integrity checks, watchdog, remote termination, this is exactly what THIS repo is, the Atlas example.

---

## Dashboard — Bans

[![Ban management](https://atlassecurity.site/readme-bans.png)](https://atlassecurity.site)

Bans issued from the dashboard lock by license key, HWID, IP, and up to 16+ hardware component hashes simultaneously. The notification confirms the ban was recorded against all hardware components. Active immediately — Simply enter a value, server finds matching details from IP's, licenses, HWID's, and even individual identifiers, and bans all in a cascade.

---

## Dashboard — Logs & Analytics

[![Connection logs and analytics](https://atlassecurity.site/readme-analytics.png)](https://atlassecurity.site)

Every event is logged with timestamp, type, license, IP, location, device, and HWID. Admin actions — session termination, messages sent — appear inline. The analytics tab shows auth response time, server load, uptime, and a live geographic heatmap of active connections.

---

## Integration

```cpp
#include "Atlas.h"

int main() {
    Atlas::Startup();
    Atlas::Login(key);
    // authenticated — full protection stack active
}
```

Windows x64 · Release · no external dependencies
