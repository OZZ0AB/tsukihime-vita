# Runtime provenance

Upstream: https://github.com/YuriSizuku/psv-OnscripterJH

Base commit: `01955d7c1a4ecdff4245aa6e3fc08fd64625e4a9`

The source release vendors the runtime sources and retains their license notices. The generic upstream game-browser frontend, screenshots, Windows tools, and unrelated upstream art are not required by the dedicated Tsukihime build.

Build from the top-level `CMakeLists.txt`. It selects `engine/src/onsjh/` and the common support sources, and packages the images in `assets/livearea/`.
