# open.mp anti-cheat

a native open.mp component (c++) that detects client modifications by asking the client to read its own memory and comparing the result against known cheat signatures

it detects s0beit, sobfox, cleo 4 / cleo 5, moonloader, silentpatch, sampfuncs, modified vorbisfile.dll, ultrawh, improved deagle, stealthremastered, sensfix, silentaim, fakemobile, [faker5-style clientcheck spoofers](#countering-faker5), and disallowed client versions.

## how it works

each detection lives in its own module under `src/detection/` (memory, version, poison, mobile, raknet, faker5). the component receives player/network/tick events, fans them to the modules, and each module reports hits through one path that applies the configured action. the mobile and raknet modules read raw packets and are off by default.

## countering faker5

[faker5](https://github.com/WaterinoS/FakeR5) is a clientcheck spoofer: it hooks the incoming memory-query rpc and answers the `0x5` (gta_sa.exe) and `0x45` (samp.dll) reads out of a clean copy of those files on disk instead of live process memory, so any in-memory cheat becomes invisible to a memory-signature scanner - and it forces the join version to `0.3.7-R5`. every check the other modules send sits inside the window faker5 intercepts.

the `faker5` module reads the pe-header of samp.dll and gta_sa.exe. faker5 resolves those addresses through its on-disk file mapping, where a header rva maps to file-offset 0, and answers `0`; a real client reads the same address from the live, mapped module image and answers with the actual (non-zero) header bytes. the module also sends an in-section canary so it only ever acts on a client that is genuinely answering checks. it runs only for classic (non-omp, non-mobile) clients, which is the only place faker5 can load.

detecting the spoofer is enough to act on it via `action.faker5`; you don't need to name the underlying cheat it was hiding.

## building

needs cmake 3.19+ and a 32-bit c/c++ toolchain. open.mp is 32-bit, so a 64-bit build will not load.

```
./bootstrap.sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

output is `anticheat.so` (linux) or `anticheat.dll` (windows). on fedora the 32-bit libs are `glibc-devel.i686 libstdc++-devel.i686 libgcc.i686`. on cmake 4.x add `-DCMAKE_POLICY_VERSION_MINIMUM=3.5`. to build without the pawn api pass `-DAC_ENABLE_PAWN=OFF`.

## install with sampctl

with [sampctl](https://github.com/Southclaws/sampctl), add it to your server's `pawn.json`:

```json
"dependencies": [
  "ricardoofnl/open.mp-anticheat"
]
```

or run:

```
sampctl install ricardoofnl/open.mp-anticheat
```

sampctl fetches the latest release and extracts `anticheat.so` / `anticheat.dll` and `anticheat.cfg` into `components/`, and `anticheat.inc` onto the compiler include path. `sampctl ensure` / `sampctl run` then picks it up.

## install manually

copy `anticheat.so` (linux) or `anticheat.dll` (windows) plus `anticheat.cfg` into your server's `components/` folder, and put `pawn/anticheat.inc` on your compiler include path. then `#include <anticheat>` in your gamemode.

## config

`components/anticheat.cfg` uses `key = value` with `#` comments. set `log_only = 1` first on a live server to watch for false positives before enforcing. toggle detectors with `module.memory|version|poison|mobile|raknet|faker5`, set the version whitelist with `allowed_versions`, and set per-cheat responses with `action.<cheat> = ignore|warn|kick|ban`.

## pawn api

```
forward OnPlayerCheatDetected(playerid, cheatid, action); // return 0 to suppress the built-in action

native AC_SetEnabled(bool:toggle);
native bool:AC_IsEnabled();
native bool:AC_AddException(playerid, cheatid);
native bool:AC_RemoveException(playerid, cheatid);
native bool:AC_IsPlayerMobile(playerid);
native AC_GetLastCheat(playerid);
native bool:AC_GetCheatName(cheatid, name[], len = sizeof(name));
```

## notes

detection by memory signature is heuristic and can false-positive across game builds, so use `log_only`, per-cheat actions and exceptions to tune it. silentpatch and modified vorbisfile default to `warn` because they are common and often benign. this is a defensive tool for a server operator's own server.
