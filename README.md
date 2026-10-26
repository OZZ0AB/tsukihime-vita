# Tsukihime Vita

A native PS Vita homebrew port for the original Tsukihime English release. It runs ONScripter game data directly, using a VitaSDK build of ONScripter-JH.

![Tsukihime LiveArea artwork](assets/livearea/bg.png)

## Download

Download `TsukihimeVita-01.04.vpk` from this repository's **Releases** section. Version 1.04 includes the working save fix and the custom bubble and LiveArea artwork.

The game script, archives, font, translation, and music are not included. Supply the data from your own compatible copy of the game.

## Requirements

- A PS Vita capable of running homebrew, with VitaShell available.
- The original Tsukihime game data prepared for the mirror moon English patch, version 1.2.
- Sufficient free storage for the game files and saves.

This port was tested on the project owner's Vita. Gameplay and saving were confirmed working; every route and scene has not been exhaustively tested. The remake and unrelated NScripter games are outside this release's tested scope.

## Installation

1. Transfer `TsukihimeVita-01.04.vpk` to the Vita and install it with VitaShell.
2. Create `ux0:data/tsukihime/game/`.
3. Copy the contents of your game's `Game` folder into that directory, using the layout below.
4. Launch the **Tsukihime** bubble.

```text
ux0:data/tsukihime/game/
  nscript.dat
  arc.nsa
  arc.sar
  arc1.nsa
  default.ttf
  CD/
    track01.ogg
    ...
    track10.ogg
```

The music tracks are optional for launching the game, but are required for its background music. Copy the files directly into `game/`; avoid an extra nested `Game/` folder. The Windows executable and DLL are unnecessary.

To upgrade, install the new VPK over the existing bubble. The title ID remains `TSUKI0001`; save files reside in the data directory.

## Controls

| Control | Action |
| --- | --- |
| Circle | Advance text or confirm a selection |
| Cross, held | Fast-forward |
| Triangle | Open the in-game menu or leave backlog |
| Square | Enable auto mode |
| L | Toggle instant page display |
| R | Enable skip |
| Select | Cycle text speed |
| Start | Advance text |
| D-pad or left stick | Navigate selections and backlog |
| Front touchscreen | Select or advance |

These are the engine's default Vita mappings. To save, open the in-game menu, select **Create a save game (Save)**, select a slot, then choose **Yes**.

## Saves and settings

Save slots are stored at `ux0:data/tsukihime/save/save1.dat` through `save20.dat`. Back up the whole `ux0:data/tsukihime/` directory before moving installations between memory cards.

The save and load menus use English text and ASCII digits compatible with the supplied English font. If a save timestamp and the system clock are both invalid, the slot displays January 1, 2000 at 00:00. This affects the displayed timestamp only; the system clock is not changed.

## Troubleshooting

The port checks the required game files at startup. When reporting a problem, include the release version, Vita model, steps to reproduce, and the relevant log:

```text
ux0:data/tsukihime/startup.log
```

If that path cannot be opened, the fallback is `ux0:data/tsukihime-startup.log`. Logging is optional and does not prevent the game from launching.

- **A required file is missing:** check the exact directory layout and include `default.ttf` and all three archives.
- **No background music:** check the `CD/track01.ogg` through `track10.ogg` filenames.
- **Saving fails:** check free storage and read the save error in the log. A failed write displays an error instead of silently succeeding.
- **The bubble closes:** include the last log lines and any on-screen error code when opening an issue.

Do not attach game archives, scripts, translation files, or music to issues.

## Building from source

See [the build guide](docs/BUILDING.md) for the VitaSDK dependencies and build commands. The repository includes the modified runtime source needed for this port. A matching source archive is included with the binary release.

## Credits and licensing

- TYPE-MOON: the original Tsukihime game.
- mirror moon: the English translation.
- Ogapee, jh10001, wetor, and YuriSizuku: the ONScripter runtime and Vita port lineage.
- [VitaSDK](https://vitasdk.org/) and the SDL contributors: the native toolchain and multimedia libraries.

The runtime base is [YuriSizuku/psv-OnscripterJH](https://github.com/YuriSizuku/psv-OnscripterJH), pinned to commit `01955d7c1a4ecdff4245aa6e3fc08fd64625e4a9`.

Engine source files retain their original license notices. See [LICENSE](LICENSE) and [third-party notices](THIRD_PARTY.md). Game data, the English translation, music, and LiveArea artwork are not relicensed under the engine's license. This is an unofficial fan port and is not endorsed by TYPE-MOON or mirror moon.
