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
- **The repository browser has no accounts.** `RBLXHub.exe` listens on `127.0.0.1:3100`
  unless it is started with `--host`, and it has no login: whoever can reach the port can read
  the repository and open issues. It is a local view of a local repository, not a forge — do
  not expose it with `--host 0.0.0.0` and expect access control.
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
`NODE_ENV=production`; the backend refuses to start otherwise. Outside production an unset
secret is not replaced by a constant: it is generated from `BCryptGenRandom` on the first
start and kept in `jwt-secret` beside the data file. A constant would be a published signing
key, because this source is public and the listener binds `0.0.0.0`; and a fresh value on
every start would change the data-file key. `DATA_KEY` is the key the data file is sealed
with; when it is unset the key is derived from `JWT_SECRET` and the backend says so on start,
because rotating `JWT_SECRET` then makes the existing file unreadable. `ADMIN_USERNAMES` adds
accounts to the administrator list on top of the install owner, which is always one. Each
configured name is resolved to an account id when the backend starts, and admin checks use
that id, so a name that is later given up does not carry the rights to whoever claims it; the
account has to exist before its name is configured, and signup and rename refuse a configured
name. The seeded owner starts with the documented development password, so
a production start requires `STARTER_PASSWORD` (12+ characters, not the documented default) and
refuses to run otherwise; the account owns the starter games and cannot be deleted or restricted.

**Randomness.** Every credential the backend mints itself - the salt in a password hash, a
recovery code, a developer token, a game instance's launch token and an AES-GCM nonce - is drawn
from `BCryptGenRandom`. Not `std::rand`, and not a seeded engine: a generator whose state can be
recovered from its own output would give away the tokens it produced, and on a toolchain whose
`std::random_device` is not a device (libstdc++ on MinGW has no `/dev/urandom`) the whole seed
would be the clock. The call fails closed: an empty result is never used as a token.

**Game instances.** A game server talks to the site with a launch token, and that token is also on
its command line. It comes from the system generator and is compared in constant time, and the
command string shown in the console and the dashboard prints it as `***`. A process that can read
another process's command line already belongs to the same user (or an administrator), which is
more access than the token itself gives. The process id of an instance is only included in the
answer for an administrator, never in the server list a player of a public game can read.

**HTTP framing.** A request that repeats `Content-Length`, `Transfer-Encoding` or `Host`, or that
carries both a length and a chunked encoding, is refused and its connection is closed. Two values
for a header that decides framing is how one request comes to mean different things on either side
of a proxy, which is how a request gets smuggled.

**Answers and console output.** Every `/api/` answer carries `Cache-Control: no-store`, because a
login reply holds a token and a profile holds account data. A client report arrives without
authentication and is printed to the operator's console, so control characters in its fields are
replaced before it is stored or shown; a newline or an escape sequence would otherwise forge a log
line or repaint the terminal. A game's title and description are text too: control bytes are folded
into spaces before they are stored, so a title cannot repaint that console or forge a line in it, and
a title written by an older build is cleaned again on the line that prints it.

**Play URLs.** The URL handed to the client comes from `PUBLIC_BASE_URL` when that is set.
Otherwise the request's `Host` is used, but only when it is a plain `host:port`; anything else
falls back to the local default. A proxy that forwards a caller-supplied `Host` must not be able to
aim a player's client, and the session token it carries, at another origin.

The play answer names the server but never the player's session token: the command shown on the
page leaves `--token` out and there is no `rblxclone://` URL, because a token in a rendered string
travels into screenshots, shell history and whichever program registered the scheme. The client this
backend starts for a local player still receives the token on its own command line, the same
trade-off as a game instance's launch token, and a client started by hand signs in through the site.

**Game images.** A game's icon and banner are paths on this site (`/assets/...`), at most 512 bytes.
The fields are not a way to point the site at another host: a public game whose image lived
elsewhere would make every visitor's browser fetch it, which tells that game's owner who is reading
the site. Absolute URLs, protocol-relative URLs, `data:` payloads and backslashes are refused when a
game is created or edited, and a value an older build stored is answered as the built-in image.

The hardware id a client reports at `/api/verify` is an assertion about itself, not an
attestation: it is recorded so a ban can follow the machine, but a determined cheater can report a
different value. Hardware bans raise the cost of ban evasion rather than guaranteeing it.

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
per client address (`AUTH_RATE_LIMIT_MAX` 20 per `AUTH_RATE_LIMIT_WINDOW_MS` 15 minutes) and every
route shares a second budget (`RATE_LIMIT_MAX` 600 per minute). The client address is the socket
peer unless that peer is listed in `TRUSTED_PROXIES`; then the right-most `X-Forwarded-For` hop
that is not itself a trusted proxy is used, so a proxy in front of the site does not merge every
visitor into one budget and a client cannot choose its own key. At most 64 connections are served
at once; past that the socket is closed without a worker, because one thread per connection with
no ceiling lets slow clients exhaust the process.

`POST /api/games/:id/play` answers a remote player with the server, port and command to join, but
only starts the client process when the request came from loopback - the desktop window is for the
person at the machine, not for anyone who can reach the site. A game may run eight servers and the
install thirty-two. Signup, login and recovery
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

**The reverse proxy.** `RBLXProxy.exe` is optional and listens on `127.0.0.1:8080` unless told
otherwise. It passes the method, path, query, body and ordinary headers to WebApp, adds
`X-Forwarded-For`, `-Host` and `-Proto`, and drops hop-by-hop headers in both directions plus the
framing headers it writes itself, so no answer arrives with two `Content-Length` lines. Two
consequences follow. WebApp sees the proxy's socket address as the peer, so an install behind it
must list that address in `TRUSTED_PROXIES`; the site then budgets requests against the last
`X-Forwarded-For` hop it did not write itself. Without that, every visitor shares one budget. And
once `--listen` names a public or LAN address, the site answers whoever can reach that port over
plain HTTP, with no TLS in front.

**Release updates.** The updater has no TLS stack, so it authenticates the release instead of the
connection: `manifest.json` carries an RSA-2048 PKCS#1 v1.5 SHA-256 signature over a fixed
line-oriented description of the version, channel and every file path, hash and size. The public
key is pinned in `update.json`, which belongs to the machine and is excluded from releases, so a
network attacker cannot replace the key along with the manifest. `src/ReleaseSignature.cpp`
verifies the signature before any file is planned or downloaded and fails closed on a missing key,
a malformed signature or a digest mismatch; a copy with no key is restricted to a loopback release
URL. Signing is `RBLXUpdater sign-manifest --key <file>`, and the private exponent never enters the
package. The plain-HTTP channel still exposes which release is installed and when it is fetched.

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
Read marks are one row per account, room and channel, kept beside the messages; an unread count
is computed from the account's own mark and the room's messages, never from another account's
data.

**Repository browser.** `RBLXHub.exe` escapes repository content in one place
(`HubViews.cpp`) instead of at each call site, so a file name, a commit message or an issue
title that contains markup is shown as text; every link built from a path percent-encodes
what is not `[A-Za-z0-9-._~/]`. Its responses carry
`Content-Security-Policy: default-src 'none'; style-src 'self'; form-action 'self'`,
`X-Content-Type-Options: nosniff` and `Referrer-Policy: no-referrer`, and the pages contain
no scripts, so the policy cannot be loosened by a page that renders repository text. A form
post is capped at 64 KiB, an issue title at 200 characters and its body at 4000.

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
