# Security

RBLX Clone is a prototype meant to run on one machine. It is not hardened for the public
internet, and this file says exactly what it does do, what it does not, and how to report a
problem. Read it with the [README](README.md) sections on anti-cheat and the web API.

Only the current `main` and the tip of this branch are supported. There are no releases to
backport to and no security team; fixes land on `main`.

## Reporting a vulnerability

Open a **private security advisory** on this repository (Security → Advisories → *Report a
vulnerability*). Do not open a public issue, and do not include a working exploit in a
public discussion.

Include what you did, what happened, which program and build (`Release`/`Debug`, and what
`RBLX_BUILD_*` options), and the smallest reproduction you have. A packaged build and a
source build of the same commit behave the same unless an option was turned off.

There is no bug bounty and no guaranteed response time. Do not test against a machine you
do not own, and do not put other people's data at risk.

## What is known and deliberate

These are documented limitations, not vulnerabilities. A report that says "the client can
lie" or "there is no TLS" is answered by the relevant paragraph below.

- **Local-only deployment.** `WebApp.exe` accepts connections on every interface, and game
  servers bind every interface too, so anything on the same network can reach them if the
  firewall allows it. The intended setup is one machine: keep the Windows firewall on, or
  bind the game servers to loopback with `GAME_SERVER_HOST=127.0.0.1`.
- **No TLS.** The website is plain HTTP and `include/HttpClient.h` speaks `http://` only;
  there is no TLS implementation anywhere in the tree. Any token or password over a network
  is readable in transit. Put it behind a TLS-terminating proxy before it faces anything
  other than loopback and trusted LAN clients.
- **The client is untrusted.** `RBLXAntiCheat` runs inside the client process, so a
  modified client can decline to run it, and the machine id the client sends can be any
  value. Neither is a boundary. The boundary is the server: the movement guard in
  `include/MovementGuard.h`, the ban ladder in `include/Moderation.h`, and the login check a
  server makes before letting a player in.
- **Data is a JSON file.** Accounts, worlds, moderation records and sessions live in
  `web/users.json` and `web/game-worlds/`. There is no database, no migration tool and no
  concurrency story beyond one `WebApp.exe` process owning the file.

## What is implemented

**Passwords.** Stored as `pbkdf2-sha256$<rounds>$<salt>$<hash>` — PBKDF2-HMAC-SHA256 with
a fresh 16-byte random salt per password and a 120000-round default. Verification is
constant-time. The plaintext is never written to the data file or logged. Changing the
iteration count leaves existing hashes readable, because the count travels with each hash.

**Sessions.** The login response is a JWT signed with HMAC-SHA256 through
`src/WebCrypto.cpp`, carrying `sub`, `iat` and `exp` (`JWT_EXPIRES_IN`, seven days by
default). Authenticated routes take `Authorization: Bearer <token>`; tokens are no longer
accepted in avatar URLs, and `/api/verify` keeps a body-token fallback only for older
clients.

**Secrets.** `JWT_SECRET` must be set to a strong, non-default value when
`NODE_ENV=production`; the backend refuses to start otherwise. `DATA_KEY` is the key the
data file is sealed with; when it is unset the key is derived from `JWT_SECRET` and the
backend says so on start, because rotating `JWT_SECRET` then makes the existing file
unreadable. `ADMIN_USERNAMES` decides which accounts are administrators; a seeded starter
account owns the starter games, so give it a real password before anyone else can reach
the install.

**Data at rest.** The store seals account secrets — password hashes, recovery codes and
the rest of the sensitive fields — with AES-256-GCM, per-user associated data, and records
`"cipher": "aes-256-gcm"` in the file. A wrong `DATA_KEY` is reported as such instead of
being read as empty data. This protects a leaked `users.json`, not a compromised machine:
the key is in the process's environment.

**Accounts and recovery.** Passwords can only be changed with the current password (or a
recovery code, which is rotated on use). Deleting an account removes what it owns. A
signup returns a one-time recovery code because there is no email — losing it means losing
the account, which is a recovery-policy trade-off, not a bug.

**Developer tokens and the AI trainer.** `AiTrainer.exe` cannot run without signing in. A
token is minted in `/developers` by a signed-in account, is 192 random bits behind an
`rblxai_` prefix, and only its SHA-256 is stored, so the value is shown once and a copy of
the data file cannot be replayed. The token carries a scope (`ai`) that no other route
accepts: it cannot be spent as an account session, and `POST /api/ai/login` is the only
route that reads one. A token belongs to the account that made it, is refused while that
account is suspended or banned, and stops working the moment it is revoked. What it buys is
narrow: the map and course of a game whose owner ticked **AI compatible**, and only of a game
the token's account owns or of one that is public. An unmarked game is refused for every
token, and another account's private game answers as though it did not exist. Tokens are
capped at eight per account and 500 per install, and a use records a last-used time. Training
requests count against the same per-address budget as the rest of `/api/`; there is no
separate quota, so a program that retries a bad token spends that address's minute.

`AiTrainer --play` is deliberately not a privileged join: it connects to a game server as a
guest, with the machine's hardware id and nothing else, so the handshake, the hardware ban,
the movement guard and every other player-facing check apply to the AI exactly as they do to
a person. A token cannot start or enter an instance on its own; the server it plays on was
launched by somebody who did have one of those rights.

**Request handling.** A JSON body is capped (`JSON_BODY_LIMIT_BYTES`, 2 MiB by default), a
published world at 2 MiB, a game script at 256 KiB. Failed authentication is rate-limited
per address (`AUTH_RATE_LIMIT_MAX` 20 per `AUTH_RATE_LIMIT_WINDOW_MS` 15 minutes) and every
route shares a second budget (`RATE_LIMIT_MAX` 600 per minute). Signup, login and recovery
sit behind a captcha: a proof-of-work challenge plus a human-readable image, both expiring
after five minutes. Every response carries `X-Content-Type-Options: nosniff`,
`X-Frame-Options: DENY`, `Referrer-Policy: same-origin` and a strict
`Content-Security-Policy` (`script-src 'self'`, no inline script, `object-src 'none'`,
`frame-ancestors 'none'`). CORS allows only the site's own origin, plus localhost in
development.

**The control panel.** `RBLXHosts.exe` serves its dashboard on `127.0.0.1` and refuses a
request that does not carry the token it printed at startup; opening the printed link
exchanges that token for an `HttpOnly`, `SameSite=Strict` cookie, and a mutating request
whose `Origin` is not the panel's own is refused. There is no account behind it and no
TLS, so it is a loopback tool: whoever can read that console controls the pool. Setting
`dashboard-host` to a LAN address hands the same power to the network, and the docs say
to leave it alone. The panel reads game-server logs, so it shows whatever a server chose
to print.

**Moderation as security.** A confirmed speed or fly detection — from the server's movement
guard, not from the client's self-report — is held and applied in a wave, so one burst of
detections is one step: two warnings, then 1-, 3- and 7-day suspensions, then a ban. Client
detections are filed against the caller's own account only. Bans also bar the machine the
account was last seen on (`POST /api/hardware-check` for guests), as a 32-character hash of
the system volume serial and the computer name; the raw values never leave the client.

**Chat.** A message is read back by game id and channel, never by a caller-supplied index, so a
room cannot be reached around the game it belongs to; a private game's room answers `404` to
everyone but its owner. Posting needs a session, and the text is cleaned server-side before it is
stored — control characters stripped, CRLF folded, blank-line runs collapsed, 500 characters at
most — so what the page renders is what the file holds and a paste cannot smuggle terminal
sequences into it. A suspended or banned account can read and cannot post. Posting is budgeted per
account (five messages per ten seconds by default) as well as by the site-wide `/api/` budget, so
one account cannot flood a room and a room cannot be used to exhaust an address's minute. Blocks
hide both accounts' messages from each other. History is bounded at 200 messages per channel and
4000 per install, oldest first, so the data file cannot be filled through chat, and deleting a game
or an account deletes the messages that belonged to it. Presence — who is in which room — lives in
memory for five minutes and is never written to disk.

## Changing security-relevant code

Anything under `src/WebCrypto.cpp`, `src/WebStore.cpp`, `src/WebApp.cpp`'s auth and
moderation paths, `include/Moderation.h`, `include/MovementGuard.h`,
`include/ClientCheatReport.h` or `src/HardwareId.cpp` should keep the tests beside it
(`tests/store_crypto_tests.cpp`, `web_app_tests.cpp`, `web_captcha_tests.cpp`,
`moderation_tests.cpp`, `movement_guard_tests.cpp`, `client_cheat_report_tests.cpp`,
`hardware_id_tests.cpp`) passing, and this file accurate. The supervisor's manual start,
stop and restart live in `src/ServerHost.cpp` and are covered by
`tests/server_host_tests.cpp`; the panel's token and origin checks live in
`src/HostDashboard.cpp`. If a change makes one of the statements above false, update the
statement in the same change.
