cmake_minimum_required(VERSION 3.25)

# ── vcpkg bootstrap ────────────────────────────────────────────
# All vcpkg wiring (toolchain file, overlay triplets, Emscripten chainload,
# manifest resolution) is encapsulated in the engine; must precede project().
# See README.md for how to swap this out for a different package manager.
include("${CMAKE_CURRENT_LIST_DIR}/Atmospheric/cmake/InstallDependencies.cmake")

project(GradientQuad VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# ── Atmospheric engine (submodule) ───────────────────────────────────
set(AE_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(AE_USE_FFMPEG     OFF CACHE BOOL "" FORCE)
add_subdirectory(Atmospheric)

# ── Dependencies ─────────────────────────────────────────────────
find_package(glm    CONFIG REQUIRED)
find_package(fmt    CONFIG REQUIRED)
find_package(Bullet CONFIG REQUIRED)

# ── App target ───────────────────────────────────────────────────
if(ANDROID)
    set(APP_TARGET main)
    add_library(${APP_TARGET} SHARED src/main.cpp)
elseif(IOS)
    set(APP_TARGET main)
    add_executable(${APP_TARGET} MACOSX_BUNDLE src/main.cpp)
else()
    set(APP_TARGET GradientQuad)
    add_executable(${APP_TARGET} src/main.cpp)
    set_target_properties(${APP_TARGET} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/GradientQuad
    )
endif()

target_include_directories(${APP_TARGET} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Atmospheric/Engine/include
)

target_link_libraries(${APP_TARGET} PRIVATE
    AtmosphericEngine
    glm::glm
    fmt::fmt
    BulletSoftBody BulletDynamics BulletCollision LinearMath
)

# ── Platform-specific link / packaging ───────────────────────────────────
if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")

    set(EM_ASSETS ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/GradientQuad/assets)

    add_custom_target(stage_gq_preload
        COMMAND ${CMAKE_COMMAND} -E make_directory   ${EM_ASSETS}
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/Atmospheric/Engine/default_assets/shaders
            ${EM_ASSETS}/shaders
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/Atmospheric/Engine/default_assets/fonts
            ${EM_ASSETS}/fonts
        COMMENT "Staging Emscripten preload assets for GradientQuad"
    )
    add_dependencies(${APP_TARGET} stage_gq_preload)

    target_link_options(${APP_TARGET} PRIVATE
        -sALLOW_MEMORY_GROWTH=1
        -sASSERTIONS=1
        -sDISABLE_EXCEPTION_CATCHING=0
        "SHELL:--preload-file ${EM_ASSETS}/shaders@assets/shaders"
        "SHELL:--preload-file ${EM_ASSETS}/fonts@assets/fonts"
        "SHELL:--shell-file ${CMAKE_CURRENT_SOURCE_DIR}/shell.html"
    )
elseif(NOT IOS)
    find_package(spdlog CONFIG REQUIRED)
    find_package(glad   CONFIG REQUIRED)
    target_link_libraries(${APP_TARGET} PRIVATE
        glad::glad
        spdlog::spdlog
        SDL3::SDL3
        raudio
    )
endif()

ae_copy_engine_assets(${APP_TARGET})
