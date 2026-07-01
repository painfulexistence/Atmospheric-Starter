# Atmospheric-Starter

A cross-platform starter app (desktop, Android, iOS, WebAssembly) built on
the [Atmospheric](https://github.com/painfulexistence/Atmospheric) engine.

## Build (Desktop)

Prerequisites: CMake 3.22+, a C++20 compiler, [Ninja](https://ninja-build.org/).

```sh
git clone --recurse-submodules https://github.com/painfulexistence/Atmospheric-Starter.git
cd Atmospheric-Starter

cmake --preset desktop
cmake --build --preset debug   # or --preset release
```

(Already cloned without `--recurse-submodules`? Run
`git submodule update --init --recursive` — `Atmospheric/` itself has a
nested `vcpkg` submodule.)

The first configure bootstraps vcpkg and builds all dependencies from
source, so it takes a while; subsequent builds are fast. The binary lands
under `build/<Debug|Release>/GradientQuad/`.

## Build (Android / iOS / WebAssembly)

- **Android**: open `platforms/android/` in Android Studio, or run
  `./gradlew assembleDebug` from that directory.
- **iOS**: see the usage comment at the top of `platforms/ios/CMakeLists.txt`.
- **WebAssembly**: `cmake --preset wasm && cmake --build --preset wasm`
  (requires an activated [Emscripten](https://emscripten.org/) SDK — see
  `EMSDK_VERSION` in `.github/workflows/ci-web.yml` for the version this
  repo is tested against).

## Dependencies

By default this project resolves its dependencies through the vcpkg
submodule bundled inside `Atmospheric/`, via
`Atmospheric/cmake/InstallDependencies.cmake` (included at the top of
`CMakeLists.txt` and `platforms/ios/CMakeLists.txt`, before `project()`).
There are three ways to work with this, depending on what your game needs:

1. **Use exactly what the engine needs, nothing more.** Do nothing — the
   default `VCPKG_MANIFEST_DIR` points at `Atmospheric/vcpkg.json`, so this
   starter builds without a `vcpkg.json` of its own.

2. **Add extra vcpkg dependencies.** vcpkg only resolves one manifest per
   configure, so you can't merge two `vcpkg.json` files. Create your own
   `vcpkg.json` at the repo root (start by copying `Atmospheric/vcpkg.json`
   and adding to it), then point `VCPKG_MANIFEST_DIR` at it *before* the
   `include(.../InstallDependencies.cmake)` line:
   ```cmake
   set(VCPKG_MANIFEST_DIR "${CMAKE_CURRENT_LIST_DIR}" CACHE STRING "")
   include("${CMAKE_CURRENT_LIST_DIR}/Atmospheric/cmake/InstallDependencies.cmake")
   ```

3. **Use a different package manager entirely** (Conan, system packages,
   etc.). `Engine/CMakeLists.txt` only calls plain `find_package(... CONFIG
   REQUIRED)` — it has no hard dependency on vcpkg. Delete the
   `include(.../InstallDependencies.cmake)` line and set up your own
   toolchain file / `CMAKE_PREFIX_PATH` instead, as long as it produces the
   same imported targets (`glad::glad`, `glm::glm`, `Bullet::*`,
   `RmlUi::RmlUi`, `box2d::box2d`, `fmt::fmt`, `spdlog::spdlog`,
   `flatbuffers::flatbuffers`).