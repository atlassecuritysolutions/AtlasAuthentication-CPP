# Atlas Authentication — C++ SDK Example

**Website** · [atlassecurity.site](https://atlassecurity.site) &nbsp;|&nbsp; **Docs** · [atlassecurity.site/docs](https://atlassecurity.site/docs) &nbsp;|&nbsp; **Plans** · [atlassecurity.site/plans](https://atlassecurity.site/plans) &nbsp;|&nbsp; **Discord** · [discord.gg/EG5dmpFaCF](https://discord.gg/EG5dmpFaCF) &nbsp;|&nbsp; **GitHub** · [atlassecuritysolutions](https://github.com/atlassecuritysolutions) &nbsp;|&nbsp; **Email** · [mail@atlassecurity.site](mailto:mail@atlassecurity.site)

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

## What Atlas is

Atlas is a **hardware-bound license authentication and software protection platform** built specifically for C++ applications. It is not a license key validator — it is an active protection system that runs throughout the session.

**What makes it different from every other C++ auth system:**

Every 5 seconds after login: executable CRC verified, import table checked, network functions scanned, watchdog threads cross-checking each other. If anything deviates from the startup snapshot — NOP patch, injected DLL, debugger attached, memory edited — the process terminates via `__fastfail()`. No dialog. No recovery path. The kernel terminates it directly.

**The authentication stack, in order:**

1. **Proof-of-work gate** — ~50ms for real users, hours for bots. Applied before any auth logic runs.
2. **Executable hash check** — your binary's SHA-256 must match the whitelist registered in the dashboard. Modified builds are rejected at the door.
3. **Hardware fingerprint** — 16+ components across firmware, storage, NIC, and platform security. The fingerprint is a keyed hash; it cannot be reconstructed without the server secret.
4. **License validation** — expiry, level, HWID binding, concurrent session limit, IP and country restrictions — all checked server-side in a single round trip.
5. **Ban vector check** — license key ban, full HWID ban, per-component ban (NIC, firmware, volume serial independently), and IP ban — applied in parallel.
6. **HMAC-signed response** — every successful auth response carries a keyed HMAC over the nonce, license key, and session ID. A nulled server cannot forge a valid reply.
7. **Session negotiation** — auth token fragmented across 4 non-adjacent memory pages with compile-time salt. Token rotates every heartbeat.

**The live enforcement stack (every 5 seconds after login):**

- Code section CRC against startup snapshot — catches all in-memory patches
- Import Address Table integrity check — catches hook injection and manual mapping
- Network function hook scan — WinSock/WinHTTP hook detection before any heartbeat data leaves
- Injection detection — unexpected loaded modules trigger immediate exit
- Hardware debug register check — `DR0–DR7` inspected before every sensitive fetch
- Watchdog mutual liveness — two threads cross-check each other; if either dies, both processes exit via `__fastfail()`

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

**`Atlas::Startup()`** — initializes crypto primitives, snapshots the executable pages (baseline for CRC verification), starts the mutual watchdog threads, resolves all API imports via PEB walking and hash-based export matching. No readable import strings remain after this call.

**`Atlas::Login(key)`** — sends the license key through the encrypted transport (per-request key derivation, HMAC-signed), validates against the server, binds the hardware fingerprint, stores the session token across 4 non-adjacent memory fragments, and starts the heartbeat loop. Everything else — integrity checks, anti-debug, remote kill handling — runs automatically from this point.

That is the entire integration. Windows x64 · Release · no external dependencies.

---

## Dashboard — Bans

[![Ban management](https://atlassecurity.site/readme-bans.png)](https://atlassecurity.site)

Bans issued from the dashboard lock by license key, HWID, IP, and up to 16+ hardware component hashes simultaneously. The notification confirms the ban was recorded against all hardware components. Active immediately — Simply enter a value, server finds matching details from IP's, licenses, HWID's, and even individual identifiers, and bans all in a cascade.

**Ban vectors (applied independently):**
- **License key** — key invalidated server-side, rejected at next heartbeat
- **Full HWID** — the combined fingerprint hash is banned
- **Per-component** — ban an individual NIC MAC, firmware UUID, or volume serial; the user cannot spoof just that component without shifting the full fingerprint
- **IP address** — global across all your applications
- **Deep ban** — flags all known fingerprint variants associated with the user

All bans propagate within one heartbeat cycle (≤5 seconds of the user's next check-in).

---

## Dashboard — Logs & Analytics

[![Connection logs and analytics](https://atlassecurity.site/readme-analytics.png)](https://atlassecurity.site)

Every event is logged with timestamp, type, license, IP, location, device, and HWID. Admin actions — session termination, messages sent — appear inline. The analytics tab shows auth response time, server load, uptime, and a live geographic heatmap of active connections.

**Log fields per entry:** timestamp · event type · license key · IP address · geolocation · device name · HWID hash · result · latency

**Filter by:** result (ALLOW / DENY / BAN) · license key · IP address · date range

---

## Server architecture

The Atlas backend is a **128-worker C++ TCP server** with a 16-shard session map and a 16-connection PostgreSQL pool.

- All application data (licenses, bans, hardware serials, session state) is cached in memory with microsecond invalidation on ban
- Session map is sharded — no global lock on concurrent auth requests
- Stat increments are batched and flushed to the database every 60 seconds, not on the hot path
- Log writes are async — zero I/O latency impact on auth response time
- Auth response time under load: **≤50ms**

---

**Premium starts at $19/month.** Monthly ($19), 6-month ($69, save 39%), annual ($99, save 57%). PayPal and crypto. Instant activation. [See plans →](https://atlassecurity.site/plans)

---

## Legal Notice

© 2025–2026 Atlas Security Solutions. All rights reserved.

This SDK exists for one purpose: to let developers integrate Atlas Authentication into their software. If you are a developer building an application and using this code to license and protect it through Atlas — you are exactly who this is for. Use it freely.

**The following acts are strictly prohibited without explicit written authorization from the owner, and apply to those who seek to abuse, exploit or undermine the Atlas platform, Atlas reserves all rights to pursue legal action:**

- Reverse engineering, decompiling, disassembling, or reconstructing the Atlas platform, its compiled binaries, network protocols, or server infrastructure
- Tampering with, bypassing, disabling, or circumventing any authentication check, anti-tamper control, or security mechanism within the Atlas system
- Accessing, probing, or interfering with Atlas servers, databases, or infrastructure without authorization
- Using knowledge of Atlas internals to build, assist, or contribute to competing platforms or security bypass tools

**Applicable Law & Enforcement:**

These acts constitute criminal and civil offenses enforceable under:

- **Saudi Arabia:** Anti-Cybercrime Law (Royal Decree No. M/17, 1428H) — Articles 3 and 4
- **United States:** Computer Fraud and Abuse Act (18 U.S.C. § 1030)
- **European Union:** Directive 2013/40/EU on Attacks Against Information Systems — binding across all EU member states
- **International:** WIPO Copyright Treaty and the TRIPS Agreement — enforceable across 180+ signatory nations

These instruments collectively provide jurisdiction and enforcement mechanisms across all major territories worldwide.

Atlas Security Solutions actively monitors for unauthorized access, reverse engineering attempts, and protocol analysis. Any violation will be met with immediate civil action, referral to the competent national authorities in the relevant jurisdiction, and pursuit of all available legal remedies — including injunctive relief, asset recovery, and cross-jurisdiction enforcement — without any prior notice or warning.

For permission requests or legal inquiries: [mail@atlassecurity.site](mailto:mail@atlassecurity.site) · [atlassecurity.site/legal](https://atlassecurity.site/legal)
