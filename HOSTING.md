# Hosting your own servers

`RBLXHosts` is this project's own answer to a hosting platform: it keeps a fixed
number of game servers alive on one machine, each on its own port with its own log,
and restarts one that exits.

That is the job the game actually needs. It is deliberately not a deploy platform:
no container build and no multi-machine control. The servers it starts are the
`Server` this repository builds, run as ordinary processes, so the whole thing is
one binary with no runtime to install, and the control panel is a page that binary
serves.

```powershell
RBLXHosts.exe init --config hosting.conf   # write a commented default file
RBLXHosts.exe up --config hosting.conf     # keep the servers running until Ctrl+C
```

With no config file the defaults are two servers on 7777 and 7778, running
`Server.exe` from the current directory, writing `logs/host-<port>.log`.

## What it does

- **One server per slot.** Slot `n` listens on `port + n`, so the ports never
  collide; each slot gets `host-<port>.log` in `log-directory`.
- **Restart with a growing delay.** A server that exits is started again after
  `restart-delay`, then double that, up to 32x: a crash loop does not spin the CPU,
  and a server that ran for an hour comes back at once.
- **Give up instead of looping.** More than `max-restarts` failures inside
  `restart-window` milliseconds and that slot is marked `failed` and left alone. The
  table keeps showing it, with the last error, so the reason is not lost in a
  scrolling log. When every slot has failed the program exits non-zero, which is
  what a service wrapper should see.
- **A control panel.** `up` also serves a page on loopback: every slot, its log, and
  start, stop or restart for one server or the whole pool. See
  [The control panel](#the-control-panel).
- **Stopping is orderly.** Ctrl+C reaches the supervisor's console control handler, which
  terminates each server and waits for it to go; closing the window does the same.

A live table is printed every five seconds:

```
instance port   pid      state    restarts  uptime   log
0        7777   41233    running  0         00:01:05 logs/host-7777.log
1        7778   41234    running  1         00:00:12 logs/host-7778.log
2 of 2 running
```

## The config file

`key = value` per line, `#` or `;` for comments, unknown keys ignored so a file
written for a later version still loads. Numbers are validated: a typo is an error
that names the key, not a silent zero.

| key | default | meaning |
| --- | --- | --- |
| `server` | `Server.exe` | the server executable, relative to `directory` or absolute |
| `directory` | `.` | working directory for the servers, so world paths stay relative |
| `world` | *(server default)* | world file each server loads (`--world`) |
| `web` | *(server default)* | website the servers report to (`--web`) |
| `log-directory` | `logs` | where `host-<port>.log` is written |
| `instances` | `2` | how many servers, 1 to 64 |
| `port` | `7777` | the first port; the rest count up |
| `max-restarts` | `5` | failures allowed inside the window before a slot is failed |
| `restart-window` | `60000` | the window, in milliseconds |
| `restart-delay` | `1000` | first restart delay, in milliseconds |
| `dashboard-host` | `127.0.0.1` | address the control panel binds; keep it on loopback |
| `dashboard-port` | `7900` | the control panel's port; `0` turns the panel off |

Every option can also be given on the command line, where it wins over the file:
`--instances`, `--port`, `--dir`, `--world`, `--web`, `--server`, `--dashboard-host`,
`--dashboard-port`.

## The control panel

`up` serves a dashboard, so the pool can be watched and driven without reading the
console table. It prints the URL to open, with a token that is made up for the run:

```
control panel: http://127.0.0.1:7900/?token=4f9c0b1d6a2e8c73
```

Opening that link exchanges the token for an `HttpOnly` cookie and redirects, so the
token is not left in the address bar, the history or a bookmark. Requests without it
get `401`. `dashboard-port = 0` turns the panel off and leaves the console as the only
view. The panel listens on `127.0.0.1` by default and should stay there: it starts and
stops processes, so exposing it is exposing the machine.

The page shows, per slot, the port, pid, state, restart count, uptime, the last error
it hit and the tail of its log. The actions are the ones an operator needs:

| action | what it does |
| --- | --- |
| Start | clears that slot's failure history and lets the next tick launch it |
| Stop | terminates that server and leaves the slot down |
| Restart | stops it and starts it again, without counting a failure |
| Start all / Stop all | the same for the whole pool |

A stopped slot stays stopped: `tick` skips it the same way it skips a slot that failed,
so a server you stopped is not brought back behind your back. A manual start also
clears the failure budget, which is why one press is enough after a slot has given up.

The page polls `GET /api/status`; the buttons `POST /api/instances/<slot>/start|stop|restart`
and `POST /api/pool/start|stop`; `GET /api/logs/<slot>?tail=200` returns a log as plain
text. Those four are enough for a script to drive the pool, and a command-line client
can send `X-RBLX-Token: <token>` instead of doing the cookie exchange.

## Playing on a hosted server

The client launches through the website, which starts its own server per Play, so a
pool like this is for servers that must stay up: a lobby, a world people drop into,
or a machine other than the one running `WebApp`. Point the client at one with the
address the server prints when it binds its port.

## What it deliberately does not do

- **No containers.** A container would add a runtime the game does not need; the
  servers are native processes, and the platform builds them.
- **No login beyond the token.** The panel has no accounts: the token printed at
  startup is the whole of its authentication, which is the right size for a tool that
  only listens on loopback.
- **No multi-machine control.** One `RBLXHosts` supervises one machine. Running it
  on two is two pools, which is all a prototype needs.
