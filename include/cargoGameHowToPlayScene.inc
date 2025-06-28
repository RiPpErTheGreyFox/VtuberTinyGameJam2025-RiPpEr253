SECTION "HowToPlay Subroutines", ROM0

; loads the background, tilemap and sets up the rest of the game field
; make sure the LCD is OFF
; @clobbers a, bc, de, hl
InitialiseHowToPlay:
	; Copy tile data into VRAM
	ld de, HowToPlayTiles				; load the address of the tiles into the DE register
	ld hl, $9000				; load the beginning VRAM address into HL (HL is easy to inc/dec)
	ld bc, HowToPlayTilesEnd - HowToPlayTiles		; load the length of Tiles into BC
	call Memcopy				; call the memcopy subroutine

	; The above tile loading will clobber the tilemap in VRAM, but for now just load the other half of tiles

	; Copy tilemap data into VRAM (functionally identical to above but pointing to tilemap data and addresses)
	ld de, HowToPlayTilemap
	ld hl, $9800
	ld bc, HowToPlayTilemapEnd - HowToPlayTilemap
	call Memcopy				; call the memcopy subroutine

	call TileLoaderReset
    call SetBlankDMGPalette

	ret

UpdateHowToPlayScene:
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
