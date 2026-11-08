# My First PlayStation 1 Game

This repository is a real PS1 homebrew game project written in C with **PSn00bSDK**.

## Build in GitHub Codespaces

Open this repository in a Linux GitHub Codespace. In its terminal, run:

```bash
bash build.sh
```

The first run downloads the PSn00bSDK Linux release and may take a few minutes. Later runs reuse the downloaded SDK.

When the build succeeds, the disc image files should be:

- `build/ps1game.bin`
- `build/ps1game.cue`

Download both files to your Android device and open the `.cue` file in DuckStation.

## Game

**Block Drop** is a falling-block puzzle game. Use the D-pad left and right to move pieces, Cross or Circle to rotate, Down to drop slowly, and Up to drop quickly. Complete rows to clear them and earn points. Press Start after a game over to play again.

## Technology

- C
- PSn00bSDK
- CMake
- Ninja
- GitHub Codespaces
