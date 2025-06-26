INCLUDE "include/hardware.inc"			; include all the defines
INCLUDE "include/cargoGameStructs.inc"	; out of file definitions
INCLUDE "include/cargoGameConstants.inc"
INCLUDE "include/cargoGameUtilitySubroutines.inc"
INCLUDE "include/cargoGameGameplaySubroutines.inc"
INCLUDE "include/cargoGameSoundData.inc"

; gameplay definitions
SECTION "Counter", WRAM0
wFrameCounter: db
wButtonDebounce: db

SECTION "Input Variables", WRAM0		; set labels in Work RAM for easy variable use
wCurKeys: db							; label: declare byte, reserves a byte for use later
wNewKeys: db

SECTION "Item Data", ROMX
wTestText::  db "testing", 255
SECTION "NumberStringData", WRAM0
wNumberStringData: db
	:db
	:db

SECTION "BoxVictoryConditionRAMBlock", WRAM0
wBoxTypeMemory: db						; need four bytes for temp storage in the victory condition area
	:db
	:db
	:db

SECTION "Gameplay Data", WRAM0
wBoxInPlay: db							; treat as a bool
wBoxBeingHeld: db						; bool
wBoxTileIndex: db						; starting tile index for the box graphics
wBoxesRemainingInLevel: db				; the amount of boxes we need to spawn
wBoxesRemainingFlammable: db			; the amount of flammable boxes left to spawn
wBoxesRemainingRadioactive: db			; the amount of radioactive boxes left to spawn
wVictoryFlagSet: db
	dstruct PLAYER, mainCharacter		; declare our structs
	dstruct BOX, currentActiveBox
	dstruct CURSOR, boxCursor

SECTION "Animation Data", WRAM0
wPlayerCurrentFrame: db

SECTION "Managed Variables", WRAM0
wTileBankZero: dw						; variables that hold the current count of tiles loaded by the manager
wTileBankOne: dw
wTileBankTwo: dw
wFontFirstTileOffset: db				; where in Bank one the font starts

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
	call DisableSound
	call InitialisePlayer
	call InitialiseBoxes
	call InitialiseCursor
	call InitialiseFont

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

call DrawBoxObject
call DrawCursorObject
call DrawPlayer

call CheckForVictory

; DEBUG update boxes remaining

	ld a, [wBoxesRemainingInLevel]
	ld hl, wNumberStringData
	call NumberToString

	ld hl, wNumberStringData
	ld de, $9800 + $10
	call DrawTextTilesLoop

call UpdateButtonDebounce
call UpdateKeys
call UpdatePlayer
call UpdateBox
call UpdateCursor

.CheckBoxSpawn
	ld a, [wCurKeys]
	and a, PADF_A
	jp nz, .BoxSpawn

	;ld a, [wCurKeys]
	;and a, PADF_B
	;jp nz, .BoxDespawn

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

PlayerSpriteData: INCBIN "gfx/player.2bpp"
PlayerSpriteDataEnd:

CursorSpriteData: INCBIN "gfx/cursor.2bpp"
CursorSpriteDataEnd:

BoxesSpriteData: INCBIN "gfx/boxes.2bpp"
BoxesSpriteDataEnd:

AlphabetTiles: INCBIN "gfx/backgrounds/text-font.2bpp"
AlphabetTilesEnd:

LevelOneTiles: INCBIN "gfx/backgrounds/Level1Background.2bpp"
LevelOneTilesEnd:
LevelOneTilemap:  INCBIN "gfx/backgrounds/Level1Background.tilemap"
LevelOneTilemapEnd:
