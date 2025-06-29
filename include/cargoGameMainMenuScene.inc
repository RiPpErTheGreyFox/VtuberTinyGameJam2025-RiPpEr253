SECTION "MainMenu Variables", WRAM0
wYScrollCounter: db

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

	; set how scrolled the screen is
	ld a, 112
	ld [wYScrollCounter], a
	ld [rSCY], a

		
	; initialise the sound driver and start the song
	ld hl, mainmenu_song
	call hUGE_init

	ret

UpdateMainMenuScene:
	
	; tick the music driver for the frame
	call hUGE_dosound

	; check if the scroll counter is above zero, if it is, scroll up 1
	ld a, [wYScrollCounter]
	cp a, 1
	jp nc, .KeepScrollingScreen

    call UpdateKeys

    ld a, [wCurKeys]
	and a, PADF_START
	jp nz, .StartGame

    jp .EndOfFunc

.StartGame
    ld a, 1
    ld [wCurrentScene], a
	call DisableSound
	call EnableSound
    ld c, 10
    call FadeToWhite
    jp ReloadGame

.KeepScrollingScreen
	dec a
	ld [rSCY], a
	ld [wYScrollCounter], a

.EndOfFunc
    ret
