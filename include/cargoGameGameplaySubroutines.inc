SECTION "Gameplay Subroutines", ROM0

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;	GAME SPECIFIC SUBROUTINE
;;	BLOCK
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

InitialisePlayer:
	; initialise player variables and load/define the tiles needed
	ld a, 32
	ld [mainCharacter_XPos], a
	ld [mainCharacter_XDestination], a
	ld a, 32
	ld [mainCharacter_YPos], a
	ld [mainCharacter_YDestination], a
	ld a, 0
	ld [mainCharacter_Direction], a
	ld a, 128
	ld [mainCharacter_OAMOffset], a
	ld a, 1
	ld [mainCharacter_AllowMove], a

	ld de, PlayerSpriteData
	ld bc, PlayerSpriteDataEnd - PlayerSpriteData
	ld a, 0

	call TileLoader

	; for now, main character tiles are upper is $+2 and lower is $+7
	ld a, c
	ld [mainCharacter_TileTL],a
	inc a
	ld [mainCharacter_TileTR], a
	inc a
	ld [mainCharacter_TileBL],a
	inc a
	ld [mainCharacter_TileBR], a

	; get OAM offset
	ld a,  [mainCharacter_OAMOffset]	; get the offset from the start of OAM
	ld h, 0
	ld l, a								; load the offset into hl
	ld bc, $FE00						; start of OAM memory
	add hl, bc							; add the offset and the start of OAM together

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
; 	snap to a grid tests
;	ld a, [mainCharacter_XPos]
;	and %11111000
;	ld [mainCharacter_XPos], a

;	ld a, [mainCharacter_YPos]
;	and %11111000
;	ld [mainCharacter_YPos], a

.UpdateOAM
	; get OAM offset
	ld a,  [mainCharacter_OAMOffset]	; get the offset from the start of OAM
	ld h, 0
	ld l, a								; load the offset into hl
	ld bc, $FE00						; start of OAM memory
	add hl, bc							; add the offset and the start of OAM together

	ld a, [mainCharacter_YPos]
	add 16							; make sure to set the sprite offsets
	ld [hli], a
	ld a, [mainCharacter_XPos]
	add 8							; make sure to set the sprite offsets
	ld [hli], a
	
	inc hl							; skip two memory locations as we're only updating positions
	inc hl

	ld a, [mainCharacter_YPos]
	add 16							; then add the metasprite offset							; make sure to set the sprite offsets
	ld [hli], a
	ld a, [mainCharacter_XPos]
	add 8							; make sure to set the sprite offsets
	add 8							; then add the metasprite offset
	ld [hli], a
		
	inc hl							; skip two memory locations as we're only updating positions
	inc hl

	ld a, [mainCharacter_YPos]
	add 16							; make sure to set the sprite offsets
	add 8							; then add the metasprite offset
	ld [hli], a
	ld a, [mainCharacter_XPos]
	add 8							; make sure to set the sprite offsets
	ld [hli], a
		
	inc hl							; skip two memory locations as we're only updating positions
	inc hl

	ld a, [mainCharacter_YPos]
	add 16							; make sure to set the sprite offsets
	add 8							; then add the metasprite offset
	ld [hli], a
	ld a, [mainCharacter_XPos]
	add 8							; make sure to set the sprite offsets
	add 8							; then add the metasprite offset
	ld [hli], a
.UpdateOAMFinished

	; screen scroll tests
	;ld a, [mainCharacter_YPos]
	;ld [rSCY], a
	;ld a, [mainCharacter_XPos]
	;ld [rSCX], a
	ret

; call to load tiles and initialise variables
InitialiseCursor:
	; zero out everything
	ld a, 0
	ld [boxCursor_YPos], a
	ld [boxCursor_XPos], a
	ld [boxCursor_Tile], a
	ld [boxCursor_Enabled], a
	ld a, 20					; TODO: hardcoded OAM
	ld [boxCursor_OAMOffset], a	; set the OAM offset to be clear of the player character and box object

	ld de, CursorSpriteData
	ld bc, CursorSpriteDataEnd - CursorSpriteData
	ld a, 0
	call TileLoader

	ld a, c
	ld [boxCursor_Tile], a

	; initialise the OAM
	ld hl, _OAMRAM				; grab the address for the top of the OAM
	ld a, [boxCursor_OAMOffset]
	ld b, a
	ld a, l
	add b	; add the offset
	ld l, a

	ld a, [boxCursor_YPos]
	ld [hli], a
	ld a, [boxCursor_XPos]
	ld [hli], a
	ld a, [boxCursor_Tile]
	ld [hli], a
	ld a, %00000000
	ld [hli], a

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
	; call get tile by pixel to get the tile offset
	call GetTileAddressByPixel
	pop bc
	; check if the tile index is above 128, if so then 
	; check if there's a box in hand, or if there's a box in front of us
	ld a, [hl]						; get the tile type at that address
	cp a, 127						; compare to 127, if > 127, carry bit will not be set
	jp nc, .EnableCursor				; if carry bit set, jump to pick up box

	ld a, [wBoxBeingHeld]
	cp a, 1
	jp nc, .EnableCursor
	
.DisableCursor
	ld a, 0
	ld [boxCursor_Enabled], a
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
	call EnableCursor
	jp .FinishedDrawing
.DoNotDraw
	call DisableCursor
.FinishedDrawing
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
	; get OAM offset
	ld a,  [boxCursor_OAMOffset]	; get the offset from the start of OAM
	ld h, 0
	ld l, a								; load the offset into hl
	ld bc, $FE00						; start of OAM memory
	add hl, bc							; add the offset and the start of OAM together
	
	ld a, 0								; update the OAM with the disabled/offscreen position
	ld [hli], a
	ld [hli], a

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
	ld a, DEFAULT_LEVELONE_BOXCOUNT
	ld [wBoxesRemainingInLevel], a 
	ld a, DEFAULT_LEVELONE_FLAMMABLE_BOXCOUNT
	ld [wBoxesRemainingFlammable], a
	ld a, DEFAULT_LEVELONE_RADIOACTIVE_BOXCOUNT
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

; disables the current box
DisableBox:
	; initialise the OAM
	ld hl, _OAMRAM				; grab the address for the top of the OAM
	ld a, [currentActiveBox_OAMOffset]
	ld b, a
	ld a, l
	add b	; add the offset
	ld l, a

    call WaitNoMode3

	ld a, 0
	ld [hli], a
	ld [hli], a
	ld [hli], a
	ld [hli], a

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
	call UpdateBox					; immediately update it to not have the one frame delay
	call SpawnBoxAtConveyor			; place down another box at the conveyor (validity checks are done in function)

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

	; Update the OAM, first calculate the offset needed
	ld hl, _OAMRAM				; grab the address for the top of the OAM
	ld a, [currentActiveBox_OAMOffset]
	ld b, a
	ld a, l
	add b	; add the offset
	ld l, a

	ld a, [currentActiveBox_YPos]
	ld [hli], a
	ld a, [currentActiveBox_XPos]
	ld [hli], a

.BoxNotBeingHeld
	; check to see if player is within 8 pixels in both axis
	; TODO: Implement
.NoBox
	ret
