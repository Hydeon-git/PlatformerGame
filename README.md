# SPACE THIEF

## Description

Space Thief is a Sci-Fi 2D Platformer where our main character, Star is assaulting a Destructor-class Battleship.
His objective: Steal a prototype of a weapon capable of destroying planets. The spacial Robin Hood awakens!

## Key Features

- XML Config File loading.
- Load & Save the game using XML.
- Tiled TMX Map loading and drawing with various layers.
- Metadata Loaded using TMX Map layer "collision".

- Map Collisions detection.
- Map navigation: Player Movement and Jump.
- Player Idle/Jump/Walk/Hit/Death animations.
- Player Double Jump.

- Player Jump sound.
- Logo screen / Level Completed screen.
- Fade-in & Fade-out between scenes.
- Music for Logo Screen, Level 1 and Level Completed Screen.

- DEBUG Keys.
- Walking Enemy that can pathfind player and can fall down to other platforms.
- Fly enemy that can pathfind player and avoids non-walkeable areas.
- Load/Save considers each enemy state.

- Enemies have a range to detect player.
- Game capped to 60 fps, it can be capped to 30 fps or just be uncapped.
- Window title shows: FPS | Avg.FPS | LastFrameMS | Vsync. 
- All movement normalized using DeltaTime.

- Destroy enemies: shooting.
- Game items to recover health(potion).
- Game score items (diamond).
- All collectible items have animations and audio feedback when collected.

- Checkpoints have autosave and feedback when are reached by the player.
- Checkpoints teleportation using Key "1".
- Player have a pistol that can shoot enemies and kill them.
- Audio Feedback for the game mechanics implemented.

Implementations to Highlight
- Calculation of the pixel perfect position in order to move the player pixel per pixel in the scene.
  This aligns all the pixels of the diferent sprites to match perfectly.
- Checkpoint in the middle of the map. When it's reached by the player it saves automatically the game.
- Collectable objects are created from the config file in the scene section with this configuration: 
  <obj x="x_position" y="y_position" type="type_of_object"/> types: 1 - diamond, 2 - healing potion.

## Controls
### Game controls
 - A for backward movement
 - D for forward movement
 - SPACE for Jump
 - Double click on SPACE for Double Jump
 - E to shoot
 - F1 to start the first level
 - F3 to start from the beggining of the current level
 - F5 ​Save the current game state
 - F6 ​Load the previous state

### Debug controls
 - F9​ View colliders and pathfinding logic
 - F10​ God Mode
 - F11 Enable/Disable FPS cap (60 to 30)
 - SHIFT + F11 Enable/Disable FPS cap
 - 1 Teleport to Checkpoint
 - J to kill all enemies
 - K to kill player
 
## Developers

 - Óscar Royo - Programmer / Artist
 - Albert Pou - Programmer / Artist

## License

This project is licensed under an unmodified MIT license, which is an OSI-certified license that allows static linking with closed source software. 
Check [LICENSE] for further details.