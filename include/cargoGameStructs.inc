INCLUDE "include/structs.inc"

    struct PLAYER
        bytes 1, YPos ; 1 byte
        bytes 1, XPos ; 1 byte
        bytes 1, TileTL ; 1 byte tile index for drawing, will be referenced in the update sub
        bytes 1, TileTR ; 1 byte
        bytes 1, TileBL ; 1 byte
        bytes 1, TileBR ; 1 byte
        bytes 1, Direction ; 1 byte
        bytes 1, OAMOffset ; 1 byte which OAM object this will be using to draw to
    end_struct

    struct BOX
        bytes 1, YPos ; 1 byte
        bytes 1, XPos ; 1 byte
        bytes 1, Tile ; 1 byte tile index for drawing/type checking, will be referenced in the update sub
        bytes 1, OAMOffset ; 1 byte which OAM object this will be using to draw to
    end_struct