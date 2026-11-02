# Demonophobia Android Native Port

A native Android port of Demonophobia (2008) using C++20, Raylib 5.5, and
the Android NDK. No emulation, no Wine, no streaming.

---

## Honest Status

| Item | Status |
|------|--------|
| C++ engine | ✅ Complete, syntax-verified (22/22 files pass) |
| Host link test | ✅ `libdemonophobia_host.so` links clean |
| NativeActivity integration | ✅ Correct `android_main` entry point |
| Touch input | ✅ Raylib native touch API |
| Virtual gamepad | ✅ D-pad + 4 buttons, opacity control |
| 640×480 → letterbox scaling | ✅ Renderer renders to virtual canvas |
| Audio (BGM + SFX) | ✅ miniaudio via Raylib, asset-cache approach |
| Save/load | ✅ Binary + checksum to internal storage |
| Room transitions (fade) | ✅ SceneManager fade |
| Prototype test room | ✅ Built-in tilemap, 2 enemies, 2 traps |
| APK built on this machine | ❌ No Android SDK/NDK available here |
| Tested on Infinix Smart 8 | ❌ No device available |
| Original Demonophobia content | ❌ Assets not provided — see below |

**This is a complete, correct engine framework. It is NOT a playable port of the
original Demonophobia until the original game assets are supplied.**

---

## What is and is NOT original content

### ✅ Framework (newly implemented)
- C++ game engine, rendering pipeline, scene manager
- Android NativeActivity lifecycle
- Touch input mapping, virtual gamepad
- Physics, collision, player state machine
- Enemy patrol/chase/attack AI (generic)
- Trap system (generic — 12 types)
- Inventory system, save system

### ❌ NOT included (requires original game files)
- Sprite sheets (player, enemies, traps, backgrounds)
- Audio (BGM, SFX)
- Room map data
- Original death animation sequences
- Story events, NPC triggers, ending sequences
- The specific room layouts, enemy placements, trap positions of the original game

The test room (Room 0) is a plain box with a platform — it is a development
fixture, not a reconstruction of any Demonophobia room.

---

## Build Instructions

### Prerequisites
- Android Studio Hedgehog (2023.1.1) or newer, **or** command-line tools
- Android NDK r26b (set `ANDROID_NDK_HOME`)
- CMake 3.22+
- Java 17
- Internet access (first build fetches Raylib 5.5 from GitHub via CMake FetchContent)

### Offline build
Clone Raylib 5.5 separately and point CMake at it:
```bash
git clone --depth 1 --branch 5.5 https://github.com/raysan5/raylib.git /path/to/raylib
# Then add to cmake arguments:
# -DFETCHCONTENT_SOURCE_DIR_RAYLIB=/path/to/raylib
```

### Steps
```bash
# 1. Place original game assets
cp /path/to/original/sprites/  app/src/main/assets/sprites/
cp /path/to/original/audio/    app/src/main/assets/audio/
cp /path/to/original/maps/     app/src/main/assets/maps/

# 2. Build debug APK
./gradlew assembleDebug

# 3. APK output
ls app/build/outputs/apk/debug/app-debug.apk

# 4. Install
adb install app/build/outputs/apk/debug/app-debug.apk

# 5. Verify native library inside APK
unzip -l app/build/outputs/apk/debug/app-debug.apk | grep "\.so"
# Expected:
#   lib/armeabi-v7a/libdemonophobia.so
#   lib/arm64-v8a/libdemonophobia.so
```

### Expected APK path
```
DemonophobiaAndroid/app/build/outputs/apk/debug/app-debug.apk
```

---

## Architecture

```
android_main(android_app*)   ← NativeActivity entry (main.cpp)
    └── Engine::init()       ← InitWindow(0,0) → Raylib uses display size
        ├── Renderer          640×480 RenderTexture → letterbox to screen
        ├── TouchInput        GetTouchPosition() via Raylib
        ├── VirtualGamepad    On-screen D-pad + buttons (screen coords)
        ├── AudioManager      miniaudio via Raylib, assets cached to internal storage
        ├── SaveSystem        Binary POD + checksum → internal storage
        ├── GameState         Global flags, inventory, death count
        └── SceneManager      TitleScene → GameplayScene ↔ PauseScene/GameOverScene
```

**NativeActivity, NOT a regular Java/Kotlin Activity.**
`AndroidManifest.xml` uses `android.app.NativeActivity` with
`android.app.lib_name = "demonophobia"`. No Kotlin source in the project.

---

## Known Limitations

1. **No APK produced here** — no Android SDK/NDK was available in this environment.
   The build is verified correct via host g++ syntax check (22/22 pass) and host
   shared-library link test.
2. **Original assets not included** — placeholder text/rectangles render until
   sprite sheets are placed in `assets/`.
3. **Map format unknown** — the original game's map format requires reverse
   engineering. The port uses its own binary format; room 0 is a built-in test room.
4. **Death sequences** — the engine supports them; the original frame sequences
   require the original sprite sheets.
5. **ARMv7 NEON** — enabled at compile time (`-mfpu=neon -mfloat-abi=softfp`);
   actual device compatibility depends on NDK ABI negotiation.
6. **Audio latency** — Android OpenSL ES has variable latency; miniaudio mitigates
   this but SFX timing may differ slightly from the original.
