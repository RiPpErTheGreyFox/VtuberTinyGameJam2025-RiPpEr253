SECTION "Gameplay Specific Variables", WRAM0

wLevelBoxCount: db
wLevelFlammableBoxCount: db
wLevelRadioactiveBoxCount: db
wLevelScoreAreaLeftX: db
wLevelScoreAreaTopY: db
wLevelScoreAreaRightX: db
wLevelScoreAreaBottomY: db

SECTION "Animation Variables", WRAM0
wPlayerAnimationCounter: db				; counts frames before next frame of animation
wPlayerAnimationCurrentFrame: db		; keeps track of the current frame
wPlayerTileTLOffsetX: db
wPlayerTileTROffsetX: db
wPlayerTileBLOffsetX: db
wPlayerTileBROffsetX: db
wPlayerFlipped: db

SECTION "Gameplay Subroutines", ROM0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;	GAME SPECIFIC SUBROUTINE
;;	BLOCK
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

InitialisePlayer:
	; initialise player variables and load/define the tiles needed
	ld a, 0
	ld [wVictoryFlagSet], a
	ld a, 32
	ld [mainCharacter_XPos], a
	ld [mainCharacter_XDestination], a
	ld a, 32
	ld [mainCharacter_YPos], a
	ld [mainCharacter_YDestination], a
	ld a, 0
	ld [mainCharacter_Direction], a
	ld [mainCharacter_TileFlags], a
	ld a, 128
	ld [mainCharacter_OAMOffset], a
	ld a, 1
	ld [mainCharacter_AllowMove], a

	; initialise animation variables
	ld a, 0
	ld [wPlayerAnimationCounter], a
	ld [wPlayerAnimationCurrentFrame], a
	ld [wPlayerFlipped], a

	call UpdatePlayerFlipped
	

	ld de, PlayerSpriteData
	ld bc, PlayerSpriteDataEnd - PlayerSpriteData
	ld a, 0

	call TileLoader

	; for now, main character tiles are upper is $+2 and lower is $+7
	ld a, PLAYER_TL_FRAME1_TILE
	ld [mainCharacter_TileTL],a
	ld a, PLAYER_TR_FRAME1_TILE
	ld [mainCharacter_TileTR], a
	ld a, PLAYER_BL_FRAME1_TILE
	ld [mainCharacter_TileBL],a
	ld a, PLAYER_BR_FRAME1_TILE
	ld [mainCharacter_TileBR], a

	; get OAM offset
	ld a,  [mainCharacter_OAMOffset]	; get the offset from the start of OAM
	ld h, 0
	ld l, a								; load the offset into hl
	ld bc, $FE00						; start of OAM memory
	add hl, bc							; add the offset and the start of OAM together

	call WaitNoMode3

	; assign and update the OAM's
	ld a, [mainCharacter_YPos]
	ld [hli], a
	ld a, [mainCharacter_XPos]
	ld [hli], a
	ld a, [mainCharacter_TileTL]
	ld [hli], a
	ld a, %00000000
	ld [hli], a
	
	ld a, [mainCharacter_YPos]
	ld [hli], a
	ld a, [mainCharacter_XPos]
	add 8
	ld [hli], a
	ld a, [mainCharacter_TileTR]
	ld [hli], a
	ld a, %00000000
	ld [hli], a
	
	ld a, [mainCharacter_YPos]
	add 8
	ld [hli], a
	ld a, [mainCharacter_XPos]
	ld [hli], a
	ld a, [mainCharacter_TileBL]
	ld [hli], a
	ld a, %00000000
	ld [hli], a
	
	ld a, [mainCharacter_YPos]
	add 8
	ld [hli], a
	ld a, [mainCharacter_XPos]
	add 8
	ld [hli], a
	ld a, [mainCharacter_TileBR]
	ld [hli], a
	ld a, %00000000
	ld [hli], a

	ret

; When passed in an XY, determine if tile is passable or not
; @param b: X
; @param c: Y
; @return a: 1 if impassable otherwise 0
CheckCollisionAtXY:
	; bc is already in the registers ready to call
	call GetTileAddressByPixel
	
	call WaitNoMode3
	
	ld a, [hl]						; get the tile type at that address
	cp a, 127						; compare to 127, if > 127, carry bit will be not set
	jp nc, .Impassable				; if carry bit not set, there's already a box there, jump out

	; redefine impassable tiles when doing the next art pass
	; tiles 1, 2 and 3 in the background bank are impassable
	;cp a, 1
	;jp z, .Impassable

	;cp a, 2
	;jp z, .Impassable
	
	;cp a, 3
	;jp z, .Impassable
.Passable
	ld a, 0
	ret

.Impassable
	ld a, 1
	ret

; Function for checking and adjusting the direction based offsets for box related positions
; @param bc: x/y
; @clobbers a
; @return bc: x/y
AdjustBoxOffsetForDirection:
	; do direction based offsetting 0 = N, 1 = E, 2 = S, 3 = W
	ld a, [mainCharacter_Direction]
	cp a, 0
	jp z, .NorthCursor
	cp a, 1
	jp z, .EastCursor
	cp a, 2
	jp z, .SouthCursor
	cp a, 3
	jp z, .WestCursor

.NorthCursor						; offset the cursor to appear in the correct spot
	ld a, b							; remember that it starts at -8/+8
	add 8
	ld b, a
	ld a, c
	sub 16
	ld c, a
	jp .EndOfFunc
.EastCursor
	ld a, b				
	add 24
	ld b, a
	ld a, c
	add 0
	ld c, a
	jp .EndOfFunc
.SouthCursor	
	ld a, b				
	add 8
	ld b, a
	ld a, c
	add 8
	ld c, a
	jp .EndOfFunc
.WestCursor
	; defaults are for west cursor
	jp .EndOfFunc

.EndOfFunc
	ret

; UpdatePlayer runs through the per frame updates required
; usually input and collision checking
UpdatePlayer:
.UpdateMovementToDestination
.CheckYDestinationPositionDifference
	; clear the allow move flag register
	ld de, 0
	; if the destination != the current position, process updates to move the player towards it
	ld a, [mainCharacter_YPos]
	ld b, a
	ld a, [mainCharacter_YDestination]
	cp a,b
	; do the move closer math here
	jp z, .YIsTheSame
	jp c, .SubFromY
	; if Y < Destination, add to Y
	; if Y > Destination, subtract from Y
.AddToY
	ld a, [mainCharacter_YPos]
	inc a
	ld [mainCharacter_YPos], a		
	jp .CheckXDestinationPositionDifference
.SubFromY			
	ld a, [mainCharacter_YPos]
	dec a
	ld [mainCharacter_YPos], a
	jp .CheckXDestinationPositionDifference
.YIsTheSame
	ld d, 1
	ld [mainCharacter_AllowMove], a
.CheckXDestinationPositionDifference
	; if the destination != the current position, process updates to move the player towards it
	ld a, [mainCharacter_XPos]
	ld b, a
	ld a, [mainCharacter_XDestination]
	cp a,b
	; do the move closer math here
	jp z, .XIsTheSame
	jp c, .SubFromX
	; if Y < Destination, add to X
	; if Y > Destination, subtract from X
.AddToX
	ld a, [mainCharacter_XPos]
	inc a
	ld [mainCharacter_XPos], a		
	jp .CheckIfAcceptingNewDestination
.SubFromX			
	ld a, [mainCharacter_XPos]
	dec a
	ld [mainCharacter_XPos], a
	jp .CheckIfAcceptingNewDestination
.XIsTheSame
	ld e, 1
	ld [mainCharacter_AllowMove], a
.CheckIfAcceptingNewDestination
	; check if both d and e flags are set
	ld a, d
	cp a, 1
	jp nz, .KeyCheckFinished
	ld a, e
	cp a, 1
	jp nz, .KeyCheckFinished
	ld a, 1
	ld [mainCharacter_AllowMove], a

	ld a,[mainCharacter_AllowMove]			; check if we're allowed to accept a new destination
	cp a, 1									; allowMove == 1 means accept new input
	jp nz, .KeyCheckFinished				; otherwise just skip movement input
.CheckForKeyPresses
	ld a, [wCurKeys]
	and a, PADF_UP
	jp z, .CheckDownPressed

	ld a, [mainCharacter_YDestination]
	; check for low Y clamp
	cp a, 8
	jp c, .CheckDownPressed					; if Y is 8 or lower, just jump
	sub 8
	ld [mainCharacter_YDestination], a
	ld a, 0
	ld [mainCharacter_Direction], a
	ld a, 0
	ld [mainCharacter_AllowMove], a

.CheckDownPressed
	ld a, [wCurKeys]
	and a, PADF_DOWN
	jp z, .CheckLeftPressed

	ld a, [mainCharacter_YDestination]
	; check for high Y clamp
	cp a, 144 - 16
	jp nc, .CheckLeftPressed
	add 8
	ld [mainCharacter_YDestination], a
	ld a, 2
	ld [mainCharacter_Direction], a
	ld a, 0
	ld [mainCharacter_AllowMove], a

.CheckLeftPressed
	ld a, [wCurKeys]
	and a, PADF_LEFT
	jp z, .CheckRightPressed

	ld a, [mainCharacter_XDestination]
	; check for low X clamp
	cp a, 8
	jp c, .CheckRightPressed				; if Y is 8 or lower, just jump
	sub 8
	ld [mainCharacter_XDestination], a
	ld a, 3
	ld [mainCharacter_Direction], a
	ld a, 0
	ld [mainCharacter_AllowMove], a

.CheckRightPressed
	ld a, [wCurKeys]
	and a, PADF_RIGHT
	jp z, .KeyCheckFinished

	ld a, [mainCharacter_XDestination]
	; check for high X clamp
	cp a, 160 - 16
	jp nc, .KeyCheckFinished
	add 8
	ld [mainCharacter_XDestination], a
	ld a, 1
	ld [mainCharacter_Direction], a
	ld a, 0
	ld [mainCharacter_AllowMove], a

.KeyCheckFinished

.DestinationCollisionCheck
	; take the intended destination, and if it would collide, just back out

	; do direction based collision checking 0 = N, 1 = E, 2 = S, 3 = W
	ld a, [mainCharacter_Direction]
	cp a, 0
	jp z, .NorthCheck
	cp a, 1
	jp z, .EastCheck
	cp a, 2
	jp z, .SouthCheck
	cp a, 3
	jp z, .WestCheck

	; TODO: exception handling
	jp .FinishCollisionChecks

.NorthCheck
	ld a, [mainCharacter_XDestination]
	ld b, a
	ld a, [mainCharacter_YDestination]
	ld c, a
	call CheckCollisionAtXY

	cp a, 1
	jp z, .Collided

	ld a, [mainCharacter_XDestination]
	add 8
	ld b, a
	ld a, [mainCharacter_YDestination]
	ld c, a
	call CheckCollisionAtXY

	cp a, 1
	jp z, .Collided

	jp .FinishCollisionChecks
.EastCheck
	ld a, [mainCharacter_XDestination]
	add 8
	ld b, a
	ld a, [mainCharacter_YDestination]
	ld c, a
	call CheckCollisionAtXY

	cp a, 1
	jp z, .Collided

	ld a, [mainCharacter_XDestination]
	add 8
	ld b, a
	ld a, [mainCharacter_YDestination]
	add 8
	ld c, a
	call CheckCollisionAtXY

	cp a, 1
	jp z, .Collided

	jp .FinishCollisionChecks
.SouthCheck
	ld a, [mainCharacter_XDestination]
	ld b, a
	ld a, [mainCharacter_YDestination]
	add 8
	ld c, a
	call CheckCollisionAtXY

	cp a, 1
	jp z, .Collided

	ld a, [mainCharacter_XDestination]
	add 8
	ld b, a
	ld a, [mainCharacter_YDestination]
	add 8
	ld c, a
	call CheckCollisionAtXY

	cp a, 1
	jp z, .Collided

	jp .FinishCollisionChecks
.WestCheck
	ld a, [mainCharacter_XDestination]
	ld b, a
	ld a, [mainCharacter_YDestination]
	ld c, a
	call CheckCollisionAtXY

	cp a, 1
	jp z, .Collided

	ld a, [mainCharacter_XDestination]
	ld b, a
	ld a, [mainCharacter_YDestination]
	add 8
	ld c, a
	call CheckCollisionAtXY

	cp a, 1
	jp z, .Collided

	jp .FinishCollisionChecks

.Collided									; just stop our character where he is
	ld a, [mainCharacter_XPos]				; grab the current position
	and %11111000							; truncate it down to the closest align 8
	ld [mainCharacter_XDestination], a
	
	ld a, [mainCharacter_YPos]
	and %11111000
	ld [mainCharacter_YDestination], a

.FinishCollisionChecks
	ld a, [mainCharacter_AllowMove]
	cp a, 1
	jp z, .ResetToFrame1					; if we're not allowed to move, assume we're moving
	call UpdatePlayerAnimation				; only call player animation update when moving
	jp .EndOfFunc
.ResetToFrame1
	ld a, $FF								; go to the last frame of the animation so that the next frame command works
	call SetPlayerAnimationFrame
.EndOfFunc
	call UpdatePlayerFlipped
	ret

; keeps track of the animation counter and updates the tile references needed
; @clobbers a
UpdatePlayerAnimation:
		; check the frame counter
	ld a, [wPlayerAnimationCounter]
	; if above CONVEYOR_BELT_ANIMATION_SPEED, reset it and swap the tiles
	cp a, PLAYER_ANIMATION_SPEED
	jp nc, .TimeToSwap
	inc a
	ld [wPlayerAnimationCounter], a
	jp EndOfAnimFunc

.TimeToSwap
	; we're moving so play the walking sound
	ld hl, sound_character_walk
	call PlaySoundHL

	ld a, 0
	ld [wPlayerAnimationCounter], a

	ld a, [wPlayerAnimationCurrentFrame]	
SetPlayerAnimationFrame:	
	; check the frame number and jump to the correct next frame (zero indexed)
	cp a, 0
	jp z, .SecondFrame
	cp a, 1
	jp z, .ThirdFrame
.FirstFrame
	ld a, PLAYER_TL_FRAME1_TILE
	ld [mainCharacter_TileTL], a
	ld a, PLAYER_TR_FRAME1_TILE
	ld [mainCharacter_TileTR], a
	ld a, PLAYER_BL_FRAME1_TILE
	ld [mainCharacter_TileBL], a
	ld a, PLAYER_BR_FRAME1_TILE
	ld [mainCharacter_TileBR], a
	ld a, 0	; zero indexed
	ld [wPlayerAnimationCurrentFrame], a
	jp .EndOfFunc
	; load in the first set of tiles
.SecondFrame
	ld a, PLAYER_TL_FRAME2_TILE
	ld [mainCharacter_TileTL], a
	ld a, PLAYER_TR_FRAME2_TILE
	ld [mainCharacter_TileTR], a
	ld a, PLAYER_BL_FRAME2_TILE
	ld [mainCharacter_TileBL], a
	ld a, PLAYER_BR_FRAME2_TILE
	ld [mainCharacter_TileBR], a
	ld a, 1	; zero indexed
	ld [wPlayerAnimationCurrentFrame], a
	jp .EndOfFunc
	; load in the second set of tiles
.ThirdFrame
	ld a, PLAYER_TL_FRAME3_TILE
	ld [mainCharacter_TileTL], a
	ld a, PLAYER_TR_FRAME3_TILE
	ld [mainCharacter_TileTR], a
	ld a, PLAYER_BL_FRAME3_TILE
	ld [mainCharacter_TileBL], a
	ld a, PLAYER_BR_FRAME3_TILE
	ld [mainCharacter_TileBR], a
	ld a, 2	; zero indexed
	ld [wPlayerAnimationCurrentFrame], a
	jp .EndOfFunc
	; load in the second set of tiles
.EndOfFunc
EndOfAnimFunc:
	ret

; sets up all the flags and offsets to set up if we're flipped or not
UpdatePlayerFlipped:
	; do direction based checking 0 = N, 1 = E, 2 = S, 3 = W
	ld a, [mainCharacter_Direction]
	cp a, 1
	jp z, .EastFlip
	cp a, 3
	jp z, .WestFlip
	jp .FlipCheck

.EastFlip
	ld a, 1
	ld [wPlayerFlipped], a
	jp .FlipCheck

.WestFlip
	ld a, 0
	ld [wPlayerFlipped], a

	; now check if the player flipped tag is set
.FlipCheck
	ld a, [wPlayerFlipped]
	cp a, 1
	jp z, .FacingRight			; if flipped, then facing right, otherwise is facing left

.FacingLeft
	ld a, PLAYER_TL_NORM_X_OFFSET
	ld [wPlayerTileTLOffsetX], a
	ld a, PLAYER_TR_NORM_X_OFFSET
	ld [wPlayerTileTROffsetX], a
	ld a, PLAYER_BL_NORM_X_OFFSET
	ld [wPlayerTileBLOffsetX], a
	ld a, PLAYER_BR_NORM_X_OFFSET
	ld [wPlayerTileBROffsetX], a
	ld a, %00000000
	ld [mainCharacter_TileFlags], a
	jp .EndOfFunc
.FacingRight
	ld a, PLAYER_TL_FLIP_X_OFFSET
	ld [wPlayerTileTLOffsetX], a
	ld a, PLAYER_TR_FLIP_X_OFFSET
	ld [wPlayerTileTROffsetX], a
	ld a, PLAYER_BL_FLIP_X_OFFSET
	ld [wPlayerTileBLOffsetX], a
	ld a, PLAYER_BR_FLIP_X_OFFSET
	ld [wPlayerTileBROffsetX], a
	ld a, %00100000
	ld [mainCharacter_TileFlags], a
	jp .EndOfFunc

.EndOfFunc
	ret

; loads the background, tilemap and sets up the rest of the game field
; make sure the LCD is OFF
; @clobbers a, bc, de, hl
InitialiseLevel:

	call LoadLevelSpecificData
	call TileLoaderReset

	call InitialisePlayer
	call InitialiseBoxes
	call InitialiseCursor
	call InitialiseFont

	call SpawnBoxAtConveyor

	call SetBlankDMGPalette

	call InitialiseConveyorAnimation

	; do a single tick as part of loading
	call UpdateGameScene

	ret


; loads in anything that may change per level
; needs the LCD off
; @clobbers a, bc, de, hl
LoadLevelSpecificData:
	ld a, [wLevelSelected]

	cp a, 1
	jp z, .LevelOneLoad

	cp a, 2
	jp z, .LevelTwoLoad

	cp a, 3
	jp z, .LevelThreeLoad

.LevelOneLoad
	; Copy tile data into VRAM
	ld de, LevelOneTiles				; load the address of the tiles into the DE register
	ld hl, $9000				; load the beginning VRAM address into HL (HL is easy to inc/dec)
	ld bc, LevelOneTilesEnd - LevelOneTiles		; load the length of Tiles into BC
	call Memcopy				; call the memcopy subroutine

	; The above tile loading will clobber the tilemap in VRAM, but for now just load the other half of tiles

	; Copy tilemap data into VRAM (functionally identical to above but pointing to tilemap data and addresses)
	ld de, LevelOneTilemap
	ld hl, $9800
	ld bc, LevelOneTilemapEnd - LevelOneTilemap
	call Memcopy				; call the memcopy subroutine

	; first load the tiles and tilemap into VRAM
	; initialise the level specific variables
	ld a, DEFAULT_LEVELONE_BOXCOUNT
	ld [wLevelBoxCount], a

	ld a, DEFAULT_LEVELONE_FLAMMABLE_BOXCOUNT
	ld [wLevelFlammableBoxCount], a

	ld a, DEFAULT_LEVELONE_RADIOACTIVE_BOXCOUNT
	ld [wLevelRadioactiveBoxCount], a

	ld a, DEFAULT_LEVELONE_SCOREAREA_TOP_X
	ld [wLevelScoreAreaLeftX], a

	ld a, DEFAULT_LEVELONE_SCOREAREA_TOP_Y
	ld [wLevelScoreAreaTopY], a

	ld a, DEFAULT_LEVELONE_SCOREAREA_BOTTOM_X
	ld [wLevelScoreAreaRightX], a

	ld a, DEFAULT_LEVELONE_SCOREAREA_BOTTOM_Y
	ld [wLevelScoreAreaBottomY], a

	jp .EndOfFunc

.LevelTwoLoad	
	; Copy tile data into VRAM
	ld de, LevelTwoTiles				; load the address of the tiles into the DE register
	ld hl, $9000				; load the beginning VRAM address into HL (HL is easy to inc/dec)
	ld bc, LevelTwoTilesEnd - LevelTwoTiles		; load the length of Tiles into BC
	call Memcopy				; call the memcopy subroutine

	; The above tile loading will clobber the tilemap in VRAM, but for now just load the other half of tiles

	; Copy tilemap data into VRAM (functionally identical to above but pointing to tilemap data and addresses)
	ld de, LevelTwoTilemap
	ld hl, $9800
	ld bc, LevelTwoTilemapEnd - LevelTwoTilemap
	call Memcopy				; call the memcopy subroutine

	; first load the tiles and tilemap into VRAM
	; initialise the level specific variables
	ld a, DEFAULT_LEVELTWO_BOXCOUNT
	ld [wLevelBoxCount], a

	ld a, DEFAULT_LEVELTWO_FLAMMABLE_BOXCOUNT
	ld [wLevelFlammableBoxCount], a

	ld a, DEFAULT_LEVELTWO_RADIOACTIVE_BOXCOUNT
	ld [wLevelRadioactiveBoxCount], a

	ld a, DEFAULT_LEVELTWO_SCOREAREA_TOP_X
	ld [wLevelScoreAreaLeftX], a

	ld a, DEFAULT_LEVELTWO_SCOREAREA_TOP_Y
	ld [wLevelScoreAreaTopY], a

	ld a, DEFAULT_LEVELTWO_SCOREAREA_BOTTOM_X
	ld [wLevelScoreAreaRightX], a

	ld a, DEFAULT_LEVELTWO_SCOREAREA_BOTTOM_Y
	ld [wLevelScoreAreaBottomY], a

	jp .EndOfFunc
.LevelThreeLoad	
	; Copy tile data into VRAM
	ld de, LevelThreeTiles				; load the address of the tiles into the DE register
	ld hl, $9000				; load the beginning VRAM address into HL (HL is easy to inc/dec)
	ld bc, LevelThreeTilesEnd - LevelThreeTiles		; load the length of Tiles into BC
	call Memcopy				; call the memcopy subroutine

	; The above tile loading will clobber the tilemap in VRAM, but for now just load the other half of tiles

	; Copy tilemap data into VRAM (functionally identical to above but pointing to tilemap data and addresses)
	ld de, LevelThreeTilemap
	ld hl, $9800
	ld bc, LevelThreeTilemapEnd - LevelThreeTilemap
	call Memcopy				; call the memcopy subroutine

	; first load the tiles and tilemap into VRAM
	; initialise the level specific variables
	ld a, DEFAULT_LEVELTHREE_BOXCOUNT
	ld [wLevelBoxCount], a

	ld a, DEFAULT_LEVELTHREE_FLAMMABLE_BOXCOUNT
	ld [wLevelFlammableBoxCount], a

	ld a, DEFAULT_LEVELTHREE_RADIOACTIVE_BOXCOUNT
	ld [wLevelRadioactiveBoxCount], a

	ld a, DEFAULT_LEVELTHREE_SCOREAREA_TOP_X
	ld [wLevelScoreAreaLeftX], a

	ld a, DEFAULT_LEVELTHREE_SCOREAREA_TOP_Y
	ld [wLevelScoreAreaTopY], a

	ld a, DEFAULT_LEVELTHREE_SCOREAREA_BOTTOM_X
	ld [wLevelScoreAreaRightX], a

	ld a, DEFAULT_LEVELTHREE_SCOREAREA_BOTTOM_Y
	ld [wLevelScoreAreaBottomY], a

	jp .EndOfFunc

.EndOfFunc
	ret 


; call to load tiles and initialise variables
InitialiseCursor:
	; zero out everything
	ld a, 0
	ld [boxCursor_YPos], a
	ld [boxCursor_XPos], a
	ld [boxCursor_Tile], a
	ld [boxCursor_Enabled], a
	ld a, 0							; TODO: hardcoded OAM
	ld [boxCursor_OAMOffset], a		; set the OAM offset to be clear of the player character and box object

	ld de, CursorSpriteData
	ld bc, CursorSpriteDataEnd - CursorSpriteData
	ld a, 0
	call TileLoader

	ld a, c
	ld [boxCursor_Tile], a

	ret

; call each frame to make sure the cursor is placed in the correct spot
UpdateCursor:
	; get player XPos and YPos
	; calculate an offset from the top left to the intended position (adjust this with direction later)
	ld a, [mainCharacter_XPos]		; grab the X position of the player
	sub 8							; offset from the top left
	ld b, a							; stick it in B ready for any function calls
	ld a, [mainCharacter_YPos]		; do the same for Y
	add 8
	ld c, a
	
	call AdjustBoxOffsetForDirection

	push bc							; save the X/Y position
	call WaitNoMode3
	; call get tile by pixel to get the tile offset
	call GetTileAddressByPixel
	pop bc
	; check if the tile index is above 128, if so then 
	; check if there's a box in hand, or if there's a box in front of us
	call WaitNoMode3
	ld a, [hl]						; get the tile type at that address
	cp a, 127						; compare to 127, if > 127, carry bit will not be set
	jp nc, .EnableCursor				; if carry bit set, jump to pick up box

	ld a, [wBoxBeingHeld]
	cp a, 1
	jp nc, .EnableCursor
	
.DisableCursor
	ld a, 0
	ld [boxCursor_Enabled], a
	ld [boxCursor_XPos], a
	ld [boxCursor_YPos], a
	jp .EnabledCheck
.EnableCursor
	ld a, 1
	ld [boxCursor_Enabled], a
	ld a, b
	add 8							; do X position offset for OAM offset
	and %11111000					; mask it to snap it to the lowest align by 8 pixel position
	ld [boxCursor_XPos], a
	ld a, c
	add 16
	and %11111000
	ld [boxCursor_YPos], a
.EnabledCheck
	ld a, [boxCursor_Enabled]
	cp a, 0
	jp z, .DoNotDraw
	;call EnableCursor
	jp .FinishedDrawing
.DoNotDraw
	call DisableCursor
.FinishedDrawing
	ret 

; Loads the tiles needed for the conveyor animation in
InitialiseConveyorAnimation:
	; Copy tile data into VRAM
	ld de, ConveyorsSpriteData				; load the address of the tiles into the DE register
	ld hl, $97C0				; load the beginning VRAM address into HL (HL is easy to inc/dec)
	ld bc, ConveyorsSpriteDataEnd - ConveyorsSpriteData		; load the length of Tiles into BC
	call Memcopy				; call the memcopy subroutine

	ld a, 0
	ld [wFrameCounter], a

	ret

; a hardcoded animation update that takes specific X/Y tiles in the map and swaps them around
UpdateConveyoranimation:
	; check the frame counter
	ld a, [wFrameCounter]
	; if above CONVEYOR_BELT_ANIMATION_SPEED, reset it and swap the tiles
	cp a, CONVEYOR_BELT_ANIMATION_SPEED
	jp nc, .TimeToSwap
	inc a
	ld [wFrameCounter], a
	jp .EndOfFunc

.TimeToSwap
	ld a, 0
	ld [wFrameCounter], a

	call WaitNoMode3
	ld a, [$9900]			
	; check the tile ID and if it matches the first tile in the set
	cp a, 124
	jp z, .SecondFrame
.FirstFrame
	call WaitNoMode3
	ld hl, $9900
	ld [hl], 124
	inc hl
	ld [hl], 125
	jp .EndOfFunc
	; load in the first and second tiles
.SecondFrame
	call WaitNoMode3
	ld hl, $9900
	ld [hl], 126
	inc hl
	ld [hl], 127
	; load in the third and fourth tiles
.EndOfFunc
	ret

; Traverse the tilemap starting at the first address (top left X/Y)
; Store the current box count in D, current X loop position in b, Y in c
CheckForVictory:
	; check if there's any crates left to spawn
	; set a box count (making sure all are in the box)
	ld a, [wLevelBoxCount]
	ld d, a
	; set an address to the first index in the tile map
	ld a, [wLevelScoreAreaLeftX]
	ld b, a
	ld a, [wLevelScoreAreaTopY]
	ld c, a
	push bc
	call GetTileAddressByIndex
	;ld bc, 33								; lower the starting index by 32 and 1 to simplify both loops
	dec hl									; lower the starting index by 1 to simplify the X loop
	pop bc
	; make an Y loop
.LoopYStart
	ld a, [wLevelScoreAreaBottomY]
	inc a
	ld e, a
	ld a, c
	cp a, e		; offset it by 1 so we can check the zero flag
	jp z, .EndOfLoops
	push hl
		; make an X loop
.LoopXStart
		inc hl
		inc b
		; if tile is a box then check off box count
		call WaitNoMode3
		ld a, [hl]
		; check the tile type (if it's special or not)
		cp a, 127				; compare to 127, if > 127, carry bit will be not set
		jp c, .NoBoxThere		; if carry bit not set, there's already a box there, jump out
			dec d					; check a box off of the count
			ld e, a
			ld a, 0
			cp a, d
			jp nz, .NotAllBoxesPlaced
			;Otherwise all boxes placed
			ld a, 1
			ld [wVictoryFlagSet], a	; if all boxes are placed in the zone, then set the victory flag to true
									; will be set false if there's anything wrong later
.NotAllBoxesPlaced
			ld a, e					; restore the box type to the a register
			cp a, 129				; compare to tile number 129 
			jp c, .NoBoxThere		; if not Tile 129/130+ then it's a normal box and we don't have to check it
			; if it's special then check the adjacent addresses to see if they've got special boxes
			cp a, 129
			jp z, .FlammableBoxDetected
			cp a, 130
			jp z, .RadioactiveBoxDetected
.FlammableBoxDetected
			; set the "wrong" box type in the A register
			ld a, 130
			jp .AdjacentCheck
.RadioactiveBoxDetected
			ld a, 129
			jp .AdjacentCheck
.AdjacentCheck
			push hl
			ld e, a
			ld a, l
			sub 32
			ld l, a

			push bc
			push de
			; grab all the four positions tiles into register so we can spend no time in mode 0-1
			call WaitNoMode3
			ld b, [hl]
			ld a, l
			add 31
			ld l, a
			ld c, [hl]
			inc hl
			inc hl
			ld d, [hl]
			ld a, l
			add 31
			ld l, a
			ld e, [hl]
			; end of VRAM reads
			; load the 4 boxes to check into WRAM
			ld hl, wBoxTypeMemory					; grab the address for the memory and load it up

			ld [hl], b
			inc hl
			ld [hl], c
			inc hl
			ld [hl], d
			inc hl
			ld [hl], e

			; now we have all the box types in memory

			pop de									; grab the "wrong" box type off of the stack
			ld hl, wBoxTypeMemory
			ld a, [hli]								; iterate over the 4 bytes of memory to see if there's a fail condition
			cp a, e
			jp z, .WrongBox
			ld a, [hli]								
			cp a, e
			jp z, .WrongBox
			ld a, [hli]								
			cp a, e
			jp z, .WrongBox
			ld a, [hl]								
			cp a, e
			jp z, .WrongBox
			; else there's no wrong boxes
			
			jp .RestoreRegisters
.WrongBox
			ld a, 0									; load the no victory flag
			ld [wVictoryFlagSet], a
			jp .BreakLoop
.RestoreRegisters
			pop bc
			pop hl
.NoBoxThere
	ld a, [wLevelScoreAreaRightX]
	inc a
	ld e, a
	ld a, b
	cp a, e		; offset it by 1 so we can check the zero flag
	jp z, .LoopYIncrement
	jp .LoopXStart
	
.LoopYIncrement
	pop hl												; grab the memory address at the start of the last row
	inc c												; increase the counter
	ld a, [wLevelScoreAreaLeftX]						; reset the X counter
	ld b, a
	push bc
	ld bc, 32
	add hl, bc
	pop bc
	jp .LoopYStart
.BreakLoop
	pop bc
	pop hl
	pop hl
.EndOfLoops

	ret

; show the cursor
EnableCursor:
	; get OAM offset
	ld a,  [boxCursor_OAMOffset]	; get the offset from the start of OAM
	ld h, 0
	ld l, a								; load the offset into hl
	ld bc, $FE00						; start of OAM memory
	add hl, bc							; add the offset and the start of OAM together

	ld a, [boxCursor_YPos]				; update the OAM with the new position
	ld [hli], a
	ld a, [boxCursor_XPos]
	ld [hli], a

	ret

; hide the cursor
DisableCursor:
	ld a, 0
	ld [boxCursor_YPos], a
	ld [boxCursor_XPos], a

	ret

; call to load the tiles and initialise variables
InitialiseBoxes:
	ld a, 0
	ld [wBoxInPlay], a
	ld [wBoxBeingHeld], a
	ld [currentActiveBox_YPos], a
	ld [currentActiveBox_XPos], a
	ld a, 16										; TODO: hardcoded OAM
	ld [currentActiveBox_OAMOffset], a				; set the OAM offset to be clear of the player character

	ld de, BoxesSpriteData
	ld bc, BoxesSpriteDataEnd - BoxesSpriteData
	ld a, 1
	call TileLoader

	ld a, c
	ld [currentActiveBox_Tile], a

	; set the counters based on level
	; TODO: make this based on level
	ld a, [wLevelBoxCount]
	ld [wBoxesRemainingInLevel], a 
	ld a, [wLevelFlammableBoxCount]
	ld [wBoxesRemainingFlammable], a
	ld a, [wLevelRadioactiveBoxCount]
	ld [wBoxesRemainingRadioactive], a

	ret

; Spawns a box at the conveyor dropoff point
; @clobbers a, bc, d, hl
SpawnBoxAtConveyor:
	; check to see if there's already a box there
	ld b, 16						; load the X/Y values to get the address
	ld c, 64
	call GetTileAddressByPixel
	call WaitNoMode3
	
	ld a, [hl]						; get the tile type at that address
	cp a, 127						; compare to 127, if > 127, carry bit will be not set
	jp nc, .EndOfFunc				; if carry bit not set, there's already a box there, jump out

	; check to see if we have any boxes remaining to spawn
	ld a, [wBoxesRemainingInLevel]
	cp a, 0
	jp z, .EndOfFunc

	; otherwise, continue
	call RandomNumberFour			; get a number between 0-2, this gives 0-3
	ld b, a							; store the random number in b to keep a safe copy
	cp a, 3							; if we get a 3
	jp nz, .CounterChecks			
	dec a							; just decrement it
	ld b, a
.CounterChecks
	; first we check if the sum of the special box types match the total
	; if so, we just set the temp box type to the "highest" special box type, so that it
	; "falls down" into the special types to ensure no issues
	ld a, [wBoxesRemainingFlammable]
	ld c, a
	ld a, [wBoxesRemainingRadioactive]
	add c							; grab both box types and then sum them together
	ld c, a 							; put the result into c
	ld a, [wBoxesRemainingInLevel]	; compare the sum to the total remaining
	cp a, c
	jp nz, .RadioactiveCheck		; if these don't match, then just skip to normal checking
									; otherwise there's only exactly enough boxes to spawn the rest of the special boxes
	ld b, 2							; set the special box type to the "highest" available

	; here we check to see if there's any boxes of the type selected available, if there isn't,
	; move on to the next type
	; if radioactive check if radioactive left
.RadioactiveCheck
	ld a, b							; load b (temp random number) into a
	cp a, 2
	jp nz, .FlammableCheck			; if there is, check if it's not zero
	ld a, [wBoxesRemainingRadioactive]
	cp a, 0
	jp z, .NoRadioactiveLeft		; if there's no radioactive boxes left in the counter, jump
	dec a
	ld [wBoxesRemainingRadioactive], a
	jp .OffsetCalc					; we're spawning a radioactive box
.NoRadioactiveLeft
	ld b, 1							; switch the temp box to flammable
.FlammableCheck
	; if flammable check if flammable left
	ld a, b							; load b (temp random number) into a
	cp a, 1
	jp nz, .OffsetCalc				; if there is, check there's more than zero remaining
	ld a, [wBoxesRemainingFlammable]
	cp a, 0
	jp z, .NoFlammableLeft			; if there's no flammables left, jump
	dec a
	ld [wBoxesRemainingFlammable], a
	jp .OffsetCalc					; we're spawning a flammable box
.NoFlammableLeft
	ld b, 0							; just do a normal crate
.OffsetCalc
	ld a, b							; retrieve the temp variable for random box type (may have been modified by box checks)
	add 128
	ld d, a

	ld b, 16
	ld c, 64

	call GetTileAddressByPixel

	call WaitNoMode3

	ld a, d
	ld [hl], a

	; we've spawned a box, lower the counter
	ld a, [wBoxesRemainingInLevel]
	dec a
	ld [wBoxesRemainingInLevel], a
.EndOfFunc
	ret

; spawn an object version of a box
; @params b: type of box to spawn
; @clobbers a, hl
SpawnBoxObject:
	ld a, b
	ld [currentActiveBox_Tile], a
	ld a, 0
	ld [currentActiveBox_YPos], a
	ld [currentActiveBox_XPos], a

	ld a, 1
	ld [wBoxInPlay], a
	ld a, 0
	ld [wBoxBeingHeld], a

	ret

; disables the current box
; @clobbers a
DisableBox:	
	ld a, 0
	ld [currentActiveBox_XPos], a
	ld [currentActiveBox_YPos], a
	ld [wBoxInPlay], a
	ld [wBoxBeingHeld], a
	ret

; Places a box held by the player into the background
; @clobbers a, bc, hl
PlaceBox:
	; get player XPos and YPos
	; calculate an offset from the top left to the intended position (adjust this with direction later)
	ld a, [mainCharacter_XPos]		; grab the X position of the player
	sub 8							; offset from the top left
	ld b, a							; stick it in B ready for any function calls
	ld a, [mainCharacter_YPos]		; do the same for Y
	add 8
	ld c, a

	call AdjustBoxOffsetForDirection

	; call get tile by pixel to get the tile offset
	call GetTileAddressByPixel
    call WaitNoMode3
	; check if the tile index is above 128, if so then 
	; check if there's a box in hand, if not, then pick up the box instead
	ld a, [hl]						; get the tile type at that address
	cp a, 127						; compare to 127, if > 127, carry bit will not be set
	jp nc, .PickUpBox				; if carry bit not set, jump to pick up box
.PutDownBox
	; check if we're holding a box, if we're not, then don't put anything down
	ld a, [wBoxBeingHeld]
	cp a, 0
	; TODO: play a sound
	jp z, .EndOfFunc
	; replace the tile index with the box tile to place down the box
	call WaitNoMode3
	ld a, [currentActiveBox_Tile]
	ld [hl], a
	; call disable box to remove the object box
	call DisableBox
	ld a, 0
	ld [wBoxBeingHeld], a			; box is no longer in hands
	; play a drop item sound
	ld hl, sound_item_drop
	call PlaySoundHL
	jp .EndOfFunc
	; otherwise if there is a box already there
.PickUpBox
	; check if box is in hand, if not then pick it up, otherwise play an error
	ld a, [wBoxBeingHeld]
	cp a, 0
	; TODO: play a sound
	jp nz, .EndOfFunc

    call WaitNoMode3
	; find the original tile in the ROM and replace that tile
	ld a, [hl]
	ld b, a
	push bc
	push hl							; save the VRAM tilemap address
	ld a, [wLevelSelected]
	ld c, a
	call ConvertVRAMTileMapToROMTileMap	; grab the original tilemap from ROM
	ld a, [hl]						; get the original tile underneath
	pop hl							; grab the VRAM tilemap address again
	call WaitNoMode3
	ld [hl], a

	; spawn the box and put in the players hand
	pop bc
	call SpawnBoxObject
	ld a, 1
	ld [wBoxBeingHeld], a
	ld [wBoxInPlay], a
	call SpawnBoxAtConveyor			; place down another box at the conveyor (validity checks are done in function)

	; play a pickup item sound
	ld hl, sound_item_grab
	call PlaySoundHL

.EndOfFunc
	ret

; Updates the currently active box (assuming there is one)
; @clobbers a, hl
UpdateBox:
	; grab the status of the box (is there one active?)
	ld a, [wBoxInPlay]				; check if wBoxInPlay == true
	cp a, 1
	jp nz, .NoBox					; if == false then just skip function
	; check the player direction and select an offset
	; TODO: actually implement direction
	; update in the in RAM positions
	ld a, [wBoxBeingHeld]				; check if wBoxBeingHeld == true
	cp a, 1
	jp nz, .BoxNotBeingHeld			; if == false then just skip to pickup checking

	ld a, [mainCharacter_XPos]
	add 16
	ld [currentActiveBox_XPos], a
	ld a, [mainCharacter_YPos]
	add 24
	ld [currentActiveBox_YPos], a

.BoxNotBeingHeld
	; check to see if player is within 8 pixels in both axis
	; TODO: Implement
.NoBox
	ret

; draw function where we actually update the OAM's to make sure it's at the start of the frame
DrawPlayer:
.UpdateOAM
	; get OAM offset
	ld a,  [mainCharacter_OAMOffset]	; get the offset from the start of OAM
	ld h, 0
	ld l, a								; load the offset into hl
	ld bc, $FE00						; start of OAM memory
	add hl, bc							; add the offset and the start of OAM together

	call WaitNoMode3

	ld a, [mainCharacter_YPos]
	add 16							; make sure to set the sprite offsets
	ld [hli], a
	ld a, [mainCharacter_XPos]
	ld b, a
	ld a, [wPlayerTileTLOffsetX]
	add b							; make sure to set the sprite offsets
	ld [hli], a
	ld a, [mainCharacter_TileTL]
	ld [hli], a
	ld a, [mainCharacter_TileFlags]
	ld [hli], a

	ld a, [mainCharacter_YPos]
	add 16							; then add the metasprite offset							; make sure to set the sprite offsets
	ld [hli], a
	ld a, [mainCharacter_XPos]
	ld b, a
	ld a, [wPlayerTileTROffsetX]
	add b							; make sure to set the sprite offsets
	ld [hli], a
	ld a, [mainCharacter_TileTR]
	ld [hli], a
	ld a, [mainCharacter_TileFlags]
	ld [hli], a

	ld a, [mainCharacter_YPos]
	add 16							; make sure to set the sprite offsets
	add 8							; then add the metasprite offset
	ld [hli], a
	ld a, [mainCharacter_XPos]
	ld b, a
	ld a, [wPlayerTileBLOffsetX]
	add b							; make sure to set the sprite offsets
	ld [hli], a
	ld a, [mainCharacter_TileBL]
	ld [hli], a
	ld a, [mainCharacter_TileFlags]
	ld [hli], a

	ld a, [mainCharacter_YPos]
	add 16							; make sure to set the sprite offsets
	add 8							; then add the metasprite offset
	ld [hli], a
	ld a, [mainCharacter_XPos]
	ld b, a
	ld a, [wPlayerTileBROffsetX]
	add b							; make sure to set the sprite offsets
	ld [hli], a
	ld a, [mainCharacter_TileBR]
	ld [hli], a
	ld a, [mainCharacter_TileFlags]
	ld [hli], a
.UpdateOAMFinished
	ret
; draw function for the box object
DrawBoxObject:
	; initialise the OAM
	ld hl, _OAMRAM				; grab the address for the top of the OAM
	ld a, [currentActiveBox_OAMOffset]
	ld b, a
	ld a, l
	add b	; add the offset
	ld l, a

    call WaitNoMode3

	ld a, [currentActiveBox_YPos]
	ld [hli], a
	ld a, [currentActiveBox_XPos]
	ld [hli], a
	ld a, [currentActiveBox_Tile]
	ld [hli], a
	ld a, %00000000
	ld [hli], a

	ret

; Cursor Draw function
DrawCursorObject:
	ld a, [boxCursor_XPos]				; load the position into bc
	ld d, a
	ld a, [boxCursor_YPos]
	ld e, a
	ld a, [boxCursor_Enabled]
	cp a, 1
	jp z, .StartDrawing
	ld d, 0								; if it's not supposed to be enabled just set the positions to 0
	ld e, 0
.StartDrawing
	call WaitNoMode3
	; initialise the OAM
	ld hl, _OAMRAM				; grab the address for the top of the OAM
	ld a, [boxCursor_OAMOffset]
	ld b, a
	ld a, l
	add b	; add the offset
	ld l, a

	call WaitNoMode3

	ld a, [boxCursor_YPos]
	ld [hl], e
	inc hl
	ld a, [boxCursor_XPos]
	ld [hl], d
	inc hl
	ld a, [boxCursor_Tile]
	ld [hli], a
	ld a, %00000000
	ld [hli], a
.EndOfFunc
	ret

; big game scene update function, put here to make it easier to separate out all the different scene types
UpdateGameScene:
call DrawBoxObject
call DrawCursorObject
call DrawPlayer

call CheckForVictory

; DEBUG update boxes remaining

	ld a, [wBoxesRemainingInLevel]
	ld hl, wNumberStringData
	call NumberToString

	ld hl, wNumberStringData
	ld de, $9800 + $12
	call DrawTextTilesLoop

call UpdateConveyoranimation
call UpdateButtonDebounce
call UpdateKeys
call UpdatePlayer
call UpdateBox
call UpdateCursor

ld a, [wVictoryFlagSet]
cp a, 1
jp nz, .CheckBoxSpawn
	ld hl, wVictoryString
	ld de, $98A3
	call DrawTextTilesLoop
	
	ld a, [wLevelSelected]						; increase the level counter
	inc a
	ld [wLevelSelected], a

	ld a, 1										; go back to cutscene after level is over
	ld [wCurrentScene], a

	ld hl, sound_speedup_round
	call PlaySoundHL

	; give a 2 second wait time
	ld b, 120
	call WaitFrames

	call FadeToWhite

	call ReloadGame

.CheckBoxSpawn
	ld a, [wCurKeys]
	and a, PADF_A
	jp nz, .BoxSpawn

	;ld a, [wCurKeys]
	;and a, PADF_B
	;jp nz, .BoxDespawn

	jp .EndOfFunc

.BoxSpawn
	; check if Debounce done
	call CheckButtonDebounce				; check the button debounce status
	cp a, 1									; if a == 1, zero flag will set
	jp z, .EndOfFunc				; skip the button
	call SetButtonDebounce					; otherwise debounce is clear and we can press the button
	call PlaceBox							; make sure to set debounce ourselves
	jp .EndOfFunc
.BoxDespawn
	;call DisableBox
.EndOfFunc
	ret