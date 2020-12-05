# SPACE THIEF

## Description

Space Thief is a Sci-Fi 2D Platformer where our main character, Star is assaulting a Destructor-class Battleship.
His objective: Steal a prototype of a weapon capable of destroying planets. The spacial Robin Hood awakens!

## Key Features

- XML Config File loading
- Load & Save the game using XML
- Tiled TMX Map loading and drawing with various layers
- Metadata Loaded using TMX Map layer "collision"
- Map Collisions detection

- Map navigation: Player Movement and Jump
- Player Idle/Jump/Walk animations
- Player Double Jump
- Player Jump sound

- Logo screen / Level Completed screen
- Fade-in & Fade-out between scenes
- Music for Logo Screen, Level 1 and Level Completed Screen
- DEBUG Keys

- Walking Enemy that can pathfind player
- Fly enemy that can pathfind player and avoids non-walkeable areas
- Load/Save cosiders each enemy state.
- Enemies have a range to detect player

- Game capped to 60 fps
- Window title: FPS|Avg.FPS|LastFrameMS|Vsync
- All movement normalized using DeltaTime
- Destroy enemies: shooting, jumping onto them

- Game items to recover health
- Game score items (diamond)
- Checkpoints, autosave with feedback
- Checkpoints teleportation

- [Optional] Player weapons to shot enemies
- [Optional] Audio Feedback

## Controls

 - A for backward movement
 - D for forward movement
 - SPACE for Jump
 - Double click on SPACE for Double Jump
 - F1 to start the first level
 - F3 to start from the beggining of the current level
 - F5 ​Save the current game state
 - F6 ​Load the previous state
 - F9​ View colliders
 - F10​ God Mode
 - F11 Enable/Disable FPS cap (60 to 30)
 - SHIFT + F11 Enable/Disable FPS cap
 - 1 Teleport to Checkpoint
 - SPACE to start the game in the Logo Screen / leave Level Completed screen
 
## Developers

 - Óscar Royo - Programmer / Artist
 - Albert Pou - Programmer / Artist

## License

This project is licensed under an unmodified MIT license, which is an OSI-certified license that allows static linking with closed source software. 
Check [LICENSE] for further details.