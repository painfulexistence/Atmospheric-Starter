# iOS build — invoke from this directory:
#
#   cmake -G Xcode \
#         -B build \
#         -S platforms/ios \
#         -DCMAKE_SYSTEM_NAME=iOS \
#         -DCMAKE_OSX_DEPLOYMENT_TARGET=16.0 \
#         -DVCPKG_TARGET_TRIPLET=arm64-ios
#
cmake_minimum_required(VERSION 3.21)

set(ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../..")

# Both arm64-ios and arm64-ios-simulator vcpkg triplets only build arm64 slices.
# Without this, Xcode defaults to its standard simulator archs (which includes
# x86_64 on the Simulator SDK), and linking fails with missing symbols for the
# x86_64 slice since vcpkg never built Bullet/RmlUi/box2d/fmt for it.
set(CMAKE_OSX_ARCHITECTURES "arm64" CACHE STRING "" FORCE)

# All vcpkg wiring (toolchain file, overlay triplets, manifest resolution) is
# encapsulated in the engine; must precede project(). See README.md for how
# to swap this out for a different package manager.
include("${ROOT_DIR}/Atmospheric/cmake/InstallDependencies.cmake")

project(GradientQuad-iOS LANGUAGES C CXX OBJC OBJCXX Swift)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_SWIFT_LANGUAGE_VERSION 6.0)

include(FetchContent)

set(AE_ROOT_DIR   "${ROOT_DIR}/Atmospheric" CACHE PATH "" FORCE)
set(AE_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(AE_USE_FFMPEG     OFF CACHE BOOL "" FORCE)

# This adds Atmospheric/Engine directly, bypassing Atmospheric's own top-level
# CMakeLists.txt. Engine/CMakeLists.txt adds its own cmake/helpers to
# CMAKE_MODULE_PATH before include(AssetPipeline), so this works regardless.
add_subdirectory("${ROOT_DIR}/Atmospheric/Engine" "${CMAKE_BINARY_DIR}/Engine")

find_package(glm    CONFIG REQUIRED)
find_package(fmt    CONFIG REQUIRED)
find_package(Bullet CONFIG REQUIRED)

add_executable(main MACOSX_BUNDLE "${ROOT_DIR}/src/main.cpp")

target_include_directories(main PRIVATE
    "${ROOT_DIR}/Atmospheric/Engine/include"
)

target_link_libraries(main PRIVATE
    AtmosphericEngine
    glm::glm
    fmt::fmt
    BulletSoftBody BulletDynamics BulletCollision LinearMath
)

# ── Xcode / bundle settings ──────────────────────────────────────────────────────
set_target_properties(main PROPERTIES
    MACOSX_BUNDLE TRUE
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist"
    XCODE_ATTRIBUTE_SWIFT_VERSION "6.0"
    XCODE_ATTRIBUTE_SWIFT_OBJC_INTEROP_MODE "objcxx"
    XCODE_ATTRIBUTE_SWIFT_OBJC_BRIDGING_HEADER
        "${CMAKE_CURRENT_SOURCE_DIR}/Sources/AtmosEngine-Bridging-Header.h"
    XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2"
    XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "16.0"
    XCODE_ATTRIBUTE_GCC_PREPROCESSOR_DEFINITIONS "IMGUI_IMPL_OPENGL_ES3=1"
    XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "com.atmospheric.gradientquad"
)

# Code signing: simulator never needs it; device requires a team + automatic profile.
if(CMAKE_OSX_SYSROOT STREQUAL "iphonesimulator")
    set_target_properties(main PROPERTIES
        XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED  "NO"
        XCODE_ATTRIBUTE_CODE_SIGNING_REQUIRED "NO"
        XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY    ""
    )
else()
    # Device build: pass -DCMAKE_XCODE_ATTRIBUTE_DEVELOPMENT_TEAM=<TeamID> at configure time.
    set_target_properties(main PROPERTIES
        XCODE_ATTRIBUTE_CODE_SIGN_STYLE    "Automatic"
        XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "Apple Development"
    )
endif()

target_sources(main PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/Sources/ios_init.mm"
    "${CMAKE_CURRENT_SOURCE_DIR}/Sources/AtmosAppDelegate.swift"
)

target_include_directories(main PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/Sources"
    $<TARGET_PROPERTY:SDL3::SDL3,INTERFACE_INCLUDE_DIRECTORIES>
    "${CMAKE_BINARY_DIR}/_deps/sdl3-src/src/video/uikit"
)

ae_copy_engine_assets(main)
ae_copy_game_assets(main "${ROOT_DIR}/assets")
