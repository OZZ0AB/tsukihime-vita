# Build guide

The release was built with the VitaSDK 2026.08 channel, GCC 15.2.0, and CMake 3.31.6 on Linux. Building requires VitaSDK, CMake 3.16 or later, and a C/C++ build tool such as Make.

## Dependencies

Follow the official [VitaSDK installation instructions](https://vitasdk.org/), then set `VITASDK` to the SDK directory and add its `bin` directory to `PATH`.

With the current VitaSDK package manager, install the dependencies:

```sh
vdpm install sdl2 sdl2_image sdl2_mixer sdl2_ttf libvita2d libvita2d_ext \
  luajit taihen bzip2 zlib libpng libjpeg-turbo libwebp libogg libvorbis \
  flac mpg123 libmikmod libmodplug
```

The package manager resolves additional dependencies such as FreeType, HarfBuzz, Opus and XMP. The build uses static archives and VitaSDK's native GXM rendering through SDL2.

## Build the VPK

Run these commands from the repository root:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target vpk -j2
```

Outputs:

- `build/TsukihimeVita.vpk`
- `build/TsukihimeVita-01.04.vpk`

The VPK contains the executable and LiveArea resources. It does not include the game script, archives, font, or music. The executable loads those from `ux0:data/tsukihime/game/` and stores saves in `ux0:data/tsukihime/save/`.

The SELF uses VitaSDK's standard homebrew identity for access to the external data directory. The application title ID is `TSUKI0001`.

## Artwork

The checked-in artwork is already prepared as indexed PNG images:

| File | Dimensions |
| --- | --- |
| `assets/livearea/icon0.png` | 128 x 128 |
| `assets/livearea/bg.png` | 840 x 500 |
| `assets/livearea/startup.png` | 280 x 158 |

To prepare different source images, install Pillow in your Python environment and run:

```sh
python3 scripts/prepare_livearea.py --icon icon.png --background background.jpg
```

The script preserves image proportions and uses centered crops for the background and launch panel.

## Local checks

The save-support check runs on Linux and requires a host C++ compiler:

```sh
mkdir -p build
g++ -std=c++11 -Wall -Wextra -fsanitize=address,undefined \
  -fno-omit-frame-pointer -Iengine/src/common \
  diagnostics/verify_save_support.cpp engine/src/common/save_support.cpp \
  -o build/verify_save_support
ASAN_OPTIONS=detect_leaks=0 build/verify_save_support
```

It checks ASCII save labels, invalid-clock fallback, save payload readback, and simulated write/close failures.

The game reader check also requires the host bzip2 development library and your own game data:

```sh
g++ -std=c++11 -DLINUX -DENABLE_1BYTE_CHAR -Iengine/src/onsjh \
  diagnostics/verify_game_data.cpp engine/src/onsjh/DirectReader.cpp \
  engine/src/onsjh/SarReader.cpp engine/src/onsjh/NsaReader.cpp \
  engine/src/onsjh/ScriptHandler.cpp engine/src/onsjh/coding2utf16.cpp \
  engine/src/onsjh/sjis2utf16.cpp engine/src/onsjh/resize_image.cpp \
  -lbz2 -o build/verify_game_data
build/verify_game_data /path/to/Game
```

It opens the original script and checks startup resources in the combined SAR and NSA archives. These host checks complement testing a save, app restart, and load on the Vita.
