#!/usr/bin/env bash
set -euo pipefail

BASE="$HOME/.local/psn00bsdk"
ZIP="/tmp/PSn00bSDK-0.24-Linux.zip"

sudo apt-get update
sudo apt-get install -y cmake ninja-build curl unzip

if ! find "$BASE" -type f -name mipsel-none-elf-gcc -print -quit 2>/dev/null | grep -q .; then
  echo "Downloading PSn00bSDK v0.24 for Linux..."
  mkdir -p "$BASE"
  curl -fL --retry 3 \
    "https://github.com/Lameguy64/PSn00bSDK/releases/download/v0.24/PSn00bSDK-0.24-Linux.zip" \
    -o "$ZIP"
  unzip -q -o "$ZIP" -d "$BASE"
fi

COMPILER="$(find "$BASE" -type f -name mipsel-none-elf-gcc -print -quit)"
if [ -z "$COMPILER" ]; then
  echo "ERROR: Could not find the PS1 compiler in the downloaded SDK."
  echo "Please send me this terminal output."
  exit 1
fi

SDK_BIN="$(dirname "$COMPILER")"
SDK_ROOT="$(dirname "$SDK_BIN")"
export PSN00BSDK_LIBS="$SDK_ROOT/lib/libpsn00b"
export PATH="$SDK_BIN:$PATH"

echo "PS1 compiler found:"
"$COMPILER" --version | head -n 1

echo "Configuring the PS1 game..."
cmake --preset default

echo "Building the game and PlayStation disc image..."
cmake --build build

echo
echo "BUILD COMPLETE"
find build -maxdepth 1 -type f \( -iname '*.bin' -o -iname '*.cue' \) -print
echo "Download BOTH the .bin and .cue files, then open the .cue in DuckStation."
