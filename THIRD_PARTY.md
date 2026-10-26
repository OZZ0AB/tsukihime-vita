# Third-party notices

## Runtime source

The engine is based on [YuriSizuku/psv-OnscripterJH](https://github.com/YuriSizuku/psv-OnscripterJH), commit `01955d7c1a4ecdff4245aa6e3fc08fd64625e4a9`. That project builds on ONScripter by Ogapee, ONScripter-JH by jh10001, and the Vita work by wetor and other contributors.

The source under `engine/src/onsjh/` preserves the upstream file-level copyright and license notices. Most runtime files specify GNU GPL version 2 or later. The GPL version 2 text is included as `LICENSE` and `engine/license/COPYING`.

The original upstream `engine/license/README.jh` and `engine/license/license.txt` are retained. The latter contains a Creative Commons Attribution-NonCommercial-ShareAlike 4.0 license text; its presence does not replace the license notices attached to individual engine source files. Other bundled source, such as `stb_image.h`, retains its own notices.

## Changes for Tsukihime Vita

The modified runtime includes:

- A dedicated game root and save directory.
- Combined SAR and NSA archive loading for the English patch.
- Single-byte English string support and readable save/load menu prompts.
- Checked save writes and a fallback display timestamp when clock data is invalid.
- Startup diagnostics, file checks, and persistent logging.
- Current VitaSDK compatibility and static dependency linking.

The additions in `engine/src/common/save_support.*` and `vita_startup.*` carry GPL-2.0-or-later notices. The root build configuration, documentation, and helper scripts accompany these changes. The source release contains the modified source used for the binary release.

## Build dependencies

VitaSDK and the linked SDL, image, audio, font, and Lua libraries are separate upstream projects with their own licenses. Install them through VitaSDK as described in `docs/BUILDING.md`. The SDK's debug-screen sample is compiled from the installed SDK; the [VitaSDK samples](https://github.com/vitasdk/samples) are provided under CC0.

## Game and artwork

Tsukihime is a TYPE-MOON work. The English translation is credited to mirror moon. The game files and translation are supplied by the player and are not included in this repository or release.

LiveArea images were supplied for this project and resized for Vita packaging. Rights remain with their original creators; they are not covered by the engine's GPL license. No affiliation or endorsement by the original creators is implied.
