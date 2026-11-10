# AI-Assisted Original PS1 Game Creator

## Purpose
Use a user-provided PS1 BIN/CUE as a reference for broad game-design patterns, then create a completely separate, original PS1 game. The reference disc is never used as the final executable or asset base.

## Android-first workflow
1. Keep the reference BIN/CUE in a private Codespaces workspace, not in a public Git repository.
2. Run `python3 tools/ps1_reference_inspector.py /path/to/reference.cue --output reports`.
3. Review the generated JSON inventory and AI analysis brief.
4. Supply gameplay video, screenshots, or observation notes from playing the reference in an emulator. Static disc metadata cannot tell the AI exactly how the game plays.
5. Ask an AI agent to separate observed facts, inferences, and unknowns.
6. Produce a new game design document with a distinct title, premise, characters, world, mechanics, levels, art, music, and code.
7. Implement original source using PSn00bSDK.
8. Build using `bash build.sh`.
9. Download both BIN and CUE from `build/` to Android and open the CUE in DuckStation.
10. Record real emulator test results; a successful compilation alone is not proof of compatibility.

## Components
- Reference intake: CUE parsing, missing-file checks, file sizes and SHA-256.
- Evidence analysis: gameplay footage/notes; mark every claim observed, inferred, or unknown.
- Original design: new game concept, control plan, progression, asset inventory and PS1 feasibility.
- Original implementation: PSn00bSDK C/C++ source isolated from reference media.
- Build and test: produce BIN/CUE and test boot, input, core loop, collision, transitions, sound and sustained play.

## Safety and IP boundaries
- Use lawfully obtained reference media.
- Do not upload copyrighted disc images to public repositories or public AI services.
- Do not include reference-game assets, source code, music, dialogue, maps, or characters in the new game.
- Never execute unknown programs extracted from a disc.
- Keep reference media, generated reports containing sensitive paths, and build binaries out of public Git history.

## Current limitations
- The inspector inventories disc metadata; it does not decode every proprietary PS1 filesystem or asset format.
- Automated gameplay understanding from raw BIN/CUE is not implemented. It requires emulator video/observation and AI analysis.
- The AI is a supervised design/coding assistant, not an autonomous game-development guarantee.
- The existing root Block Drop game remains intact and is not overwritten by this system.
