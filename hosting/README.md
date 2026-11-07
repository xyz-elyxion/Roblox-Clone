# Hosting

The server-hosting platform the game's servers are meant to be deployed by, kept
apart from the game itself.

## `coolify/`

[Coolify](https://github.com/coollabsio/coolify) source, vendored whole: upstream
`main` at `29aa42b982a3c469c713d3b4a3ebb2092127ec09`, cloned on 2026-11-07 with
`git clone --depth 1` and its `.git` removed. No file in it was edited, so it can
be replaced wholesale by a newer clone rather than merged into.

- **Licence: AGPL-3.0** (`coolify/LICENSE`), which is not this project's licence.
  The vendored tree keeps its own licence, notices and `.gitignore`; the project's
  terms do not extend over it, and anything published from a modified copy has to
  respect the AGPL's source-availability terms.
- **It is not part of the game build.** `RBLXBuildLists.txt`, `RBLXBuildPresets.json`
  and `scripts/package-build.ps1` name the files they build and copy, and none of
  them reaches into `hosting/`, so no Coolify file is compiled or shipped in a
  release. Its 5,485 files are here as a checkout to run, not as a dependency.
- **It runs the way Coolify runs.** `hosting/coolify` is a Laravel application
  fronted by Docker: see its `README.md` and `docker-compose.yml`, and start it
  with `docker compose` on a machine that has Docker. That is separate from the
  game server, which this repository builds and which stays a native process.
- The `.env.*` templates it ships are read by its compose files. They contain no
  secrets, and the root `.gitignore` puts them back for this path only.

Nothing in the C++ tree talks to it yet: the game servers in the console are child
processes started on the same machine. Driving Coolify instead (creating an
application per game server, starting and stopping it, reading its logs) needs its
HTTP API and a token, and is separate work from carrying the source here.

## `RBLXHosting.exe` — the native panel

`RBLXHosting` is that separate work, at the size that fits a desktop: a
**Windows-only** C++ program that talks to a Coolify instance's API instead of
running one. It lists the applications the token can see, shows whether each is
up, and offers Start, Stop, Restart, Logs (the last 200 lines of the container)
and a button that opens the real dashboard in a browser.

```powershell
build\windows-release\Release\RBLXHosting.exe
```

Paste the address (`http://192.168.1.10:8000`), then a token from Coolify's
**Keys & Tokens → API tokens**. The token needs `read` for the list, `deploy` for
the three buttons and `read:sensitive` for logs; a token without the last one
still lists and starts, and only the Logs button is refused, with Coolify's own
reason. The token is used from memory and never written to disk, and it is not
logged.

Two limits are real, not bugs. **There is no TLS in this project** (see
`include/HttpClient.h`), so `https://` is refused with an explanation instead of
failing like a wrong token: use Coolify's plain-http port on a trusted network, or
put a local proxy that speaks http in front of it. And the panel only drives what
already exists — creating an application is a dozen decisions (source, build pack,
port, domain) that belong in the dashboard the button opens.

On any other platform the program compiles but exits with a message saying so; it
is built on every CI platform exactly to keep that code compiling.
