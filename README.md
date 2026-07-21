# XBolo

XBolo is a clone of the original tank warfare game Bolo written by Stuart
Cheshire.  It is an action/strategy game played over the internet where 2 or
more players battle for control of pillboxes and bases.

XBolo was developed without the original source code, although the look and
sound effects were captured from the original game.  All the graphics and
sounds are captured from the original Bolo © 1993 Stuart Cheshire.  The
source code is distributed under the MIT License (see LICENSE).

## Building

Requires Xcode on macOS 11 or later (builds natively on Apple Silicon).

```sh
xcodebuild -target "Mac OS X"      # the game, build/<config>/XBolo.app
xcodebuild -target "Dedicated Host" # headless game server
xcodebuild -target "XBolo Tracker"  # meta-server for game discovery
```

`Dedicated Host/build.sh` also builds the headless server with plain gcc/cc,
no Xcode required.

## Source layout

The engine is portable C; the Mac app is Objective-C (ARC) on top of it.

| Area | Files |
|------|-------|
| Client engine core | `client.c` (lifecycle, threads, sockets, join) |
| Client protocol | `client_recv.c`, `client_send.c` |
| Client game simulation | `client_logic.c` |
| Server engine core | `server.c` |
| Server protocol | `server_recv.c`, `server_send.c` |
| Server world logic | `server_logic.c` |
| Map file format | `bmap.c`, `bmap_client.c`, `bmap_server.c` |
| Utilities | `buf.c`, `io.c`, `list.c`, `rect.c`, `vector.c`, `timing.c`, `resolver.c`, `errchk.c`, `tracker.c` |
| Mac app | `Mac OS X/` (controller, game view, key config), `GSRobot.m` (robot plugin host) |

`client_internal.h` / `server_internal.h` declare functions shared between
their module's translation units; `client.h` / `server.h` are the public
APIs.

## Error handling conventions

The engine records an error trace (file/function/line) when a call fails;
`errchk.h` provides the machinery.  Two styles exist:

- **Preferred (converted code):** plain control flow with `ERRLOG(err)`,
  which records the trace, sets `errno`, and evaluates to -1:
  `if (write(...) == -1) return ERRLOG(errno);`.  See `buf.c` for the
  template.
- **Legacy:** `TRY` / `LOGFAIL` / `CLEANUP` / `END` goto-based macros, still
  used by most of the engine.  Convert functions module by module (not
  mid-function), and playtest after converting a module — the protocol and
  simulation code has no test suite.

## UI

The entire interface is built in code (`Mac OS X/GSXBoloController+CodeUI.m`),
constructed at applicationWillFinishLaunching:.  There is no nib.  The
original IB3 nib's structure is preserved in `nib-ui-spec.json`, captured
with the GSUIDump tool (`XBolo -GSDumpUI YES [-GSDumpUIPath <file>]`), which
dumps the live window/view/outlet/menu tree as JSON for regression-diffing
UI changes.

## Quick Look

The Map Thumbnails target builds a Quick Look thumbnail extension
(embedded in XBolo.app) that renders .map files as the classic
mini-map in Finder and Quick Look.  It replaces the retired
.qlgenerator plug-in; the rendering is a port of the original
generator.

## Known legacy items


## Community

You can still find a few players on IRC (EFnet) idling in #xbolo.
