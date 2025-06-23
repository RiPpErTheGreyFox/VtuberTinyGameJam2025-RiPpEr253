INCLUDE "include/hardware.inc"			; include all the defines
INCLUDE "include/cargoGameStructs.inc"	; out of file definitions

; graphical defines
DEF PLAYER_TOP_OAM EQU _OAMRAM


; Gameplay Constants
DEF DEFAULT_ITEMS_REMAINING EQU %00010000	; 10 in BCD
DEF DEFAULT_DEBOUNCE_FRAMES EQU 30
; gameplay definitions

SECTION "Counter", WRAM0
wFrameCounter: db
wButtonDebounce: db

SECTION "Input Variables", WRAM0		; set labels in Work RAM for easy variable use
wCurKeys: db							; label: declare byte, reserves a byte for use later
wNewKeys: db

SECTION "Item Data", WRAM0
wItemMomentumX: db

SECTION "Gameplay Data", WRAM0
wBoxInPlay: db							; treat as a bool
wBoxBeingHeld: db						; bool
wBoxTileIndex: db						; starting tile index for the box graphics
	dstruct PLAYER, mainCharacter
	dstruct BOX, currentActiveBox

SECTION "Animation Data", WRAM0
wPlayerCurrentFrame: db

SECTION "Managed Variables", WRAM0
wTileBankZero: dw						; variables that hold the current count of tiles loaded by the manager
wTileBankOne: dw
wTileBankTwo: dw

; System definitions
SECTION "System Type", WRAM0
wCGB: db
wAGB: db
wSGB: db
	
SECTION "Random Seed", WRAM0
wSeed0: db
wSeed1: db
wSeed3: db

; Jump table for interrupts

SECTION "StatInterrupt", ROM0[$0048]
	jp ScanlineInterruptHandler

SECTION "Header", ROM0[$100]

	jp EntryPoint

	ds $150 - @, 0 ; Make room for the header

; Subroutines

; Copy bytes from one are to another.
; @param de: Source
; @param hl: Destination
; @param bc: Length
; @return hl: current memory address
Memcopy:
	ld a, [de]					; load one byte from [DE] into the A register
	ld [hli], a					; place the byte from A into the address at HL and increment it
	inc de						; increment the address stored in DE
	dec bc						; decrement the counter in BC
	ld a, b						; load register B's data into the Accumulator
	or a, c						; OR it with C to check if both bytes are 0
	jp nz, Memcopy				; loop if not zero

	ret

; Waits until out of mode 3
WaitNoMode3:
	push hl						; save the desination address to the stack as HL gets clobbered

	; check if we're in mode 0 or 1 (will be true once we're out of mode 3)
	ld   hl, rSTAT     			; check STAT Register
	.wait
	bit  1, [hl]       			; Wait until Mode is 0 or 1
	jr   nz, .wait

	pop hl

	ret

; Generate a random number and return it.
; @clobbers: hl
; @return a: random number between 0-255
RandomNumber:
	ld hl, wSeed0
	ld a, [hli]
	sra a
	sra a
	sra a
	xor [hl]
	inc hl
	rra 
	rl [hl]
	dec hl
	rl [hl]
	dec hl
	rl [hl]
	ld a, [$fff4]				; get divider register to increase randomness
	add [hl]
	ret 

; Calls RandomNumber and gets an ouput of between 0-3
; @clobbers: b,hl
; @return a: random number between 0-3
RandomNumberFour:
	call RandomNumber			; get random number between 0-255

	ld b, %00000011				; load $03 into register for bitwise and
	and b						; and $03 with a to discard top 6 bits

	ret

; Wait for specified number of frames
; @param b: Number of frames to wait + 1
; @clobbers: a
WaitFrames:
	dec b
.WaitNotVBlank
	ld a, [rLY]			; loads into the A register, the current scanline (rLY)
	cp 144				; compares the value in the A register, against 144
	jp nc, .WaitNotVBlank; jump if carry not set (if a > 144)
.WaitVBlank
	ld a, [rLY]			; loads into the A register, the current scanline (rLY)
	cp 144				; compares the value in the A register, against 144
	jp c, .WaitVBlank	; jump if carry set (if a < 144)
	ld a, b
	cp 0
	jp nz, WaitFrames
	; above is waiting for a full complete frame
	ret

; Called on startup to determine the type of gameboy program is running on
; @clobbers: af, bc, de, hl
SystemDetection:
	; zero detection result variables
	push af
	ld a, 0
	ld [wCGB], a
	ld [wSGB], a
	ld [wAGB], a
	pop af

	; first detect CGB
	cp a, $11
	jp nz, .SGBDetect
	ld a, 1
	ld [wCGB], a
	; detect if AGB
.AGBDetect
	ld a, b
	cp a, $01
	jp nz, .SGBDetect
	ld a, 1
	ld [wAGB], a
	; detect if SGB
.SGBDetect
	ld a, c
	cp a, $14
	jp nz, .EndDetect
	ld a, 1
	ld [wSGB], a

.EndDetect
	ret

; Initialise Tile Loader
; resets the tile pointers to defaults
; @clobbers a, de
TileLoaderReset:
	ld a, $80
	ld [wTileBankZero], a
	ld a, $00
	ld [wTileBankZero + 1], a
	ld a, $88
	ld [wTileBankOne], a
	ld a, $00
	ld [wTileBankOne + 1], a
	ld a, $90	
	ld [wTileBankTwo], a
	ld a, $00
	ld [wTileBankTwo + 1], a

	ret

; Sprite loader for managing tile locations
; TODO: doesn't handle loading too many tiles (writes to out of bounds memory)
; TODO: out of bounds memory checking and failing a load
; TODO: handle tiles 128+ for bank 2
; @param: de: address of the tiles to load
; @param: bc: the length of tile data to load
; @param: a: the desired location of the tiles 0 = object, 1 = both, 2 = background
; @return: bc: index of the first tile loaded via this function
TileLoader:
	; de and bc are already set up before this subroutine for Memcopy to be called directly
	; ld de, TitleScreenTiles				; load the address of the tiles into the DE register
	; ld bc, TitleScreenTilesEnd - TitleScreenTiles		; load the length of Tiles into BC

	; check a for which flag
	; if 0, memory address $8000
	cp a, 0
	jp z, .bank0
	; if 1, memory address $8800
	cp a, 1
	jp z, .bank1
	; if 2, memory address $9000
	cp a, 2
	jp z, .bank2

.bank0
	ld a, [wTileBankZero]					; Get the MSB of the tile bank's first free tile address
	ld h, a									; load it into the H register
	ld a, [wTileBankZero + 1]				; now the LSB
	ld l, a									; into the L register
	push hl									; store the current address onto the stack
	call Memcopy							; call the memcopy subroutine
	ld a, h									
	ld [wTileBankZero], a					; store the address of the next free tile
	ld a, l									
	ld [wTileBankZero + 1], a				
	; get the beginning address (HL) - the base address, divide by 16 and that should give the tile indicies
	pop hl									; grab the first address off of the stack
	ld a, h									; do a 16-bit subtraction and store the result in BC
	sub $80									
	ld b, a									
	ld a, l									
	sub $00									
	ld c, a									
	jp .DivideAndReturn						; jumpt to the divide function

.bank1
	ld a, [wTileBankOne]
	ld h, a
	ld a, [wTileBankOne + 1]
	ld l, a
	push hl
	call Memcopy				; call the memcopy subroutine
	ld a, h
	ld [wTileBankOne], a
	ld a, l
	ld [wTileBankOne + 1], a
	; get the beginning address (HL) - the base address, divide by 16 and that should give the tile indicies
	pop hl
	ld a, h
	sub $80
	ld b, a
	ld a, l
	sub $00
	ld c, a
	jp .DivideAndReturn

.bank2
	ld a, [wTileBankTwo]
	ld h, a
	ld a, [wTileBankTwo + 1]
	ld l, a
	push hl
	call Memcopy				; call the memcopy subroutine
	ld a, h
	ld [wTileBankTwo], a
	ld a, l
	ld [wTileBankTwo + 1], a
	
	; get the beginning address (HL) - the base address, divide by 16 and that should give the tile indicies
	pop hl
	ld a, h
	sub $90
	ld b, a
	ld a, l
	sub $00
	ld c, a

.DivideAndReturn
	srl b
  	rr c
  	srl b
  	rr c
  	srl b
  	rr c
  	srl b
  	rr c
	ret

; Interrupt handler for LCD Stat interrupt
; @touches: af, bc, de, hl
ScanlineInterruptHandler:
	; entry point of STAT Interrupt Handler
	
	; save current registers
	push af
	push bc
	;push de
	;push hl

	; check if LYC == LY
	ld a, [rLYC]
	ld b, a
	ld a, [rLY]
	cp a, b
	; otherwise skip rest of interrupt
	jp nz, .EndOfInterrupt

	call WaitNoMode3
	
	; scroll the screen by player X position
	ld a, [PLAYER_TOP_OAM + 1]
	srl a
	srl a
	srl a
	sub 10
	ld b, a

	; get Scroll X register
	ld a, [rSCX]
	; scroll the screen by fixed amount
	; add 1

	add b

	; set Scroll X register
	ld [rSCX], a

	; next line
	ld a, [rLYC]
	add 2
	ld [rLYC], a

	; if LYC > 144 reset to zero
	cp 144
	jp c, .EndOfInterrupt
	ld a, 20
	ld [rLYC], a

	;reset scroll to 0
	ld a, 0
	ld [rSCX], a

.EndOfInterrupt
	; restore previous registers
	;pop hl
	;pop de
	pop bc
	pop af

	reti

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;	SOUND SUBROUTINE
;;	BLOCK
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Disable the sound hardware to save battery life
; @clobbers: a
DisableSound:
	ld a, $00
	ld [$FF26], a					; load $00 into NR52: Audio Master Control
	ld [$FF25], a
	ld [$FF24], a
	ret

; Enables the sound device, and also sets some sane defaults (full volume, all channels active)
; @clobbers: a
EnableSound:
	ld a, %10001111				; load $8F into NR52 to enable master audio (F is needed for some emulators)
	ld [$FF26], a					; this enables power to the APU (must be done first)

	ld a, %11111111				; pans all four channels to both left and right speakers
	ld [$FF25], a					; load into register NR51: Sound panning

	ld a, %01110111				; maximum volume for left and right channels, but mute VIN sources
	ld [$FF24], a					; load into register NR50: Mater Volume and VIN panning

	ret

; Example pulse sound with channel 1 settings
; sound_pulse_example: ; name the sound with a label
;	DW $FF10		; SOUND_CH1_START ; specifiy which channel 
;	DB %00001000	; Bit 7: unused, 6-4: sweep time, 3: sweep freq +/-, 2-0: sweep shifts
;	DB %01000100	; Bits 7-6: wave duty, 5-0: length of sound data (counting up to 64 at 256hz)
;	DB %11110000	; Bits 7-4: envelope start, 3: envelope +/-, 2-0: num. sweeps
;	DB %01000000	; Bits 7-0: lower 8 bits of the sound frequency
;	DB %11000000	; Bit 7: trigger, 6: use length, 5-3: unused, 2-0 highest 3 bits of frequency

; Plays a sound by reading table data starting at address hl.
; Expects memory address of channel and 5 bytes with attributes
; @clobbers: a, hl
PlaySoundHL:
	push de

	; Read channel start into DE
	ld a, [hli]
	ld e, a
	ld a, [hli]
	ld d, a

	; Read data from the passed in table and feed into the channel start
	ld a, [hli]
	ld [de], a
	inc de
	
	ld a, [hli]
	ld [de], a
	inc de

	ld a, [hli]
	ld [de], a
	inc de

	ld a, [hli]
	ld [de], a
	inc de

	ld a, [hl]
	ld [de], a

	pop de
	ret

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;	END SOUND SUBROUTINE
;;	BLOCK
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

; Button debounce routines

; Sets the button debounce timer to the default defined
; @clobbers: a
SetButtonDebounce:
	ld a, DEFAULT_DEBOUNCE_FRAMES
	ld [wButtonDebounce], a
	ret
; Returns a = 1 if the debounce isn't cleared
; @return a: 1 if debounce still happening, otherwise 0
CheckButtonDebounce:
	ld a, [wButtonDebounce]				; grab the current debounce timer
	cp a, 0								; check if it's not zero
	jp nz, .AboveZero					; if it is, zero bit is not set, so jump
	ld a, 0								; return 0 as debounce isn't cleared
	ret
.AboveZero
	ld a, 1								; return 1 as debounce is cleared
	ret
; processes a single frame of debounce
; @clobbers a
UpdateButtonDebounce:
	ld a, [wButtonDebounce]				; grab the current debounce timer
	cp a, 0								; if it's at zero, then just ignore it
	jp z, .DebounceCleared

	dec a								; decrement and save
	ld [wButtonDebounce], a

.DebounceCleared
	ret

; Convert a pixel position to a tilemap address
; hl = $9800 + X + Y * 32
; @param b: X
; @param c: Y
; @return hl: tile address
GetTileAddressByPixel:
	; First, we need to divide by 8 to convert a pixel position to a tile position.
	; After this we want to multiply the Y position by 32.
	; These operations effectively cancel out so we only need to mask the Y value.
	ld a, c
	and a, %11111000
	ld l, a
	ld h, 0
	; Now we have the position * 8 in hl
	add hl, hl ; position * 16
	add hl, hl ; position * 32
	; Convert the X position to an offset
	ld a, b
	srl a ; a / 2
	srl a ; a / 4
	srl a ; a / 8
	; Add the two offsets together
	add a, l
	ld l, a
	adc a, h
	sub a, l
	ld h, a
	; Add the offset to the tilemap's base address and we're done!
	ld bc, $9800
	add hl, bc
	ret

; Convert a pixel position to a tilemap index
; @param b: X
; @param c: Y
; @return b, c: tile X, tile Y
; @clobbers a
GetTileIndexByPixel:
	; just get the pixel location in both axis and divide it by 8
	ld a, c			; get the Y position
	srl a ; a / 2 
	srl a ; a / 4
	srl a ; a / 8
	ld c, a
	ld a, b			; get the X position
	srl a ;a / 2
	srl a ;a / 4
	srl a ;a / 8
	ld b, a

	ret

; Take a TileMap address, and convert it to the correct address for the "same" tile but in the ROM
; used to get the original tilemap in case it was changed
; @param c: LevelNumber
; @param hl: address to be converted
; @return hl: converted address
; @clobbers b
ConvertVRAMTileMapToROMTileMap:
	; remove the offset to the original TileMap
	ld a, h
	sub $98								; turns the base address from $9800+ to $0000+
	ld h, a

	;check which level number it is and adjust the offset accordingly
	ld a, c
	cp a, 1
	jp z, .TileMapOne
	; TODO: if we have more than one level, add more conditionals
.TileMapOne
	ld bc, LevelOneTilemap
	add hl, bc
	ret

; Updates the controller variables
UpdateKeys:
	; Poll half the controller
	ld a, P1F_GET_BTN
	call .onenibble
	ld b, a ; B7-4 = 1; B3-0 = unpressed buttons

	; Poll the other half
	ld a, P1F_GET_DPAD
	call .onenibble
	swap a ; A3-0 = unpressed directions; A7-4 = 1
	xor a,b ; A = pressed buttons + directions
	ld b,a ; B = pressed buttons + directions

	; And release the controller
	ld a, P1F_GET_NONE
	ldh [rP1], a

	; Combine with previous wCurKeys to make wNewKeys
	ld a, [wCurKeys]
	xor a,b ; A = keys that changed state
	and a,b ; A = keys that changed to pressed
	ld [wNewKeys], a
	ld a, b
	ld [wCurKeys], a
	ret

.onenibble
	ldh [rP1], a ;switch the key matrix
	call .knownret ; burn 10 cycles calling a known return
	ldh a, [rP1] ; ignore value while waiting for the key matrix to settle
	ldh a, [rP1]
	ldh a, [rP1] ; this read counts
	or a, $F0 ; A7-4 = 1: A3-0 = unpressed keys
.knownret
	ret

EntryPoint:
	call SystemDetection		; first thing to do is check what kind of system game's running on
	call EnableSound
	jp StartMenuEntry			; Jump to the main menu


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;	GAME SPECIFIC SUBROUTINE
;;	BLOCK
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

InitialisePlayer:
	; initialise player variables and load/define the tiles needed
	ld a, 32
	ld [mainCharacter_XPos], a
	ld a, 32
	ld [mainCharacter_YPos], a
	ld a, 0
	ld [mainCharacter_Direction], a
	ld [mainCharacter_OAMOffset], a

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

	; assign and update the OAM's
	ld a, [mainCharacter_YPos]
	ld [PLAYER_TOP_OAM], a
	ld a, [mainCharacter_XPos]
	ld [PLAYER_TOP_OAM + 1], a
	ld a, [mainCharacter_TileTL]
	ld [PLAYER_TOP_OAM + 2], a
	ld a, %00000000
	ld [PLAYER_TOP_OAM + 3], a
	
	ld a, [mainCharacter_YPos]
	ld [PLAYER_TOP_OAM+4], a
	ld a, [mainCharacter_XPos]
	add 8
	ld [PLAYER_TOP_OAM+4 + 1], a
	ld a, [mainCharacter_TileTR]
	ld [PLAYER_TOP_OAM+4 + 2], a
	ld a, %00000000
	ld [PLAYER_TOP_OAM+4 + 3], a
	
	ld a, [mainCharacter_YPos]
	add 8
	ld [PLAYER_TOP_OAM+8], a
	ld a, [mainCharacter_XPos]
	ld [PLAYER_TOP_OAM+8 + 1], a
	ld a, [mainCharacter_TileBL]
	ld [PLAYER_TOP_OAM+8 + 2], a
	ld a, %00000000
	ld [PLAYER_TOP_OAM+8 + 3], a
	
	ld a, [mainCharacter_YPos]
	add 8
	ld [PLAYER_TOP_OAM+12], a
	ld a, [mainCharacter_XPos]
	add 8
	ld [PLAYER_TOP_OAM+12 + 1], a
	ld a, [mainCharacter_TileBR]
	ld [PLAYER_TOP_OAM+12 + 2], a
	ld a, %00000000
	ld [PLAYER_TOP_OAM+12 + 3], a

	ret

; UpdatePlayer runs through the per frame updates required
; usually input and collision checking
UpdatePlayer:
	ld a, [wCurKeys]
	and a, PADF_UP
	jp z, .CheckDownPressed

	ld a, [mainCharacter_YPos]
	sub 1
	ld [mainCharacter_YPos], a

.CheckDownPressed
	ld a, [wCurKeys]
	and a, PADF_DOWN
	jp z, .CheckLeftPressed

	ld a, [mainCharacter_YPos]
	add 1
	ld [mainCharacter_YPos], a

.CheckLeftPressed
	ld a, [wCurKeys]
	and a, PADF_LEFT
	jp z, .CheckRightPressed

	ld a, [mainCharacter_XPos]
	sub 1
	ld [mainCharacter_XPos], a

.CheckRightPressed
	ld a, [wCurKeys]
	and a, PADF_RIGHT
	jp z, .KeyCheckFinished

	ld a, [mainCharacter_XPos]
	add 1
	ld [mainCharacter_XPos], a

.KeyCheckFinished

; 	snap to a grid tests
;	ld a, [mainCharacter_XPos]
;	and %11111000
;	ld [mainCharacter_XPos], a

;	ld a, [mainCharacter_YPos]
;	and %11111000
;	ld [mainCharacter_YPos], a

.UpdateOAM
	ld a, [mainCharacter_YPos]
	add 16							; make sure to set the sprite offsets
	ld [PLAYER_TOP_OAM], a
	ld a, [mainCharacter_XPos]
	add 8							; make sure to set the sprite offsets
	ld [PLAYER_TOP_OAM + 1], a
	
	ld a, [mainCharacter_YPos]
	add 16							; then add the metasprite offset							; make sure to set the sprite offsets
	ld [PLAYER_TOP_OAM+4], a
	ld a, [mainCharacter_XPos]
	add 8							; make sure to set the sprite offsets
	add 8							; then add the metasprite offset
	ld [PLAYER_TOP_OAM+4 + 1], a
	
	ld a, [mainCharacter_YPos]
	add 16							; make sure to set the sprite offsets
	add 8							; then add the metasprite offset
	ld [PLAYER_TOP_OAM+8], a
	ld a, [mainCharacter_XPos]
	add 8							; make sure to set the sprite offsets
	ld [PLAYER_TOP_OAM+8 + 1], a
	
	ld a, [mainCharacter_YPos]
	add 16							; make sure to set the sprite offsets
	add 8							; then add the metasprite offset
	ld [PLAYER_TOP_OAM+12], a
	ld a, [mainCharacter_XPos]
	add 8							; make sure to set the sprite offsets
	add 8							; then add the metasprite offset
	ld [PLAYER_TOP_OAM+12 + 1], a
.UpdateOAMFinished

	; screen scroll tests
	;ld a, [mainCharacter_YPos]
	;ld [rSCY], a
	;ld a, [mainCharacter_XPos]
	;ld [rSCX], a
	ret

; call to load the tiles and initialise variables
InitialiseBoxes:
	ld a, 0
	ld [wBoxInPlay], a
	ld [wBoxBeingHeld], a
	ld [currentActiveBox_YPos], a
	ld [currentActiveBox_XPos], a
	ld a, 16
	ld [currentActiveBox_OAMOffset], a				; set the OAM offset to be clear of the player character

	ld de, BoxesSpriteData
	ld bc, BoxesSpriteDataEnd - BoxesSpriteData
	ld a, 1
	call TileLoader

	ld a, c
	ld [currentActiveBox_Tile], a
	ret

; Spawns a box at the conveyor dropoff point
; @clobbers a, bc, d, hl
SpawnBoxAtConveyor:

	call RandomNumberFour			; get a number between 0-2, this gives 0-3
	cp a, 3							; if we get a 3
	jp nz, .OffsetCalc				; just decrement it
	dec a
.OffsetCalc
	add 128
	ld d, a

	ld b, 16
	ld c, 64

	call GetTileAddressByPixel

	ld a, d
	ld [hl], a

	ret

; spawn an object version of a box
; @params b: type of box to spawn
; @clobbers a, hl
SpawnBoxObject:
	ld a, b
	ld [currentActiveBox_Tile], a
	ld a, 48
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
	add 16							; offset from the top left
	ld b, a							; stick it in B ready for any function calls
	ld a, [mainCharacter_YPos]		; do the same for Y
	add 8
	ld c, a
	; call get tile by pixel to get the tile offset
	call GetTileAddressByPixel
	; check if the tile index is above 128, if so then 
	; check if there's a box in hand, if not, then pick up the box instead
	ld a, [hl]						; get the tile type at that address
	cp a, 127						; compare to 127, if > 127, carry bit will be set
	jp nc, .PickUpBox				; if carry bit not set, jump to pick up box
.PutDownBox
	; check if we're holding a box, if we're not, then don't put anything down
	ld a, [wBoxBeingHeld]
	cp a, 0
	; TODO: play a sound
	jp z, .EndOfFunc
	; replace the tile index with the box tile to place down the box
	ld a, [currentActiveBox_Tile]	; TODO: This is hardcoded to a specific tile index, undo this later
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

	; find the original tile in the ROM and replace that tile
	ld a, [hl]
	ld b, a
	push bc
	push hl							; save the VRAM tilemap address
	call ConvertVRAMTileMapToROMTileMap	; grab the original tilemap from ROM
	ld a, [hl]						; get the original tile underneath
	pop hl							; grab the VRAM tilemap address again
	ld [hl], a

	; spawn the box and put in the players hand
	pop bc
	call SpawnBoxObject
	ld a, 1
	ld [wBoxBeingHeld], a
	ld [wBoxInPlay], a
	call UpdateBox					; immediately update it to not have the one frame delay

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

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;	START MENU
;;	BLOCK
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

StartMenuEntry:							; main game loop
	; Wait until it's *not* VBlank
	ld a, [rLY]			; loads into the A register, the current scanline (rLY)
	cp 144				; compares the value in the A register, against 144
	jp nc, StartMenuEntry; jump if carry not set (if a > 144)
.WaitVBlank2:
	ld a, [rLY]			; loads into the A register, the current scanline (rLY)
	cp 144				; compares the value in the A register, against 144
	jp c, .WaitVBlank2	; jump if carry set (if a < 144)
	; above is waiting for a full complete frame

.WaitVBlank:
	ld a, [rLY]					; loads into the A register, the current scanline (rLY)
	cp 144						; compares the value in the A register, against 144
	jp c, .WaitVBlank			; jump if carry set (if a < 144)

	; Turn the LCD off
	ld a, 0
	ld [rLCDC], a

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

	call TileLoaderReset
	
	; clear the OAM data in VRAM to ensure the screen isn't covered in garbage
	; OAM is 40 sets of 4 bytes each, so clear 160 bytes to clear all of the OAM RAM
	ld a, 0
	ld b, 160
	ld hl, _OAMRAM
.ClearOAM:
	ld [hli], a
	dec b
	jp nz, .ClearOAM

	; once the OAM is clear, we can draw an object by writing its properties

	; TODO: Remove test
	; Test for CGB palette manipulation
	ld a, [wCGB]
	cp a, 0
	jp z, .EndOfCGBPalette

	; Set the destination palette address
	; first bit sets auto increment on write (successful or not)
	ld a, %10000000
	ld [rBCPS], a
	; Write palette data in little endian, padding bit first
	; format is RGB555
	; BBBBB GGGGG RRRRR Pad bit
	;FF FF FF
	ld a, %11111111
	ld [rBCPD], a
	ld a, %11111111
	ld [rBCPD], a
	;20 20 20 ;Pad 10100 10100 10100
	ld a, %10010100
	ld [rBCPD], a
	ld a, %01010010
	ld [rBCPD], a
	;10 10 10 ;Pad 01010 01010 01010
	ld a, %01001010
	ld [rBCPD], a
	ld a, %00101001
	ld [rBCPD], a
	;00 00 00
	ld a, %00000000
	ld [rBCPD], a
	ld a, %00000000
	ld [rBCPD], a

	; Do the same with the first object palette
	ld a, %10000000
	ld [rOCPS], a

	ld a, %11111111
	ld [rOCPD], a
	ld a, %11111111
	ld [rOCPD], a
	;20 20 20 ;Pad 10100 10100 10100
	ld a, %10010100
	ld [rOCPD], a
	ld a, %01010010
	ld [rOCPD], a
	;10 10 10 ;Pad 01010 01010 01010
	ld a, %01001010
	ld [rOCPD], a
	ld a, %00101001
	ld [rOCPD], a
	;00 00 00
	ld a, %00000000
	ld [rOCPD], a
	ld a, %00000000
	ld [rOCPD], a
	
	;second object palette (player)

	ld a, %11111111
	ld [rOCPD], a
	ld a, %11111111
	ld [rOCPD], a
	;19 27 27 blue ;Pad 11011 11011 10011  
	ld a, %01110011
	ld [rOCPD], a
	ld a, %01101111
	ld [rOCPD], a
	;22 16 11 red ;Pad 01011 10000 10110  
	ld a, %00010110
	ld [rOCPD], a
	ld a, %00101110
	ld [rOCPD], a
	;14 14 14 grey; Pad 01110 01110 01110
	ld a, %11001110
	ld [rOCPD], a
	ld a, %00111001
	ld [rOCPD], a

	;third object palette (thrower)
	
	ld a, %11111111
	ld [rOCPD], a
	ld a, %11111111
	ld [rOCPD], a
	;25 21 14 flesh ;Pad 01110 10101 11001
	ld a, %10111001
	ld [rOCPD], a
	ld a, %00111010
	ld [rOCPD], a
	;10 4 18 purple ;Pad 10010 00100 01010
	ld a, %10001010
	ld [rOCPD], a
	ld a, %01001000
	ld [rOCPD], a
	;17 10 4 brown ; Pad 00100 01010 10001
	ld a, %01010001
	ld [rOCPD], a
	ld a, %00010001
	ld [rOCPD], a


.EndOfCGBPalette

	; Initialise variables

	call InitialisePlayer
	call InitialiseBoxes
	call SpawnBoxAtConveyor

	ld a, 0
	ld [wButtonDebounce], a

	; Turn the LCD on
	ld a, LCDCF_ON | LCDCF_BGON	| LCDCF_OBJON | LCDCF_BG8800 ; OR together the desired flags for LCD control
	ld [rLCDC], a				; then push them to the LCD controller

	; During the first (blank) frame, set the palettes
	ld a, %11100100
	ld [rBGP], a
	ld a, %11100100
	ld [rOBP0], a
	ld [rOBP1], a

	; initialise the sound driver and start the song
	; ld hl, sample_song
	; call hUGE_init

	; enable interrupt handling for the scanline interrupt

	;ei ; enable global interrupts
	; Interrupt Enable flags 7 = null, 6 = null, 5 = null, 4 = joypad, 3 = serial, 2 = timer, 1 = lcd, 0 = vblank
	;ld a, IEF_STAT
	;ld [rIE], a

	; LCD Status flags (writing LY Coincidence interrupt)
	;ld a, STATF_LYC
	;ld [rSTAT], a

	;ld a, 50
	;ld [rLYC], a

StartMenuMain:
	; Wait until it's *not* VBlank
	ld a, [rLY]			; loads into the A register, the current scanline (rLY)
	cp 144				; compares the value in the A register, against 144
	jp nc, StartMenuMain			; jump if carry not set (if a > 144)
.WaitVBlank2:
	ld a, [rLY]			; loads into the A register, the current scanline (rLY)
	cp 144				; compares the value in the A register, against 144
	jp c, .WaitVBlank2	; jump if carry set (if a < 144)
	; above is waiting for a full complete frame

	; tick the music driver for the frame
	; call hUGE_dosound

call UpdateButtonDebounce
call UpdateKeys

call UpdatePlayer

call UpdateBox

.CheckBoxSpawn
	ld a, [wCurKeys]
	and a, PADF_A
	jp nz, .BoxSpawn

	ld a, [wCurKeys]
	and a, PADF_B
	jp nz, .BoxDespawn

	jp .CheckStartPressed

.BoxSpawn
	; check if Debounce done
	call CheckButtonDebounce				; check the button debounce status
	cp a, 1									; if a == 1, zero flag will set
	jp z, .CheckStartPressed				; skip the button
	call SetButtonDebounce					; otherwise debounce is clear and we can press the button
	call PlaceBox							; make sure to set debounce ourselves
	jp .CheckStartPressed
.BoxDespawn
	call SpawnBoxAtConveyor					; DEBUG TODO: remove this
	;call DisableBox
.CheckStartPressed:
	;ld a, [wCurKeys]
	;and a, PADF_A
	;jp nz, .StartPressed
	
	ld a, [wCurKeys]
	and a, PADF_B
	jp nz, .StartPressed
	
	ld a, [wCurKeys]
	and a, PADF_START
	jp nz, .StartPressed

	jp StartMenuMain
.StartPressed:
	; Cycle sound to clear all playing sounds
	call DisableSound
	call EnableSound
	ld b, 31					; wait a half second to insure against
	;call WaitFrames				; double pressing
	;jp MainGameStart

jp StartMenuMain



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;	DATA
;;	BLOCK
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

SECTION "Graphics Data", ROMX

AlphabetTiles: INCBIN "gfx/Alphabet.2bpp"
AlphabetTilesEnd:

PlayerSpriteData: INCBIN "gfx/player.2bpp"
PlayerSpriteDataEnd:

BoxesSpriteData: INCBIN "gfx/boxes.2bpp"
BoxesSpriteDataEnd:

	; TODO: Come up with a way to load all 240 tiles at once
	; Seems like the tile map generator expects to play the title screen
	; at memory location $8000, so make sure to flip LCDC bit 4

LevelOneTiles: INCBIN "gfx/backgrounds/Level1Background.2bpp"
LevelOneTilesEnd:
LevelOneTilemap:  INCBIN "gfx/backgrounds/Level1Background.tilemap"
LevelOneTilemapEnd:

SECTION "Sound Data", ROMX

; Example pulse sound with channel 1 settings
sound_pulse_example: ; name the sound with a label
	DW $FF10		; SOUND_CH1_START ; specifiy which channel 
	DB %00001000	; Bit 7: unused, 6-4: sweep time, 3: sweep freq +/-, 2-0: sweep shifts
	DB %01000100	; Bits 7-6: wave duty, 5-0: length of sound data (counting up to 64 at 256hz)
	DB %11110000	; Bits 7-4: envelope start, 3: envelope +/-, 2-0: num. sweeps
	DB %01000000	; Bits 7-0: lower 8 bits of the sound frequency
	DB %11000000	; Bit 7: trigger, 6: use length, 5-3: unused, 2-0 highest 3 bits of frequency

sound_game_over:; give the sound a name in the code
	DW $FF1F 		; SOUND_CH4_START ; specify which channel
	; in this case channel 4 (noise)
	DB %00000000	; Data to be written to SOUND_CH4_START
	DB %00000100	; Data to be written to SOUND_CH4_LENGTH
	DB %11110111	; Data to be written to SOUND_CH4_ENVELOPE
	DB %01011101	; Data to be written to SOUND_CH4_POLY
	DB %11000110	; Data to be written to SOUND_CH4_Options
	
sound_next_round:; give the sound a name in the code
	DW $FF10		; SOUND_CH1_START ; specifiy which channel 
	DB %11110110	; Bit 7: unused, 6-4: sweep time, 3: sweep freq +/-, 2-0: sweep shifts
	DB %01111111	; Bits 7-6: wave duty, 5-0: length of sound data (counting up to 64 at 256hz)
	DB %11010000	; Bits 7-4: envelope start, 3: envelope +/-, 2-0: num. sweeps
	DB %11110111	; Bits 7-0: lower 8 bits of the sound frequency
	DB %10000110	; Bit 7: trigger, 6: use length, 5-3: unused, 2-0 highest 3 bits of frequency
	
sound_speedup_round:; give the sound a name in the code
	DW $FF10		; SOUND_CH1_START ; specifiy which channel 
	DB %11100110	; Bit 7: unused, 6-4: sweep time, 3: sweep freq +/-, 2-0: sweep shifts
	DB %10111111	; Bits 7-6: wave duty, 5-0: length of sound data (counting up to 64 at 256hz)
	DB %10001111	; Bits 7-4: envelope start, 3: envelope +/-, 2-0: num. sweeps
	DB %00111011	; Bits 7-0: lower 8 bits of the sound frequency
	DB %10000101	; Bit 7: trigger, 6: use length, 5-3: unused, 2-0 highest 3 bits of frequency

sound_item_drop: ; name the sound with a label
	DW $FF10		; SOUND_CH1_START ; specifiy which channel 
	DB %00101001	; Bit 7: unused, 6-4: sweep time, 3: sweep freq +/-, 2-0: sweep shifts
	DB %10000001	; Bits 7-6: wave duty, 5-0: length of sound data (counting up to 64 at 256hz)
	DB %11110001	; Bits 7-4: envelope start, 3: envelope +/-, 2-0: num. sweeps
	DB %01000000	; Bits 7-0: lower 8 bits of the sound frequency
	DB %11100000	; Bit 7: trigger, 6: use length, 5-3: unused, 2-0 highest 3 bits of frequency
	
sound_item_grab: ; name the sound with a label
	DW $FF15		; SOUND_CH2_START ; specifiy which channel 
	DB %00000000	; Not used, 0's for padding
	DB %10111111	; Bits 7-6: wave duty, 5-0: length of sound data (counting up to 64 at 256hz)
	DB %10110001	; Bits 7-4: envelope start, 3: envelope +/-, 2-0: num. sweeps
	DB %10101100	; Bits 7-0: lower 8 bits of the sound frequency
	DB %10000111	; Bit 7: trigger, 6: use length, 5-3: unused, 2-0 highest 3 bits of frequency

sound_ball_bounce: ; give the sound a name in the code
	DW $FF1F 		; SOUND_CH4_START ; specify which channel
	; in this case channel 4 (noise)
	DB %00000000	; Data to be written to SOUND_CH4_START
	DB %00000100	; Data to be written to SOUND_CH4_LENGTH
	DB %11110111	; Data to be written to SOUND_CH4_ENVELOPE
	DB %01010101	; Data to be written to SOUND_CH4_POLY
	DB %11000110	; Data to be written to SOUND_CH4_Options

sound_character_walk: ; give the sound a name in the code
	DW $FF1F 		; SOUND_CH4_START ; specify which channel
	; in this case channel 4 (noise)
	DB %00000000	; Data to be written to SOUND_CH4_START
	DB %00000100	; Data to be written to SOUND_CH4_LENGTH
	DB %11110111	; Data to be written to SOUND_CH4_ENVELOPE
	DB %01011101	; Data to be written to SOUND_CH4_POLY
	DB %11000110	; Data to be written to SOUND_CH4_Options