# TODO — Demonophobia Android Port

## Immediate (required to run original game content)
- [ ] Obtain and place original game assets in `app/src/main/assets/`
  - `sprites/player.png`     — player sprite sheet (32×48 frames, 8 cols)
  - `sprites/enemy_0.png`    — Ghost enemy sheet
  - `sprites/enemy_1.png`    — Crawler enemy sheet
  - `sprites/enemy_2.png`    — Shadow enemy sheet
  - `audio/bgm_room00.ogg`   — Room 0 background music
  - `audio/sfx_death.wav`    — Death sound
  - `audio/sfx_pickup.wav`   — Item pickup sound
  - `maps/room00.bin`        — Room 0 binary map (see docs/map_format.md)
- [ ] Parse original Demonophobia map file format (unknown — requires RE work)
- [ ] Implement all 14+ death animation sequences from original game
- [ ] Map all original room layouts into the binary map format

## Build
- [ ] Run `./gradlew assembleDebug` on a machine with Android SDK + NDK r26b+
- [ ] Verify `lib/armeabi-v7a/libdemonophobia.so` exists in APK
- [ ] Test on Infinix Smart 8 (Android 14, armeabi-v7a)

## Gameplay (prototype → original)
- [ ] Implement all original room transitions and door logic
- [ ] Implement original enemy patrol paths per room
- [ ] Implement all 12 trap types with correct trigger geometry
- [ ] Implement full inventory item effects (keys, tools, usables)
- [ ] Implement NPC/story event triggers
- [ ] Implement ending sequences

## Polish
- [ ] Add touch button position customisation (drag-to-reposition)
- [ ] Add opacity slider for virtual gamepad
- [ ] Add options screen (volume, language)
- [ ] ARM64 optimisation pass
- [ ] 30 FPS cap mode for battery saving
- [ ] Haptic feedback on death
