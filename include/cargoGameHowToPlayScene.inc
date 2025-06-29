SECTION "HowToPlay Variables", WRAM0
wCurrentHowToPlayPage: db

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

	; initialise variables
	ld a, 0
	ld [wCurrentHowToPlayPage], a

	ret

; flips the how to play page, involves turning off the LCD
; @clobbers a, bc, de, hl
HowToPlayFlipToPageTwo:
	ld c, 15
	call FadeToWhite

	call DisableLCD

	; Copy tile data into VRAM
	ld de, HowToPlay2Tiles				; load the address of the tiles into the DE register
	ld hl, $9000				; load the beginning VRAM address into HL (HL is easy to inc/dec)
	ld bc, HowToPlay2TilesEnd - HowToPlay2Tiles		; load the length of Tiles into BC
	call Memcopy				; call the memcopy subroutine

	; The above tile loading will clobber the tilemap in VRAM, but for now just load the other half of tiles

	; Copy tilemap data into VRAM (functionally identical to above but pointing to tilemap data and addresses)
	ld de, HowToPlay2Tilemap
	ld hl, $9800
	ld bc, HowToPlay2TilemapEnd - HowToPlay2Tilemap
	call Memcopy				; call the memcopy subroutine
	
	call EnableLCD

	ld c, 15
	call FadeFromWhite

	ret

UpdateHowToPlayScene:
    call UpdateKeys

    ld a, [wCurKeys]
	and a, PADF_START
	jp nz, .StartPressed

    jp .EndOfFunc

.StartPressed
	ld a, [wCurrentHowToPlayPage]
	cp a, 0
	jp z, .FlipPage2
	jp .StartGame

.FlipPage2
	call HowToPlayFlipToPageTwo
	ld hl, wCurrentHowToPlayPage
	inc [hl]
	jp .EndOfFunc

.StartGame
    ld a, 3
    ld [wCurrentScene], a
    ld c, 10
    call FadeToWhite
    jp ReloadGame

.EndOfFunc
    ret
