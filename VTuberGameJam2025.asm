INCLUDE "include/hardware.inc"			; include all the defines

; graphical defines
DEF PLAYER_TOP_OAM EQU _OAMRAM
DEF PLAYER_BOT_OAM EQU _OAMRAM + 4
DEF THROWER_TOP_OAM EQU _OAMRAM + 8
DEF THROWER_BOT_OAM EQU _OAMRAM + 12
DEF THROWN_ITEM_OAM EQU _OAMRAM + 16

DEF ROUNDTENSDIGITTOPOAM EQU _OAMRAM + 20
DEF ROUNDTENSDIGITBOTOAM EQU _OAMRAM + 24
DEF ROUNDONESDIGITTOPOAM EQU _OAMRAM + 28
DEF ROUNDONESDIGITBOTOAM EQU _OAMRAM + 32
DEF SCORETENSDIGITTOPOAM EQU _OAMRAM + 36
DEF SCORETENSDIGITBOTOAM EQU _OAMRAM + 40
DEF SCOREONESDIGITTOPOAM EQU _OAMRAM + 44
DEF SCOREONESDIGITBOTOAM EQU _OAMRAM + 48
DEF PRESSSTARTOAM EQU _OAMRAM + 52

DEF PLAYERTOPFRAME1 EQU 3
DEF PLAYERBOTFRAME1 EQU 10
DEF PLAYERTOPFRAME2 EQU 4
DEF PLAYERBOTFRAME2 EQU 11
DEF PLAYERTOPFRAME3 EQU 3
DEF PLAYERBOTFRAME3 EQU 12

DEF THROWERTOPFRAME1 EQU 2
DEF THROWERBOTFRAME1 EQU 7
DEF THROWERTOPFRAME2 EQU 2
DEF THROWERBOTFRAME2 EQU 8
DEF THROWERTOPFRAME3 EQU 2	
DEF THROWERBOTFRAME3 EQU 9

DEF ITEMSPRITE1 EQU 0
DEF ITEMSPRITE2 EQU 1
DEF ITEMSPRITE3 EQU 5
DEF ITEMSPRITE4 EQU 6

; These definitions assume they're sprites
DEF ALPHABETZEROTOP EQU 128
DEF ALPHABETZEROBOT EQU 138
DEF ALPHABETONETOP EQU 129
DEF ALPHABETONEBOT EQU 139
DEF ALPHABETTWOTOP EQU 130
DEF ALPHABETTWOBOT EQU 140
DEF ALPHABETTHREETOP EQU 130
DEF ALPHABETTHREEBOT EQU 141
DEF ALPHABETFOURTOP EQU 131
DEF ALPHABETFOURBOT EQU 142
DEF ALPHABETFIVETOP EQU 132
DEF ALPHABETFIVEBOT EQU 143
DEF ALPHABETSIXTOP EQU 133
DEF ALPHABETSIXBOT EQU 144
DEF ALPHABETSEVENTOP EQU 134
DEF ALPHABETSEVENBOT EQU 145
DEF ALPHABETEIGHTTOP EQU 135
DEF ALPHABETEIGHTBOT EQU 146
DEF ALPHABETNINETOP EQU 136
DEF ALPHABETNINEBOT EQU 147

; Animation Constants
DEF THROWER_ANIMATION_TIMER_DEFAULT EQU 7
DEF PLAYER_ANIMATION_TIMER_DEFAULT EQU 6

; Gameplay Constants
DEF THROWER_LEFT_WALL EQU 25
DEF THROWER_RIGHT_WALL EQU 143
DEF INITIAL_THROWER_SPEED EQU 1
DEF MAXIMUM_THROWER_SPEED EQU 3
DEF INITIAL_TIME_BETWEEN_THROWS EQU 60
DEF MINIMUM_TIME_BETWEEN_THROWS EQU 05
DEF DEFAULT_ITEMS_REMAINING EQU %00010000	; 10 in BCD
DEF DEFAULT_TIME_BETWEEN_SPEEDUPS EQU 2
DEF SECONDS_COUNTER EQU 1
DEF ROUND_COUNTDOWN_START EQU 60
; gameplay definitions

SECTION "Counter", WRAM0
wFrameCounter: db

SECTION "Input Variables", WRAM0		; set labels in Work RAM for easy variable use
wCurKeys: db							; label: declare byte, reserves a byte for use later
wNewKeys: db

SECTION "Item Data", WRAM0
wItemMomentumX: db
wItemMomentumY: db

SECTION "Gameplay Data", WRAM0
wCurrentScore: db
wRemainingItems: db
wRoundCounter: db
wGameState: db
wRoundCountdown: db
wItemDropCurrentTimeBetweenThrows: db
wItemDropCountdown: db
wItemDropCounterFramesRemaining: db
wTimeBetweenSpeedups: db

SECTION "Animation Data", WRAM0
wPlayerCurrentFrame: db
wPlayerFrameTimer: db
wThrowerCurrentFrame: db
wThrowerFrameTimer: db
wThrowerDirection: db
wThrowerSpeed: db

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


; Convert a pixel position to a tilemap address
; hl = $9800 + X + Y * 32
; @param b: X
; @param c: Y
; @return hl: tile address
GetTileByPixel:
	; First, we need to divide by 8 to convert a pixel position to a tile position.
	; After this we want to multiply the Y position by 32.
	; These operations effectively cancelt out so we only need to mask the Y value.
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

; Update the scoreboard sprites with the current data
UpdateScoreboard:
	ld a, [wRoundCounter]	; load the score again
	and $F0					; remove the ones digit by masking 4 bits 11110000
	srl a					; shift right 4 times to move the 4 bit BCD into the
	srl a					; lower nibble to be treated as another ones digit
	srl a
	srl a
	ld b, a
	ld hl, ROUNDTENSDIGITTOPOAM
	call UpdateSingleDigitScoreboard
	ld a, [wRoundCounter]	; load A with the BCD score
	and $0F					; remove the tens digit by masking 4 bits 00001111
	ld b, a					; save the value of a into b
	ld hl, ROUNDONESDIGITTOPOAM
	call UpdateSingleDigitScoreboard

	
	ld a, [wRemainingItems]	; load the score again
	and $F0					; remove the ones digit by masking 4 bits 11110000
	srl a					; shift right 4 times to move the 4 bit BCD into the
	srl a					; lower nibble to be treated as another ones digit
	srl a
	srl a
	ld b, a
	ld hl, SCORETENSDIGITTOPOAM
	call UpdateSingleDigitScoreboard
	ld a, [wRemainingItems]	; load A with the BCD score
	and $0F					; remove the tens digit by masking 4 bits 00001111
	ld b, a					; save the value of a into b
	ld hl, SCOREONESDIGITTOPOAM
	call UpdateSingleDigitScoreboard

	ret


; Update a scoreboard sprite with the required tiles
; expects the bottom OAM to be exactly 4 bytes after the top
; @param b: number to set to
; @param hl: OAM address of the top tile
; @return D: Top Tile ID
; @return E: Bottom Tile ID
UpdateSingleDigitScoreboard:
; use D as the top tile ID
; use E as the bottom tile ID
	
	ld a, b						;place b value into a
	cp a, 0
	jp z, .SetNumberZero
	cp a, 1
	jp z, .SetNumberOne
	cp a, 2
	jp z, .SetNumberTwo
	cp a, 3
	jp z, .SetNumberThree
	cp a, 4
	jp z, .SetNumberFour
	cp a, 5
	jp z, .SetNumberFive
	cp a, 6
	jp z, .SetNumberSix
	cp a, 7
	jp z, .SetNumberSeven
	cp a, 8
	jp z, .SetNumberEight
	cp a, 9
	jp .SetNumberNine

	
.SetNumberZero:
	ld d, ALPHABETZEROTOP
	ld e, ALPHABETZEROBOT
	jp .FinishNumbers
.SetNumberOne:
	ld d, ALPHABETONETOP
	ld e, ALPHABETONEBOT
	jp .FinishNumbers

.SetNumberTwo:
	ld d, ALPHABETTWOTOP
	ld e, ALPHABETTWOBOT
	jp .FinishNumbers

.SetNumberThree:
	ld d, ALPHABETTHREETOP
	ld e, ALPHABETTHREEBOT
	jp .FinishNumbers

.SetNumberFour:
	ld d, ALPHABETFOURTOP
	ld e, ALPHABETFOURBOT
	jp .FinishNumbers

.SetNumberFive:
	ld d, ALPHABETFIVETOP
	ld e, ALPHABETFIVEBOT
	jp .FinishNumbers

.SetNumberSix:
	ld d, ALPHABETSIXTOP
	ld e, ALPHABETSIXBOT
	jp .FinishNumbers

.SetNumberSeven:
	ld d, ALPHABETSEVENTOP
	ld e, ALPHABETSEVENBOT
	jp .FinishNumbers

.SetNumberEight:
	ld d, ALPHABETEIGHTTOP
	ld e, ALPHABETEIGHTBOT
	jp .FinishNumbers

.SetNumberNine:
	ld d, ALPHABETNINETOP
	ld e, ALPHABETNINEBOT
.FinishNumbers:

	inc hl
	inc hl
	ld [hl], d
	inc hl
	inc hl
	inc hl
	inc hl
	ld [hl], e
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
	ld de, TitleScreenTiles				; load the address of the tiles into the DE register
	ld hl, $9000				; load the beginning VRAM address into HL (HL is easy to inc/dec)
	ld bc, TitleScreenTilesEnd - TitleScreenTiles		; load the length of Tiles into BC
	call Memcopy				; call the memcopy subroutine

	; The above tile loading will clobber the tilemap in VRAM, but for now just load the other half of tiles
	
	; Copy tile data into VRAM
	;ld de, (TitleScreenTiles + $0800)				; load the address of the tiles into the DE register
	;ld hl, $8800				; load the beginning VRAM address into HL (HL is easy to inc/dec)
	;ld bc, TitleScreenTilesEnd - (TitleScreenTiles + $0800)		; load the length of Tiles into BC
	;call Memcopy				; call the memcopy subroutine

	; Copy tilemap data into VRAM (functionally identical to above but pointing to tilemap data and addresses)
	ld de, TitleScreenTilemap
	ld hl, $9800
	ld bc, TitleScreenTilemapEnd - TitleScreenTilemap
	call Memcopy				; call the memcopy subroutine
	
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

call UpdateKeys

.CheckAPressed:
	ld a, [wCurKeys]
	and a, PADF_A
	jp nz, .APressed
	
	ld a, [wCurKeys]
	and a, PADF_B
	jp nz, .APressed
	
	ld a, [wCurKeys]
	and a, PADF_START
	jp nz, .APressed
	
	jp StartMenuMain
.APressed:
	; Cycle sound to clear all playing sounds
	call DisableSound
	call EnableSound
	ld b, 31					; wait a half second to insure against
	call WaitFrames				; double pressing
	;jp MainGameStart

jp StartMenuMain



;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;	DATA
;;	BLOCK
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

SECTION "Graphics Data", ROMX

AlphabetTiles: INCBIN "gfx/Alphabet.2bpp"
AlphabetTilesEnd:

SpriteData: INCBIN "gfx/spritesheettest.2bpp"
	
SpriteDataEnd:

	; TODO: Come up with a way to load all 240 tiles at once
	; Seems like the tile map generator expects to play the title screen
	; at memory location $8000, so make sure to flip LCDC bit 4

TitleScreenTiles: INCBIN "gfx/backgrounds/TitleScreenMockup.2bpp"
TitleScreenTilesEnd:
TitleScreenTilemap:  INCBIN "gfx/backgrounds/TitleScreenMockup.tilemap"
TitleScreenTilemapEnd:

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