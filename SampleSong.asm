include "include/hUGE.inc"

SECTION "sample_song Song Data", ROMX

sample_song::
db 7
dw order_cnt
dw order1, order2, order3, order4
dw duty_instruments, wave_instruments, noise_instruments
dw routines
dw waves

order_cnt: db 2
order1: dw P0
order2: dw P1
order3: dw P2
order4: dw P3

P0:
 dn ___,0,$8FF
 dn G#5,10,$C48
 dn G#5,10,$C48
 dn D#6,10,$C48
 dn G#5,10,$C48
 dn F#6,10,$C48
 dn G#6,10,$C48
 dn G#5,10,$C48
 dn G#6,10,$C48
 dn D#6,10,$C48
 dn G#5,10,$C48
 dn A#6,10,$C48
 dn D#6,10,$C48
 dn B_6,10,$C48
 dn A#6,10,$C48
 dn E_6,10,$C48
 dn B_6,10,$C48
 dn G#5,10,$C48
 dn E_6,10,$C48
 dn E_6,10,$C48
 dn G#5,10,$C48
 dn E_6,10,$C48
 dn D#6,10,$C48
 dn B_5,10,$C48
 dn E_6,10,$C48
 dn C#6,10,$C48
 dn B_5,10,$C48
 dn D#6,10,$C48
 dn C#6,10,$C48
 dn F#6,10,$C48
 dn G#6,10,$C48
 dn G#5,10,$C48
 dn G#6,10,$C48
 dn G#5,10,$C48
 dn G#5,10,$C48
 dn D#6,10,$C48
 dn G#5,10,$C48
 dn F#6,10,$C48
 dn G#6,10,$C48
 dn G#5,10,$C48
 dn G#6,10,$C48
 dn D#6,10,$C48
 dn G#5,10,$C48
 dn A#6,10,$C48
 dn D#6,10,$C48
 dn B_6,10,$C48
 dn A#6,10,$C48
 dn C#7,10,$C48
 dn B_6,10,$C48
 dn G#5,10,$C48
 dn C#7,10,$C48
 dn G#6,10,$C48
 dn G#5,10,$C48
 dn A#6,10,$C48
 dn D#6,10,$C48
 dn F#6,10,$C48
 dn D#6,10,$C48
 dn C#6,10,$C48
 dn F#6,10,$C48
 dn D#6,10,$C48
 dn C#6,10,$C48
 dn F#6,10,$C48
 dn A#6,10,$C48
 dn B_6,10,$C48

P1:
 dn G#5,10,$F07
 dn G#5,10,$C48
 dn D#6,10,$000
 dn G#5,10,$C48
 dn F#6,10,$000
 dn G#6,10,$000
 dn G#5,10,$000
 dn G#6,10,$C48
 dn D#6,10,$000
 dn G#5,10,$C48
 dn A#6,10,$000
 dn D#6,10,$C48
 dn B_6,10,$000
 dn A#6,10,$C48
 dn E_6,10,$000
 dn B_6,10,$C48
 dn G#5,10,$000
 dn E_6,10,$C48
 dn E_6,10,$000
 dn G#5,10,$C48
 dn E_6,10,$000
 dn D#6,10,$000
 dn B_5,10,$000
 dn E_6,10,$C48
 dn C#6,10,$000
 dn B_5,10,$C48
 dn D#6,10,$000
 dn C#6,10,$C48
 dn F#6,10,$000
 dn G#6,10,$000
 dn G#5,10,$000
 dn G#6,10,$C48
 dn G#5,10,$000
 dn G#5,10,$C48
 dn D#6,10,$000
 dn G#5,10,$C48
 dn F#6,10,$000
 dn G#6,10,$000
 dn G#5,10,$000
 dn G#6,10,$C48
 dn D#6,10,$000
 dn G#5,10,$C48
 dn A#6,10,$000
 dn D#6,10,$C48
 dn B_6,10,$000
 dn A#6,10,$C48
 dn C#7,10,$000
 dn B_6,10,$C48
 dn G#5,10,$000
 dn C#7,10,$C48
 dn G#6,10,$000
 dn G#5,10,$C48
 dn A#6,10,$000
 dn D#6,10,$000
 dn F#6,10,$000
 dn D#6,10,$C48
 dn C#6,10,$000
 dn F#6,10,$C48
 dn D#6,10,$000
 dn C#6,10,$C48
 dn F#6,10,$000
 dn A#6,10,$000
 dn B_6,10,$000
 dn A#6,10,$C48

P2:
 dn G#3,7,$4A1
 dn ___,0,$4A2
 dn ___,0,$400
 dn ___,0,$400
 dn ___,0,$400
 dn ___,0,$400
 dn G#4,7,$330
 dn ___,0,$330
 dn G#3,7,$4A1
 dn ___,0,$4A2
 dn ___,0,$400
 dn G#3,7,$000
 dn ___,0,$000
 dn ___,0,$000
 dn D#4,7,$000
 dn ___,0,$000
 dn E_4,7,$4A1
 dn ___,0,$4A2
 dn ___,0,$400
 dn ___,0,$400
 dn E_4,7,$400
 dn ___,0,$400
 dn C#4,7,$4A1
 dn ___,0,$4A2
 dn ___,0,$400
 dn ___,0,$400
 dn C#4,7,$000
 dn ___,0,$000
 dn ___,0,$000
 dn ___,0,$000
 dn F#4,7,$000
 dn ___,0,$000
 dn G#3,7,$4A1
 dn ___,0,$4A2
 dn ___,0,$400
 dn ___,0,$400
 dn ___,0,$400
 dn ___,0,$400
 dn G#4,7,$330
 dn ___,0,$330
 dn G#3,7,$4A1
 dn ___,0,$4A2
 dn ___,0,$400
 dn G#3,7,$000
 dn ___,0,$000
 dn ___,0,$000
 dn D#4,7,$000
 dn ___,0,$000
 dn E_4,7,$4A1
 dn ___,0,$4A2
 dn ___,0,$400
 dn ___,0,$400
 dn E_4,7,$400
 dn ___,0,$400
 dn F#4,7,$330
 dn ___,0,$330
 dn ___,0,$4A1
 dn ___,0,$4A2
 dn F#4,7,$400
 dn ___,0,$400
 dn ___,0,$400
 dn ___,0,$400
 dn D#4,7,$320
 dn ___,0,$320

P3:
 dn D#8,3,$000
 dn ___,0,$000
 dn D#8,5,$000
 dn D#8,2,$000
 dn D#8,4,$000
 dn ___,0,$000
 dn D#8,3,$000
 dn ___,0,$000
 dn D#8,3,$000
 dn ___,0,$000
 dn D#8,5,$000
 dn D#8,2,$000
 dn D#8,4,$000
 dn ___,0,$000
 dn D#8,5,$000
 dn D#8,4,$000
 dn D#8,3,$000
 dn ___,0,$000
 dn D#8,5,$000
 dn D#8,2,$000
 dn D#8,4,$000
 dn ___,0,$000
 dn D#8,5,$000
 dn D#8,2,$000
 dn D#8,3,$000
 dn D#8,3,$000
 dn D#8,5,$000
 dn D#8,3,$000
 dn D#8,4,$000
 dn ___,0,$000
 dn D#8,5,$000
 dn D#8,4,$000
 dn D#8,3,$000
 dn ___,0,$000
 dn D#8,5,$000
 dn D#8,2,$000
 dn D#8,4,$000
 dn ___,0,$000
 dn D#8,3,$000
 dn ___,0,$000
 dn D#8,3,$000
 dn ___,0,$000
 dn D#8,5,$000
 dn D#8,2,$000
 dn D#8,4,$000
 dn ___,0,$000
 dn D#8,5,$000
 dn D#8,4,$000
 dn D#8,3,$000
 dn ___,0,$000
 dn D#8,5,$000
 dn D#8,2,$000
 dn D#8,4,$000
 dn ___,0,$000
 dn D#8,5,$000
 dn D#8,2,$000
 dn D#8,3,$000
 dn D#8,3,$000
 dn D#8,5,$000
 dn D#8,3,$000
 dn D#8,4,$000
 dn D#8,4,$000
 dn D#8,5,$000
 dn D#8,4,$000

duty_instruments:
itSquareinst1:
db 8
db 128
db 240
dw 0
db 128

itSquareinst2:
db 8
db 128
db 240
dw 0
db 128

itSquareinst3:
db 8
db 128
db 240
dw 0
db 128

itSquareinst4:
db 8
db 128
db 240
dw 0
db 128

itSquareinst5:
db 8
db 128
db 240
dw 0
db 128

itSquareinst6:
db 8
db 128
db 240
dw 0
db 128

itSquareinst7:
db 8
db 128
db 240
dw 0
db 128

itSquareinst8:
db 8
db 128
db 240
dw 0
db 128

itSquareinst9:
db 8
db 128
db 240
dw 0
db 128

itSquareinst10:
db 8
db 128
db 242
dw 0
db 128



wave_instruments:
itWaveinst1:
db 0
db 32
db 0
dw 0
db 128

itWaveinst2:
db 0
db 32
db 1
dw 0
db 128

itWaveinst3:
db 0
db 32
db 2
dw 0
db 128

itWaveinst4:
db 0
db 32
db 3
dw 0
db 128

itWaveinst5:
db 0
db 32
db 4
dw 0
db 128

itWaveinst6:
db 0
db 32
db 5
dw 0
db 128

itWaveinst7:
db 0
db 32
db 0
dw 0
db 128



noise_instruments:
itNoiseinst1:
db 240
dw 0
db 0
ds 2

itNoiseinst2:
db 240
dw 0
db 102
ds 2

itNoiseinst3:
db 240
dw 0
db 104
ds 2

itNoiseinst4:
db 240
dw 0
db 68
ds 2

itNoiseinst5:
db 241
dw 0
db 46
ds 2



routines:
__hUGE_Routine_0:

__end_hUGE_Routine_0:
ret

__hUGE_Routine_1:

__end_hUGE_Routine_1:
ret

__hUGE_Routine_2:

__end_hUGE_Routine_2:
ret

__hUGE_Routine_3:

__end_hUGE_Routine_3:
ret

__hUGE_Routine_4:

__end_hUGE_Routine_4:
ret

__hUGE_Routine_5:

__end_hUGE_Routine_5:
ret

__hUGE_Routine_6:

__end_hUGE_Routine_6:
ret

__hUGE_Routine_7:

__end_hUGE_Routine_7:
ret

__hUGE_Routine_8:

__end_hUGE_Routine_8:
ret

__hUGE_Routine_9:

__end_hUGE_Routine_9:
ret

__hUGE_Routine_10:

__end_hUGE_Routine_10:
ret

__hUGE_Routine_11:

__end_hUGE_Routine_11:
ret

__hUGE_Routine_12:

__end_hUGE_Routine_12:
ret

__hUGE_Routine_13:

__end_hUGE_Routine_13:
ret

__hUGE_Routine_14:

__end_hUGE_Routine_14:
ret

__hUGE_Routine_15:

__end_hUGE_Routine_15:
ret

waves:
wave0: db 119,117,0,0,0,0,0,0,255,254,238,238,236,34,23,118

