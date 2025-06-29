SECTION "Sound Subroutines", ROM0

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

sound_character_text: ; give the sound a name in the code
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
	DB %00111101	; Data to be written to SOUND_CH4_LENGTH
	DB %11110111	; Data to be written to SOUND_CH4_ENVELOPE
	DB %01011101	; Data to be written to SOUND_CH4_POLY
	DB %11000110	; Data to be written to SOUND_CH4_Options