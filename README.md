# Atmospheric-Starter

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

## Android

The root `CMakeLists.txt` requires CMake 3.25+, newer than what the Android
SDK Manager offers as an installable `cmake;X.Y.Z` package (it tops out at
3.22.1). `platforms/android/app/build.gradle.kts` doesn't pin a `version` for
its `externalNativeBuild.cmake` block, so you need to point Gradle at a CMake
3.25+ binary yourself via `platforms/android/local.properties`:

```properties
cmake.dir=/path/to/a/cmake-3.25-or-newer/install/prefix
```

(the directory containing `bin/cmake`, not the binary itself). CI computes
this automatically from whatever `cmake` is on `PATH` — see `ci-android.yml`.