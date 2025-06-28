SECTION "Cutscene Subroutines", ROM0

SECTION "String Data", ROMX
wCutsceneString::  db "typewriter test", 255

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
    ld b, 2
    call WaitFrames

    ld hl, wCutsceneString
	ld de, $9980
    call DrawTextTilesWithDelays
    call UpdateKeys

    ld a, [wCurKeys]
	and a, PADF_START
	jp nz, .StartGame

    jp UpdateCutsceneScene

.StartGame
    ld a, 2
    ld [wCurrentScene], a
    ld c, 10
    call FadeToWhite
    jp ReloadGame

.EndOfFunc
    ret
