import java.io.BufferedInputStream
import java.io.FileOutputStream
import java.net.URI
import java.util.zip.ZipInputStream

plugins {
    id("com.android.application")
    id("org.jetbrains.kotlin.android")
}

val downloadSDLJavaSources = tasks.register("downloadSDLJavaSources") {
    val outputDir = file("src/main/java")
    val sdlVersion = "release-3.2.12"
    val zipUrl = "https://github.com/libsdl-org/SDL/archive/refs/tags/$sdlVersion.zip"

    outputs.dir(outputDir.resolve("org/libsdl/app"))

    doLast {
        val targetDir = outputDir.resolve("org/libsdl/app")
        if (targetDir.exists() && targetDir.list()?.isNotEmpty() == true) {
            return@doLast
        }

        println("Downloading SDL3 Java sources from GitHub ($zipUrl)...")
        targetDir.mkdirs()

        URI(zipUrl).toURL().openStream().use { inputStream ->
            ZipInputStream(BufferedInputStream(inputStream)).use { zipInputStream ->
                var entry = zipInputStream.nextEntry
                while (entry != null) {
                    val name = entry.name
                    val prefix = "SDL-$sdlVersion/android-project/app/src/main/java/org/libsdl/app/"
                    if (name.startsWith(prefix) && !entry.isDirectory) {
                        val fileName = name.substring(prefix.length)
                        val outFile = targetDir.resolve(fileName)
                        outFile.parentFile.mkdirs()
                        FileOutputStream(outFile).use { outputStream ->
                            zipInputStream.copyTo(outputStream)
                        }
                        println("Extracted: $fileName")
                    }
                    entry = zipInputStream.nextEntry
                }
            }
        }
        println("SDL3 Java sources successfully downloaded and extracted.")
    }
}

val copyAssetsTask = tasks.register<Copy>("copyAssetsForAndroid") {
    // Engine default assets from the Atmospheric submodule
    from("../../../Atmospheric/Engine/default_assets") {
        into("assets")
    }
    into(file("build/generated/assets"))
}

android {
    namespace = "com.atmospheric.gradientquad"
    compileSdk = 34
    ndkVersion = "29.0.13113456"

    defaultConfig {
        applicationId = "com.atmospheric.gradientquad"
        minSdk = 26
        targetSdk = 34
        versionCode = 1
        versionName = "1.0.0"

        externalNativeBuild {
            cmake {
                arguments(
                    "-DANDROID_STL=c++_shared",
                    "-DANDROID_ARM_NEON=TRUE",
                    "-DVCPKG_CHAINLOAD_TOOLCHAIN_FILE=${android.ndkDirectory.absolutePath}/build/cmake/android.toolchain.cmake"
                )
                targets("main")
            }
        }
        ndk {
            abiFilters += listOf("arm64-v8a", "x86_64")
        }
    }

    externalNativeBuild {
        cmake {
            // Points to root CMakeLists.txt (3 levels up from platforms/android/app/)
            path = file("../../../CMakeLists.txt")
            // No pinned `version` here: the root CMakeLists.txt requires CMake 3.25+,
            // newer than AGP's default SDK-managed CMake package. Point Gradle at a
            // system CMake instead via `cmake.dir` in local.properties (see README /
            // ci-android.yml, which computes it from `command -v cmake`).
        }
    }

    buildTypes {
        release {
            isMinifyEnabled = false
            proguardFiles(getDefaultProguardFile("proguard-android-optimize.txt"))
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }

    sourceSets {
        getByName("main") {
            assets.srcDir(file("build/generated/assets"))
        }
    }
}

kotlin {
    compilerOptions {
        jvmTarget = org.jetbrains.kotlin.gradle.dsl.JvmTarget.JVM_17
    }
}

tasks.named("preBuild") {
    dependsOn(downloadSDLJavaSources)
    dependsOn(copyAssetsTask)
}

dependencies {
    // SDL3 Java sources are downloaded by downloadSDLJavaSources during preBuild
}
