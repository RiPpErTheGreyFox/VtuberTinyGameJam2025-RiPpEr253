PNG_FILES := $(wildcard gfx/*.png)
2BPP_FILES := $(PNG_FILES:.png=.2bpp)
BACKGROUNDPNG_FILES := $(wildcard gfx/backgrounds/*.png)
BACKGROUND2BPP_FILES := $(BACKGROUNDPNG_FILES:.png=.2bpp)
TILEMAP_FILES := $(BACKGROUNDPNG_FILES:.png=.tilemap)

RGBDS ?=
RGBASM  ?= $(RGBDS)rgbasm
RGBFIX  ?= $(RGBDS)rgbfix
RGBGFX  ?= $(RGBDS)rgbgfx
RGBLINK ?= $(RGBDS)rgblink

%.2bpp: %.png
	$(RGBGFX) $(rgbgfx) -c "#FFFFFF,#aaaaaa,#555555,#000000;" -u -o $@ $<

%.tilemap: %.png
	$(RGBGFX) -c "#FFFFFF,#aaaaaa,#555555,#000000;" \
		--unique-tiles \
		--tilemap $@ \
		$<

all: 2bpp backgrounds tilemaps
	rgbasm -o VTuberGameJam2025.o VTuberGameJam2025.asm
	rgbasm -o hUGEDriver.o hUGEDriver.asm
	rgbasm -o MainMenuMusic.o music/MainMenuMusic.asm
#	rgblink -o VTuberGameJam2025.gb VTuberGameJam2025.o hUGEDriver.o SampleSong.o
	rgblink -t -m VTuberGameJam2025.map -n VTuberGameJam2025.sym -o VTuberGameJam2025.gb VTuberGameJam2025.o hUGEDriver.o MainMenuMusic.o
	rgbfix -j -l 0x33 -k "R2" -n 0x01 -s -t "VTuberGameJam2025" -v  -p 0xFF VTuberGameJam2025.gb

2bpp: $(2BPP_FILES)

backgrounds: $(BACKGROUND2BPP_FILES)
tilemaps: $(TILEMAP_FILES)

clean:
	rm gfx/*.2bpp
	rm gfx/backgrounds/*.2bpp
	rm gfx/backgrounds/*.tilemap
	rm *.o *.gb
