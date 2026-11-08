#!/usr/bin/env bash
set -euo pipefail

SDK_VERSION="0.24"
SDK_ROOT="$HOME/.local/psn00bsdk/PSn00bSDK-${SDK_VERSION}-Linux"
SDK_ZIP="/tmp/PSn00bSDK-${SDK_VERSION}-Linux.zip"

if [ ! -x "$SDK_ROOT/bin/mipsel-none-elf-gcc" ]; then
  echo "Downloading PSn00bSDK ${SDK_VERSION}..."
  mkdir -p "$HOME/.local/psn00bsdk"
  curl -fL --retry 3 \
    "https://github.com/Lameguy64/PSn00bSDK/releases/download/v${SDK_VERSION}/PSn00bSDK-${SDK_VERSION}-Linux.zip" \
    -o "$SDK_ZIP"
  unzip -q -o "$SDK_ZIP" -d "$HOME/.local/psn00bsdk"
fi

export PSN00BSDK_LIBS="$SDK_ROOT/lib/libpsn00b"
export PSN00BSDK_TC="$SDK_ROOT/bin"
export PATH="$SDK_ROOT/bin:$PATH"

echo "Checking PS1 compiler..."
mipsel-none-elf-gcc --version | head -n 1

echo "Configuring the game..."
cmake --preset default

echo "Compiling the PlayStation game and disc image..."
cmake --build build

echo
echo "BUILD COMPLETE"
echo "Look for build/ps1game.bin and build/ps1game.cue"
