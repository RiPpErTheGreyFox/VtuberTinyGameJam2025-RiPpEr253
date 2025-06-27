SECTION "MainMenu Subroutines", ROM0

; loads the background, tilemap and sets up the rest of the game field
; make sure the LCD is OFF
; @clobbers a, bc, de, hl
InitialiseMainMenu:
	; Copy tile data into VRAM
	ld de, MainMenuTiles				; load the address of the tiles into the DE register
	ld hl, $9000				; load the beginning VRAM address into HL (HL is easy to inc/dec)
	ld bc, MainMenuTilesEnd - MainMenuTiles		; load the length of Tiles into BC
	call Memcopy				; call the memcopy subroutine

	; The above tile loading will clobber the tilemap in VRAM, but for now just load the other half of tiles

	; Copy tilemap data into VRAM (functionally identical to above but pointing to tilemap data and addresses)
	ld de, MainMenuTilemap
	ld hl, $9800
	ld bc, MainMenuTilemapEnd - MainMenuTilemap
	call Memcopy				; call the memcopy subroutine

	call TileLoaderReset
    call SetBlankDMGPalette

	ret

UpdateMainMenuScene:
    call UpdateKeys

    ld a, [wCurKeys]
	and a, PADF_START
	jp nz, .StartGame

    jp .EndOfFunc

.StartGame
    ld a, 3
    ld [wCurrentScene], a
    ld c, 10
    call FadeToWhite
    jp ReloadGame

.EndOfFunc
    ret
