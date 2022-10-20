## Gest stamp
| Primary gest    | Arm position   | Roll direction | Hand scroll | Hand swipe | Hand position | Pull     |
|-----------------|----------------|----------------|-------------|------------|---------------|----------|
| Possible values | F - flat       | X - none       | X - none    | X - none   | O - open      | X - none |
|                 | U - up         | L - left       | U - up      | L - left   | C - closed    | P - pull |
|                 | D - down       | R - right      | D - down    | R - right  |               |          |
|                 | L - left       |                |             |            |               |          |
|                 | R - right      |                |             |            |               |          |
|                 | O - overturned |                |             |            |               |          |
| Priority        | 1              | 3              | 3           | 3          | 4             | 2        |

## Simple gests list
| ID  | Name              |
|-----|-------------------|
| 0   | Inactive          |
| 1   | Pull              |
| 2   | Static or unused  |
| 3   | Roll left         |
| 4   | Roll right        |
| 5   | Scroll down       |
| 6   | Scroll up         |
| 7   | Swipe left        |
| 8   | Swipe right       |
| 9   | Raised roll left  |
| 10  | Raised roll right |
| 11  | Fist swipe left   |
| 12  | Fist swipe right  |
| 255 | Ambiguous         |

## Simple gests
| ID  | Name              | Arm position | Roll direction  | Hand scroll | Hand swipe | Hand position | Stamp (first 4) | Pull |
|-----|-------------------|--------------|-----------------|-------------|------------|---------------|-----------------|------|
| 1   | Pull              |              |                 |             |            |               |                 | Pull |
| 0   | Inactive          | Down         |                 |             |            |               |                 | None |
| 9   | Raised roll left  | Up           | Left            |             |            |               |                 | None |
| 10  | Raised roll right | Up           | Right           |             |            |               |                 | None |
| 2   | Unused            | Up           |                 | Down        |            |               |                 | None |
| 2   | Unused            | Up           |                 | Up          |            |               |                 | None |
| 2   | Unused            | Up           |                 |             | Left       |               |                 | None |
| 2   | Unused            | Up           |                 |             | Right      |               |                 | None |
| 2   | Static            | Up           |                 |             |            |               |                 | None |
| 3   | Roll left         | Flat         | Left            |             |            |               |                 | None |
| 4   | Roll right        | Flat         | Right           |             |            |               |                 | None |
| 5   | Scroll down       | Flat         |                 | Down        |            |               |                 | None |
| 6   | Scroll up         | Flat         |                 | Up          |            |               |                 | None |
| 7   | Swipe left        | Flat         |                 |             | Left       | Open          |                 | None |
| 8   | Swipe right       | Flat         |                 |             | Right      | Open          |                 | None |
| 12  | Fist swipe right  | Flat         |                 |             | Left       | Closed        |                 | None |
| 11  | Fist swipe left   | Flat         |                 |             | Right      | Closed        |                 | None |
| 2   | Static            | Flat         |                 |             |            |               |                 | None |
| 3   | Roll left         | Left         | Left            |             |            |               |                 | None |
| 4   | Roll right        | Left         | Right           |             |            |               |                 | None |
| 8   | Swipe right       | Left         |                 | Down        |            |               |                 | None |
| 7   | Swipe left        | Left         |                 | Up          |            |               |                 | None |
| 2   | Unused            | Left         |                 |             | Left       |               |                 | None |
| 2   | Unused            | Left         |                 |             | Right      |               |                 | None |
| 2   | Static            | Left         |                 |             |            |               |                 | None |
| 3   | Roll left         | Right        | Left            |             |            |               |                 | None |
| 4   | Roll right        | Right        | Right           |             |            |               |                 | None |
| 7   | Swipe left        | Right        |                 | Down        |            |               |                 | None |
| 8   | Swipe right       | Right        |                 | Up          |            |               |                 | None |
| 2   | Unused            | Right        |                 |             | Left       |               |                 | None |
| 2   | Unused            | Right        |                 |             | Right      |               |                 | None |
| 2   | Static            | Right        |                 |             |            |               |                 | None |
| 3   | Roll left         | Overturned   | Left            |             |            |               |                 | None |
| 4   | Roll right        | Overturned   | Right           |             |            |               |                 | None |
| 2   | Unused            | Overturned   |                 | Down        |            |               |                 | None |
| 2   | Unused            | Overturned   |                 | Up          |            |               |                 | None |
| 2   | Unused            | Overturned   |                 |             | Left       |               |                 | None |
| 2   | Unused            | Overturned   |                 |             | Right      |               |                 | None |
| 2   | Static            | Overturned   |                 |             |            |               |                 | None |
| 255 | Ambiguous         |              | Less than 2 X-s |             |            |               |                 |      |

## Complex gests
| ID | Name                  | Simple gest trigger                | Recoil reduction in cycles | deviceStatus | awakeEnough | Gest mode - A            | Media mode - B | Browser mode - C |
|----|-----------------------|------------------------------------|----------------------------|--------------|-------------|--------------------------|----------------|------------------|
| 0  | Sleep                 | "Inactive OR Pull (0,1)"           |                            | true         |             |                          |                |                  |
| 1  | Wake up               | "Raised rolls (9,10) in any order" |                            | false        |             |                          |                |                  |
| 2  | "Active, no movement" | Static or unused                   |                            | true         |             |                          |                |                  |
| 3  | Roll left             | Roll left (3)                      | 25                         | true         |             | Minimize                 | Mute           | Refresh          |
| 4  | Roll right            | Roll right (4)                     |                            | true         |             | Maximize                 | Play/pause     | Stop             |
| 5  | Scroll down           | Scroll down (5)                    | 25                         | true         |             | Scroll up                | Volume down    | Scroll up        |
| 6  | Scroll up             | Scroll up (6)                      |                            | true         |             | Scroll down              | Volume up      | Scroll down      |
| 7  | Swipe left            | Swipe left (7)                     | 25                         | true         |             | Next virtual desktop     | Next track     | Next tab         |
| 8  | Swipe right           | Swipe right (8)                    |                            | true         |             | Previous virtual desktop | Previous track | Previous tab     |
| 9  | Mode-                 | Raised roll left (9)               | 25                         | true         | true        |                          |                |                  |
| A  | Mode+                 | Raised roll right (10)             |                            | true         |             |                          |                |                  |
| B  | Fist swipe left       | Fist swipe left (11)               | 25                         | true         |             | Next window              |                | Forward          |
| C  | Fist swipe right      | Fist swipe right (12)              |                            | true         |             | Previous window          |                | Back             |

## Device modes
| ID | Mode name    | RGB LED |
|----|--------------|---------|
| A  | Gest mode    | White   |
| B  | Media mode   | Green   |
| C  | Browser mode | Red     |