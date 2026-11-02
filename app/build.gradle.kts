plugins {
    id("com.android.application")
}

android {
    namespace  = "com.demonophobia.game"
    compileSdk = 34

    defaultConfig {
        applicationId   = "com.demonophobia.game"
        minSdk          = 24      // Android 7.0+ (covers Infinix Smart 8 running Android 14)
        targetSdk       = 34
        versionCode     = 1
        versionName     = "0.1.0-prototype"

        ndk {
            // armeabi-v7a mandatory for Infinix Smart 8 / low-end devices
            // arm64-v8a for modern devices
            abiFilters += listOf("armeabi-v7a", "arm64-v8a")
        }

        externalNativeBuild {
            cmake {
                // C++20 enforced here and in CMakeLists.txt
                cppFlags  += listOf("-std=c++20", "-fexceptions")
                cFlags    += listOf("-std=c11")
                arguments += listOf(
                    "-DANDROID_STL=c++_shared",
                    "-DANDROID_ARM_NEON=TRUE",
                    "-DCMAKE_BUILD_TYPE=Release"
                )
            }
        }
    }

    externalNativeBuild {
        cmake {
            path    = file("src/main/cpp/CMakeLists.txt")
            version = "3.22.1"
        }
    }

    buildTypes {
        debug {
            isDebuggable    = true
            isJniDebuggable = true
            externalNativeBuild {
                cmake {
                    arguments += "-DCMAKE_BUILD_TYPE=Debug"
                    cppFlags  += "-DDEBUG_BUILD"
                }
            }
        }
        release {
            isMinifyEnabled    = false
            isShrinkResources  = false
            proguardFiles(
                getDefaultProguardFile("proguard-android-optimize.txt"),
                "proguard-rules.pro"
            )
        }
    }

    compileOptions {
        sourceCompatibility = JavaVersion.VERSION_17
        targetCompatibility = JavaVersion.VERSION_17
    }

    // NativeActivity app: no Kotlin source sets needed,
    // but the android plugin still needs sourceSets configured.
    sourceSets {
        getByName("main") {
            assets.srcDirs("src/main/assets")
            jniLibs.srcDirs("src/main/jniLibs")
        }
    }

    packaging {
        jniLibs { useLegacyPackaging = false }
    }
}

// No Kotlin/Java runtime dependencies needed for a pure NativeActivity app.
// The android plugin provides android.jar automatically.
dependencies {
    // intentionally empty
}
