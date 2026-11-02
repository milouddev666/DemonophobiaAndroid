# Room Binary Map Format (`maps/roomNN.bin`)

**Status: REVERSE-ENGINEERING REQUIRED**

The original Demonophobia (2008) map format is unknown. The binary map format
documented here is the **port's own format**, designed to be populated once the
original data has been extracted or reverse-engineered.

## Format (little-endian)

| Offset | Type     | Field         | Notes                              |
|--------|----------|---------------|------------------------------------|
| 0      | uint16   | cols          | Tile columns                       |
| 2      | uint16   | rows          | Tile rows                          |
| 4      | uint16   | door_count    |                                    |
| 6      | uint16   | pickup_count  |                                    |
| 8      | uint8[]  | tiles         | `cols × rows` bytes, row-major     |
| 8+tiles| Door[]   | doors         | 29 bytes each (see below)          |
| …      | Pickup[] | pickups       | 13 bytes each (see below)          |
| …      | cstring  | bgm_path      | Null-terminated asset path         |
| …      | cstring  | bg_path       | Null-terminated background sprite  |

## Tile values
| Value | Meaning   |
|-------|-----------|
| 0     | Empty     |
| 1     | Solid     |
| 2     | Ladder    |
| 3     | Hazard    |
| 4     | Water     |

## Door record (29 bytes)
| Offset | Type   | Field        |
|--------|--------|--------------|
| 0      | uint32 | id           |
| 4      | float  | rect.x       |
| 8      | float  | rect.y       |
| 12     | float  | rect.width   |
| 16     | float  | rect.height  |
| 20     | uint32 | targetRoom   |
| 24     | float  | spawnX       |
| 28     | float  | spawnY       |
| 32     | uint8  | keyRequired  |

## Pickup record (13 bytes)
| Offset | Type   | Field       |
|--------|--------|-------------|
| 0      | uint32 | instanceId  |
| 4      | float  | x           |
| 8      | float  | y           |
| 12     | uint8  | itemId      |

## Generating map files
A Python helper script (`tools/build_room.py`) is planned to convert a
human-readable JSON room definition into this binary format.
