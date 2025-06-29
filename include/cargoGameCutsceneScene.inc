SECTION "Cutscene Subroutines", ROM0

SECTION "String Data", ROMX
wCutsceneString::  db "typewriter test", 255

wCutscene1String1: db "hey newbie!",255
;blank line
wCutscene1String2: db "welcome to the",255
wCutscene1String3: db "cargo docks,",255
wCutscene1String4: db "we got some boxes",255
wCutscene1String5: db "that need to be",255
wCutscene1String6: db "loaded into the",255
wCutscene1String7: db "cargo hold of",255
wCutscene1String8: db "the ship that",255
wCutscene1String9: db "docked earlier,",255
wCutscene1String10: db "stack the boxes",255
wCutscene1String11: db "onto their lift",255
wCutscene1String12: db "area so they can",255
wCutscene1String13: db "leave asap,",255
wCutscene1String14: db "youre a boxhound",255
wCutscene1String15: db "now kid!",255

wCutscene2String1: db "not a bad",255
wCutscene2String2: db "job newbie",255
wCutscene2String3: db "but now you",255
wCutscene2String4: db "are going to be",255
wCutscene2String5: db "working with some",255
wCutscene2String6: db "more dangerous",255
wCutscene2String7: db "crates, think",255
wCutscene2String8: db "ahead when",255
wCutscene2String9: db "stacking!",255

	
wCutscene3String1: db "hey that was",255
wCutscene3String2: db "good, now you",255
wCutscene3String3: db "have a big final",255
wCutscene3String4: db "job before you",255
wCutscene3String5: db "get to go home!",255

		
wCutscene4String1: db "wow, not bad",255
wCutscene4String2: db "for a newbie",255
wCutscene4String3: db "i guess that",255
wCutscene4String4: db "means you get",255
wCutscene4String5: db "to clock out now,",255
wCutscene4String6: db "see you later!",255

; loads the background, tilemap and sets up the rest of the game field
; make sure the LCD is OFF
; @clobbers a, bc, de, hl
InitialiseCutscene:
	; Copy tile data into VRAM
	ld de, CutsceneTiles				; load the address of the tiles into the DE register
	ld hl, $9000				; load the beginning VRAM address into HL (HL is easy to inc/dec)
	ld bc, CutsceneTilesEnd - CutsceneTiles		; load the length of Tiles into BC
	call Memcopy				; call the memcopy subroutine

	; The above tile loading will clobber the tilemap in VRAM, but for now just load the other half of tiles

	; Copy tilemap data into VRAM (functionally identical to above but pointing to tilemap data and addresses)
	ld de, CutsceneTilemap
	ld hl, $9800
	ld bc, CutsceneTilemapEnd - CutsceneTilemap
	call Memcopy				; call the memcopy subroutine

	call TileLoaderReset
    call SetBlankDMGPalette
    call InitialiseFont

	ret


UpdateCutsceneScene:
    
	ld a, [wLevelSelected]
	cp a, 1
	jp z, .IntroScene
	cp a, 2
	jp z, .EndOfLevel1
	cp a, 3
	jp z, .EndOfLevel2
	cp a, 4
	jp z, .EndOfLevel3


.IntroScene
    call Cutscene1Run
.EndOfLevel1
    call Cutscene2Run
.EndOfLevel2
    call Cutscene3Run
.EndOfLevel3
    call Cutscene4Run

.EndOfFunc
    ret

; Clears the textbox back to its default state
ClearTextbox:
	; grab the VRAM address in the cartridge of $99E1 and then run to $9A12 (length 49 in decimal)
	ld c, 0
	ld hl, $99E1
	call ConvertVRAMTileMapToROMTileMap
	ld d, h
	ld e, l
	ld hl, $99E1
	ld bc, 49
	call MemcopyNoMode3
	ret

; loops forever until an A press is called
; @clobbers a, bc, de, hl
WaitForAPress:
	ld b, 31		; little bit of debounce as a treat
	call WaitFrames
.Loop
	ld b, 2
	call WaitFrames
	call UpdateKeys

    ld a, [wCurKeys]
	and a, PADF_A
	jp nz, .EndOfFunc
	jp .Loop
.EndOfFunc
	ret

; Cutscene functions just statically move through their intended dialog and then end
; @clobbers a, bc, de, hl
Cutscene1Run:
    ld hl, wCutscene1String1
	ld de, $99E1
    call DrawTextTilesWithDelays

	call WaitForAPress

	call ClearTextbox

    ld hl, wCutscene1String2
	ld de, $99E1
    call DrawTextTilesWithDelays
    ld hl, wCutscene1String3
	ld de, $9A01
    call DrawTextTilesWithDelays

	call WaitForAPress

	call ClearTextbox

    ld hl, wCutscene1String4
	ld de, $99E1
    call DrawTextTilesWithDelays
    ld hl, wCutscene1String5
	ld de, $9A01
    call DrawTextTilesWithDelays

	call WaitForAPress

	call ClearTextbox

    ld hl, wCutscene1String6
	ld de, $99E1
    call DrawTextTilesWithDelays
    ld hl, wCutscene1String7
	ld de, $9A01
    call DrawTextTilesWithDelays

	call WaitForAPress

	call ClearTextbox

    ld hl, wCutscene1String8
	ld de, $99E1
    call DrawTextTilesWithDelays
    ld hl, wCutscene1String9
	ld de, $9A01
    call DrawTextTilesWithDelays

	call WaitForAPress

	call ClearTextbox

    ld hl, wCutscene1String10
	ld de, $99E1
    call DrawTextTilesWithDelays
    ld hl, wCutscene1String11
	ld de, $9A01
    call DrawTextTilesWithDelays

	call WaitForAPress

	call ClearTextbox

    ld hl, wCutscene1String12
	ld de, $99E1
    call DrawTextTilesWithDelays
    ld hl, wCutscene1String13
	ld de, $9A01
    call DrawTextTilesWithDelays

	call WaitForAPress

	call ClearTextbox

    ld hl, wCutscene1String14
	ld de, $99E1
    call DrawTextTilesWithDelays
    ld hl, wCutscene1String15
	ld de, $9A01
    call DrawTextTilesWithDelays

	call WaitForAPress

	ld a, 2
    ld [wCurrentScene], a
    ld c, 10
    call FadeToWhite
    jp ReloadGame

	ret 

Cutscene2Run:
    ld hl, wCutscene2String1
	ld de, $99E1
    call DrawTextTilesWithDelays
    ld hl, wCutscene2String2
	ld de, $9A01
    call DrawTextTilesWithDelays

	call WaitForAPress

	call ClearTextbox

    ld hl, wCutscene2String3
	ld de, $99E1
    call DrawTextTilesWithDelays
    ld hl, wCutscene2String4
	ld de, $9A01
    call DrawTextTilesWithDelays

	call WaitForAPress

	call ClearTextbox

    ld hl, wCutscene2String5
	ld de, $99E1
    call DrawTextTilesWithDelays
    ld hl, wCutscene2String6
	ld de, $9A01
    call DrawTextTilesWithDelays

	call WaitForAPress

	call ClearTextbox

    ld hl, wCutscene2String7
	ld de, $99E1
    call DrawTextTilesWithDelays
    ld hl, wCutscene2String8
	ld de, $9A01
    call DrawTextTilesWithDelays

	call WaitForAPress

	call ClearTextbox

    ld hl, wCutscene2String9
	ld de, $99E1
    call DrawTextTilesWithDelays

	call WaitForAPress

	ld a, 3
    ld [wCurrentScene], a
    ld c, 10
    call FadeToWhite
    jp ReloadGame

	ret 

Cutscene3Run:
    ld hl, wCutscene3String1
	ld de, $99E1
    call DrawTextTilesWithDelays
    ld hl, wCutscene3String2
	ld de, $9A01
    call DrawTextTilesWithDelays

	call WaitForAPress

	call ClearTextbox

    ld hl, wCutscene3String3
	ld de, $99E1
    call DrawTextTilesWithDelays
    ld hl, wCutscene3String4
	ld de, $9A01
    call DrawTextTilesWithDelays

	call WaitForAPress

	call ClearTextbox

    ld hl, wCutscene3String5
	ld de, $99E1
    call DrawTextTilesWithDelays

	call WaitForAPress

	ld a, 3
    ld [wCurrentScene], a
    ld c, 10
    call FadeToWhite
    jp ReloadGame
	ret

Cutscene4Run:
    ld hl, wCutscene4String1
	ld de, $99E1
    call DrawTextTilesWithDelays
    ld hl, wCutscene4String2
	ld de, $9A01
    call DrawTextTilesWithDelays

	call WaitForAPress

	call ClearTextbox

    ld hl, wCutscene4String3
	ld de, $99E1
    call DrawTextTilesWithDelays
    ld hl, wCutscene4String4
	ld de, $9A01
    call DrawTextTilesWithDelays

	call WaitForAPress

	call ClearTextbox

    ld hl, wCutscene4String5
	ld de, $99E1
    call DrawTextTilesWithDelays
    ld hl, wCutscene4String6
	ld de, $9A01
    call DrawTextTilesWithDelays

	call WaitForAPress

	ld a, 1
	ld [wLevelSelected], a
	ld a, 0
    ld [wCurrentScene], a
    ld c, 10
    call FadeToWhite
    jp ReloadGame
	ret
	ret
