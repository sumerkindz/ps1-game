# Capturing Reference Gameplay on Android

This workflow is designed for an Android phone or tablet. The reference game is played in an emulator; the recording is evidence for analysis, not an asset source for the new game.

## 1. Keep the reference private
- Store the BIN/CUE in a private Codespaces workspace or local device storage.
- Do not commit commercial game disc images, BIOS files, screenshots, or recordings to a public repository.
- Use only media you are legally allowed to use.

## 2. Play and record focused clips
Use your Android PS1 emulator (for example, DuckStation) and Android's built-in screen recorder. Record short clips, ideally 30–90 seconds each, with the emulator controls visible when possible.

Capture separate clips for:
1. Title screen, menus, and starting a new game.
2. Normal movement and camera behavior.
3. Jumping, attacking, interacting, or the main action.
4. Enemies, hazards, puzzles, or other key systems.
5. Damage, failure, checkpoints, lives, and retry behavior.
6. Level transitions, rewards, and progression.

Do not record long stretches of idle gameplay. Show the action and its result.

## 3. Add observation notes
For each clip, note the game state, button presses, visible result, and approximate timing. If a detail is unclear, mark it unknown rather than guessing. Use `templates/GAMEPLAY_OBSERVATION_LOG.md`.

## 4. Share evidence for analysis
Provide the clips or selected screenshots in the ChatGPT conversation, together with the observation log. If a clip cannot be uploaded, provide timestamped screenshots and describe the button sequence.

The analysis should label each conclusion:
- **Observed**: directly visible in the recording or screenshot.
- **Inferred**: a plausible explanation supported by evidence but not directly confirmed.
- **Unknown**: evidence is insufficient.

## 5. Turn analysis into a new game
The AI should produce a reference-neutral gameplay specification: control feel, camera rules, core loop, timing, challenge patterns, and progression. Then design original characters, setting, level layouts, art, sound, names, and story. Implement the new game independently with PSn00bSDK.

## Important limitation
A BIN/CUE inventory cannot by itself reveal the exact gameplay. This capture-and-observation step is required for a faithful gameplay match. AI analysis can assist design and implementation, but human review and repeated DuckStation testing remain necessary.
