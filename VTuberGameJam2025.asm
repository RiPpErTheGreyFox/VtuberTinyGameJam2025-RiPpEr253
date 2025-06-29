INCLUDE "include/hardware.inc"			; include all the defines
INCLUDE "include/cargoGameStructs.inc"	; out of file definitions
INCLUDE "include/cargoGameConstants.inc"
INCLUDE "include/cargoGameUtilitySubroutines.inc"
INCLUDE "include/cargoGameGameplaySubroutines.inc"
INCLUDE "include/cargoGameMainMenuScene.inc"
INCLUDE "include/cargoGameHowToPlayScene.inc"
INCLUDE "include/cargoGameCutsceneScene.inc"
INCLUDE "include/cargoGameSoundData.inc"

; gameplay definitions
SECTION "Counter", WRAM0
wFrameCounter: db
wButtonDebounce: db

SECTION "Input Variables", WRAM0		; set labels in Work RAM for easy variable use
wCurKeys: db							; label: declare byte, reserves a byte for use later
wNewKeys: db

SECTION "Item Data", ROMX
wVictoryString::  db "youre winner", 255
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

wLevelSelected: db						
wCurrentScene: db						; 0=MainMenu, 1=Cutscene, 2=HowToPlay, 3=Game

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
;;	MAIN PROGRAM
;;	BLOCK
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;


EntryPoint:
	call SystemDetection		; first thing to do is check what kind of system game's running on
	call EnableSound
	ld a, 1
	ld [wLevelSelected], a 
	ld a, 0						; TODO: hardcoding the game scene
	ld [wCurrentScene], a

ReloadGame:
	ld sp, $FFFE				; reset the stack pointer

	jp ProgramEntry			; Jump to the main menu

ProgramEntry:							; main game loop
	; Wait until it's *not* VBlank
	ld a, [rLY]			; loads into the A register, the current scanline (rLY)
	cp 144				; compares the value in the A register, against 144
	jp nc, ProgramEntry; jump if carry not set (if a > 144)
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
	ld [rSCY], a				; reset the scroll registers
	ld [rSCX], a
	
	call ClearOAM


	; once the OAM is clear, we can draw an object by writing its properties
	call SetDefaultDMGPalette
	call LoadDefaultCGBPalette
	
	; check which scene is gunna be loaded and load that
	; 0=MainMenu, 1=Cutscene, 2=HowToPlay, 3=Game
	ld a, [wCurrentScene]
	cp a, 0
	jp z, .MainMenuLoading
	
	cp a, 1
	jp z, .CutsceneLoading
	
	cp a, 2
	jp z, .HowToPlayLoading
	
	cp a, 3
	jp z, .GameSceneLoading

.MainMenuLoading
	call InitialiseMainMenu
	jp .FinishedLoadingScene
.CutsceneLoading
	call InitialiseCutscene
	jp .FinishedLoadingScene
.HowToPlayLoading
	call InitialiseHowToPlay
	jp .FinishedLoadingScene
.GameSceneLoading
	call InitialiseLevel
	jp .FinishedLoadingScene
.FinishedLoadingScene

	; Initialise variables
	;call DisableSound

	ld a, 0
	ld [wButtonDebounce], a

	; Turn the LCD on
	ld a, LCDCF_ON | LCDCF_BGON	| LCDCF_OBJON | LCDCF_BG8800 ; OR together the desired flags for LCD control
	ld [rLCDC], a				; then push them to the LCD controller
	
	; initialise the sound driver and start the song
	;ld hl, sample_song
	;call hUGE_init

	ld c, 15
	call FadeFromWhite

ProgramMain:
	; Wait until it's *not* VBlank
	ld a, [rLY]			; loads into the A register, the current scanline (rLY)
	cp 144				; compares the value in the A register, against 144
	jp nc, ProgramMain			; jump if carry not set (if a > 144)
.WaitVBlank2:
	ld a, [rLY]			; loads into the A register, the current scanline (rLY)
	cp 144				; compares the value in the A register, against 144
	jp c, .WaitVBlank2	; jump if carry set (if a < 144)
	; above is waiting for a full complete frame

	; check which scene we're on and tick that
	; 0=MainMenu, 1=Cutscene, 2=HowToPlay, 3=Game
	ld a, [wCurrentScene]
	cp a, 0
	jp z, .MainMenuTick
	
	cp a, 1
	jp z, .CutsceneTick
	
	cp a, 2
	jp z, .HowToPlayTick
	
	cp a, 3
	jp z, .GameSceneTick

.MainMenuTick
	call UpdateMainMenuScene
	jp .FinishedTickingScene
.CutsceneTick
	call UpdateCutsceneScene
	jp .FinishedTickingScene
.HowToPlayTick
	call UpdateHowToPlayScene
	jp .FinishedTickingScene
.GameSceneTick
	call UpdateGameScene
	jp .FinishedTickingScene
.FinishedTickingScene

	; tick the music driver for the frame
	;call hUGE_dosound

jp ProgramMain

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

ConveyorsSpriteData: INCBIN "gfx/Conveyors.2bpp"
ConveyorsSpriteDataEnd:

AlphabetTiles: INCBIN "gfx/backgrounds/text-font.2bpp"
AlphabetTilesEnd:

LevelOneTiles: INCBIN "gfx/backgrounds/Level1Background.2bpp"
LevelOneTilesEnd:
LevelOneTilemap:  INCBIN "gfx/backgrounds/Level1Background.tilemap"
LevelOneTilemapEnd:

CutsceneTiles: INCBIN "gfx/backgrounds/Cutscene1.2bpp"
CutsceneTilesEnd:
CutsceneTilemap:  INCBIN "gfx/backgrounds/Cutscene1.tilemap"
CutsceneTilemapEnd:

HowToPlayTiles: INCBIN "gfx/backgrounds/HowToPlayBackground.2bpp"
HowToPlayTilesEnd:
HowToPlayTilemap:  INCBIN "gfx/backgrounds/HowToPlayBackground.tilemap"
HowToPlayTilemapEnd:

HowToPlay2Tiles: INCBIN "gfx/backgrounds/HowToPlayBackground2.2bpp"
HowToPlay2TilesEnd:
HowToPlay2Tilemap:  INCBIN "gfx/backgrounds/HowToPlayBackground2.tilemap"
HowToPlay2TilemapEnd:

MainMenuTiles: INCBIN "gfx/backgrounds/MainMenuBackground.2bpp"
MainMenuTilesEnd:
MainMenuTilemap:  INCBIN "gfx/backgrounds/MainMenuBackground.tilemap"
MainMenuTilemapEnd: