# A Flicker of Stillness: The Video Game
This project contains the code for the video game I created for my band, *Veracity Plant*. The video game is themed 
around our album *A Flicker of Stillness*, and it can be accessed via a QR code on the back of our CDs. The game's story
and progression will not be detailed in here, since all of that is determined by the "game_data" folder. Instead, 
you will learn about how the game engine works, and, in turn, be capable of creating your own game inside of it. If you 
wish to learn more about the game's story and progression, there is another file titled "GAME.md" that details the
inner-workings of the specific implementation that I created for this game engine.

## Table of Contents:
- [Prerequisites](#Prerequisites)
- [Files](#Files)
  - [Map Files](#Map-Files)
  - [Save Files](#Save-Files)
  - [Entity Info File](#Entity-Info-File)
  - [Weapon Info File](#Weapon-Info-File)
  - [Settings File](#Settings-File)

## Prerequisites
- Toolchain Requirements
  - **CMake**: Version **3.20** or higher.
  - **C Compiler**: A standard C compiler supporting **C99**.
- External Libraries
  - **SDL3** (Development Libraries)
  - **SDL3_image** (Development Libraries)

*Note: Within CMakeList.txt, you will see the file paths I used for both SDL3 and SDL3_image. If your SDL libraries 
exist at a different location on your computer, make sure to change these! (It also tells you where to do so in the 
CMakeList.txt file)*

## Build Instructions

### Building via Command Line

1. **Clone the repository and navigate to the project directory:**

    ```bash
    git clone https://github.com/ColePickley/AFlickerOfStillness
    ```

2. **Navigate to the project directory:**

    ```bash
    cd your_file_path/AFlickerOfStillness
    ```

3. **Generate build configuration files:**

   - Windows (matching default paths)

    ```bash
    cmake -B build
    ```

   - Linux / macOS / Custom Path setups

    ```bash
    cmake -B build -DCMAKE_PREFIX_PATH="/usr/local;/opt/homebrew"
    ```

4. **Compile the executable:**

    ```bash
    cmake --build build
    ```

5. **Run the video game:**

   - Windows

    ````bash
    ./build/A Flicker of Stillness.exe
    ````

    - Linux / macOS

    ````bash
    ./build/A Flicker of Stillness
    ````

## Files

### Map Files

**Sections:** 
map_grid, chunk_grid, texture_locations, checkpoints, entities, sky_texture, player

- **map_grid:**
Rows and columns correspond to the map itself and what each cell's type is.
  - **Cell Types:** -2 = open door, -1 = checkpoint, 0 = empty, 1 = wall, 2 = door

- **chunk_grid:**
Rows and columns correspond to the map itself and what each cell's chunk is.

  - **Chunks:** When a player enters a cell with a new chunk index, all the entities that exist within the cells that have
  that same chunk index are loaded in. Doors only open when all the entities in that door's chunk are removed. When a 
  player enters a chunk that is equal to negative one times a doors chunk, that door will close. Because of the way doors 
  work in relation to chunks, you can design a section where a player must kill all the enemies in order to open a door. 
  You can also design a section where there's a key (which is an entity) within a door's chunk. When the player picks up 
  that key, all entities in that door's chunk have been removed; therefore, the door will be able to open when the player 
  picks up the key.

- **texture_locations:**
Each section of 4 values corresponds to a row, column, side (0 - 5), and texture index.

- **checkpoints:**
Each row corresponds to one checkpoint.

  - **Arrangement:** The first three values correspond to the checkpoint's x, y, and number of pages. Since checkpoints 
  often contain dialogue, there are pages that the dialogue goes on. Within the pages there are dialogue boxes that the 
  actual text goes inside, and the checkpoints are arranged in the csv file by dialogue box. For each dialogue box, the 
  first value corresponds to its page number. The second value is the text contained inside the dialogue box. The third 
  value is alignment of the text and dialogue (left or right). The final three values represent the r, g, and b components
  of the dialogue box's color.

- **entities:**
Each row corresponds to one entity, and they contain the entity's grid x, grid y, and texture index.

- **sky_texture:**
This is the index of the sky texture used for the level.

- **player:**
This contains the player's x, y, and rotation.

### Save Files

**Format:**
level_index, map_grid, num_checkpoints_hit, entities, player

- **map_grid:**
This is the same deal as the Map File, but it's also put in the save files to make sure doors are in the correct state.

- **num_checkpoints_hit:**
When the game reads the checkpoints from the Map File, it loads them into a queue that corresponds to the order in which
they are hit (they're already arranged this way in the Map File). When you load a save, it first reads the map file, 
then it reads the save data. To prevent the save reader from having to reload all the checkpoints, the game counts the 
number of checkpoints that have been hit so it can deque all the checkpoints that have already been hit.

- **entities:**
When the entity is loaded in from the map, it has a default state detailed in the Entity Info file; however, once the 
player has played part of the level, certain things change. For this reason, the entities from the Map File reading are 
scrapped, and the new entities are loaded in with the following format: x, y, index, health, melee_cool_down_state, 
projectile_cool_down_state, ammo_count, mag_count. The index functions the same as it has before, but the x and y values
no longer correspond to grid values. Instead, they are doubles that represent the exact position of the entity. Health 
represents the entity's current health since it might have changed, and the melee cool-down and projectile cool-down 
states represent the cool-down progression that the entity has undergone when the level was saved. Finally, the ammo and
magazine counts are included because when a player picks up a new weapon, it drops the current weapon in its inventory 
as an entity. This entity will have a specific ammo and magazine count based on how much the player has used that 
weapon.

- **player:** 
The first three values correspond to the player's x, y, and rotation, just like they did in the Map File, but there are 
more values as well. The player's health (an integer ranging from 0 to 100) is the third value. The final four values 
represent the player's primary weapon index, ammo count, magazine count, and secondary weapon.

### Entity Info File

**Format:** 
index, scale, height, type, health_amount, weapon_index, mag_amount, ammo_type, melee_range, projectile_range, 
melee_cool_down, projectile_cool_down, melee_damage, projectile_damage, projectile_index, speed, health

- **index:**
The index of the entity's texture.

- **scale:**
64 means the entity spans from the floor to the ceiling, 0 means the enemy won't appear.

- **height:**
0 means the entity in on the ground, 64 means it's bottom is where the ceiling starts (theoretically).

- **type:**
  - 0 = aesthetic: The player cannot interact with the entity
  - 1 = collectable_health: The player can pick up the entity, and it will increase their health
  - 2 = collectable_weapon: The player can pick up the entity, and it will equip them with the corresponding weapon
  - 3 = collectable_ammo: The player can pick up the entity, and it will increase their ammo
  - 4 = collectable_key: The player can pick up the entity, and it will allow them to open a door
  - 5 = projectile: Enemies fire this entity, and it does damage to the player when touched
  - 6 = enemy_melee: Enemy that can only damage the player via melee
  - 7 = enemy_projectile: Enemy that can only damage the player via firing a projectile
  - 8 = enemy_melee_and_projectile: Enemy that can damage the player via melee or firing a projectile

- **health_amount (type 1):** 
Specifies the amount of health the player gains.

- **weapon_index (type 2):** 
Specifies the index of the weapon that the entity represents.

- **mag_amount (type 3):**
Specifies the number of magazines the player gains.

- **ammo_type (type 3):**
Specifies the type of ammo the player is picking up.

- **melee_range (types 6 and 8):**
Specifies the distance the enemy has to be from the player to melee.

- **projectile_range (types 7 and 8):**
Specifies the distance the enemy has to be from the player to fire a projectile.

- **melee_cool_down (types 6 and 8):**
Specifies the time the enemy has to wait between melee attacks.

- **projectile_cool_down (types 7 and 8):**
Specifies the time the enemy has to wait between firing projectiles.

- **melee_damage (types 6 and 8):**
The amount the damage the enemy does to the player when performing a melee attack.

- **projectile_damage (type 5):**
Amount of damage the projectile does to the player when it touches them.

- **projectile_index (types 7 and 8):** 
The entity index of the projectile the enemy fires.

- **speed (types 5 - 8):** 
The speed that the entity moves at (10 = the player's normal speed, 15 = the player's sprint speed).

- **health (types 6 - 8):** 
The amount of health the enemy has

### Weapon Info File

**Format:** 
index, type, damage, ammo_type, projectile_index, max_ammo_count, default_mag_count, max_mag_count, range, 
attack_cool_down, relead_speed

- **index:** 
The index of the weapon's texture.

- **type:**
  - 0 = melee: Only deals damage when the entity is in close range
  - 1 = ranged: Deals damage by hit-scan and fires once for each mouse click
  - 2 = automatic: Deals damage by hit-scan and fires at a flat rate when the mouse is held down
  - 3 = projectile: Fires projectile entities that deal damage

- **damage (all types):**
The amount of damage a weapon deals

- **ammo_type (types 1 and 2):**
The type index of ammo that the weapon excepts

- **projectile_index (type 3):**
The type index of the projectile that the weapon shoots

- **max_ammo_count (types 1 - 3):**
Amount of ammo in each magazine

- **default_mag_count (types 1 - 3):**
The number of magazines the weapon has when you first pick it up

- **max_mag_count (types 1 - 3):**
The max number of magazines the weapon can have

- **range (types 0 - 2):**
How close you have to be to the entity.

### Settings File

**Format:**
frame_rate_cap, fullscreen, graphics_quality, sensitivity

- **frame_rate_cap:**
This is either 15, 30, or 60.

- **fullscreen:**
This is a boolean variable that is either 1 or 0 (true or false).

- **graphics_quality:**
The floor and ceiling cost the most processing power to render, so the vertical size of the pixels (the width is always 
equal to the width of the rays) can be adjusted. LOW = 6, MEDIUM = 3, and HIGH = 1.

- **sensitivity:**
This is the amount that the player rotates when they move their cursor on the screen. The value ranges from 0 to 100 
where 0 means the rotation doesn't change at all, 50 establishes a 1:1 ratio between the number of pixels the cursor 
moves and the number of pixels the player rotates, and 100 makes that ratio 1:2.