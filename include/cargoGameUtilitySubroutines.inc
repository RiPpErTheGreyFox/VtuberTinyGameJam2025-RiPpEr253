SECTION "Subroutines", ROM0

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
; idles the CPU until we're out of mode 3
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

; Load Text font into VRAM for future use
; @clobbers a, bc, de, hl
InitialiseFont:
	
	ld de, AlphabetTiles
	ld bc, AlphabetTilesEnd - AlphabetTiles
	ld a, 1

	call TileLoader

	ld a, c
	ld [wFontFirstTileOffset], a

	ret

; Draw Text tiles to the background buffer
; @param de: address of the first tile to start writing to
; @param hl: pointer to the address containing the literal string
; @clobbers a
DrawTextTilesLoop:
	; check for the end of string character (255/$FF)
	ld a, [hl]
	cp 255
	ret z

	; write the current character (in hl) to the address
	; on the tilemap (in de)
	ld a, [wFontFirstTileOffset]
	ld b, a
	ld a, [hl]
	add b
	ld [de], a

	inc hl
	inc de

	; move to the next character and next background tile
	jp DrawTextTilesLoop

; Takes the number in the A register, turns it into BCD and outputs a string to hl
; @param a: the number to turn into a string
; @param hl: memory address of the block of RAM to output the string to
NumberToString:
	add 0
    daa                                     ; Use Decimal Adjust to turn the 8-bit number into a BCD number
    ld b, a                                 ; copy the number into the b register
    swap a                                  ; swap the 4-bit nibbles so the tens is in the 4 LSB
    and %00001111                           ; mask out the top nibble
    add START_OF_NUMBERS_IN_CHAR_MAP        ; add the offset to the 0 character in the character map (more offset is handled by the drawing function)
    ld [hli], a                             ; store the tile reference number into the first character of the string
    ld a, b                                 ; now grab the copy stored in b
    and %00001111                           ; mask out the top nibble
    add START_OF_NUMBERS_IN_CHAR_MAP        ; add the offset to 0 character again
    ld [hli], a                             ; store the tile ref. number into the second character of the string
    ld a, 255                           
    ld [hli], a                             ; end the string with 255
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

; Convert an X/Y tile position to an address
; @param b: X
; @param c: Y
; @return hl: address of Tile
; @clobbers a
GetTileAddressByIndex:
	ld h, 0
	ld l, c		; grab the Y index of the tile we're looking for
	add hl, hl		; * 2
	add hl, hl		; * 4
	add hl, hl		; * 8
	add hl, hl		; * 16
	add hl, hl		; * 32

	ld c, b
	ld b, 0
	add hl, bc		; now add the X offset to it
	ld bc, $9800	; now add the tilemap address
	add hl, bc
	
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

