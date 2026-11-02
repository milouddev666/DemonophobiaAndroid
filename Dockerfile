# Build the Android APK in a reproducible Android SDK/NDK environment.
FROM ubuntu:22.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive
ENV ANDROID_HOME=/opt/android-sdk
ENV ANDROID_SDK_ROOT=/opt/android-sdk
ENV ANDROID_NDK_HOME=/opt/android-sdk/ndk/26.3.11579264
ENV ANDROID_NDK_ROOT=/opt/android-sdk/ndk/26.3.11579264
ENV PATH=/opt/gradle/gradle-8.6/bin:/opt/android-sdk/cmdline-tools/latest/bin:/opt/android-sdk/platform-tools:$PATH

RUN apt-get update && apt-get install -y --no-install-recommends \
    openjdk-17-jdk wget unzip git ca-certificates python3 \
    build-essential file && \
    rm -rf /var/lib/apt/lists/*

# Android command-line tools
RUN mkdir -p ${ANDROID_HOME}/cmdline-tools && \
    wget -q https://dl.google.com/android/repository/commandlinetools-linux-11076708_latest.zip -O /tmp/cmdline-tools.zip && \
    unzip -q /tmp/cmdline-tools.zip -d /tmp/android-tools && \
    mv /tmp/android-tools/cmdline-tools ${ANDROID_HOME}/cmdline-tools/latest && \
    rm -rf /tmp/android-tools /tmp/cmdline-tools.zip

# Install the SDK, NDK and CMake versions used by this project.
RUN yes | sdkmanager --licenses >/dev/null && \
    sdkmanager --install \
      "platform-tools" \
      "platforms;android-34" \
      "build-tools;34.0.0" \
      "ndk;26.3.11579264" \
      "cmake;3.22.1"

# Gradle wrapper JAR is absent from the uploaded project, so install Gradle itself.
RUN wget -q https://services.gradle.org/distributions/gradle-8.6-bin.zip -O /tmp/gradle.zip && \
    unzip -q /tmp/gradle.zip -d /opt && \
    rm /tmp/gradle.zip

WORKDIR /workspace
COPY . .

# Build and fail the image if the APK or the required ARMv7 library is missing.
RUN gradle --no-daemon --stacktrace assembleDebug && \
    test -s app/build/outputs/apk/debug/app-debug.apk && \
    unzip -l app/build/outputs/apk/debug/app-debug.apk | grep -q 'lib/armeabi-v7a/libdemonophobia.so' && \
    mkdir -p /out && \
    cp app/build/outputs/apk/debug/app-debug.apk /out/Demonophobia-debug-armv7.apk

# Small runtime image: only serves the generated APK.
FROM python:3.12-slim AS runtime
WORKDIR /app
COPY --from=builder /out/Demonophobia-debug-armv7.apk /app/Demonophobia-debug-armv7.apk
COPY render_server.py /app/render_server.py
ENV PORT=10000
EXPOSE 10000
CMD ["python3", "/app/render_server.py"]
