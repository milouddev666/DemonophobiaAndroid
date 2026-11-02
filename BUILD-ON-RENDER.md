# Build Demonophobia Android prototype on Render

## What this setup does
- Builds the existing Android project inside Docker.
- Uses Java 17, Gradle 8.6, Android SDK 34, NDK 26.3.11579264 and CMake 3.22.1.
- Checks that the generated APK contains `lib/armeabi-v7a/libdemonophobia.so`.
- Serves the resulting debug APK from the Render web service.

## Deploy
1. Extract this ZIP.
2. Upload the contents of the `DemonophobiaAndroid` folder to a new GitHub repository (the `Dockerfile`, `render.yaml` and `render_server.py` must be at repository root).
3. In Render, choose **New → Blueprint** and connect that repository.
   - Alternatively choose **New → Web Service**, connect the repository, select Docker, and use the repository root as the Docker context.
4. Wait for the Docker build and deployment to finish.
5. Open the Render service URL. If the build succeeded, the page contains a download link for `Demonophobia-debug-armv7.apk`.

## Important
- The first build downloads Raylib 5.5 from GitHub through CMake FetchContent, so the Docker build needs outbound internet access.
- Render builds the Docker image before starting the service. If the Android compilation fails, the service will not deploy; inspect the Render build logs and share the first actual compiler/CMake error.
- This is a debug prototype, not the original Demonophobia game. Original sprites, maps, audio and story content are not included.
- The project has no Gradle wrapper JAR, so the Dockerfile installs and invokes Gradle 8.6 directly.
- A successful Docker build verifies the APK contains ARMv7 native code; it does not prove the app launches correctly on the Infinix Smart 8. Device testing is still required.
