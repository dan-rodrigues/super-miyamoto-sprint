### Build config:

# This must be set according to the prefix of the RISC-V toolchain
CROSS = riscv-none-embed-
 
### Options:

# Enables music playback and the extra dependencies needed
MUSIC ?= 0

# Enables some onscreen debug diagnostics
DEBUG_PRINT ?= 0

###

ICS32_SW_DIR = icestation-32/software/

SOURCES = \
	main.c \
	game_loop.c \
	hero/hero.c \
	hero/hero_drawing.c \
	hero/hero_init.c \
	hero/hero_life_meter.c \
	sprites/sprite_buffer.c \
	sprites/sprite_position.c \
	sprites/sprite_actor.c \
	sprites/sprite_actor_handle.c \
	sprites/sprite_block_interaction.c \
	sprites/sprite_collision.c \
	sprites/sprite_drawing.c \
	sprites/sprite_loading.c \
	sprites/sprite_text.c \
	sprites/actors/smoke_sprite.c \
	sprites/actors/moving_smoke_sprite.c \
	sprites/actors/basic_enemy_sprite.c \
	sprites/actors/impact_sprite.c \
	sprites/actors/enemy_generator_sprite.c \
	sprites/actors/layered_enemy_sprite.c \
	sprites/actors/platform_sprite.c \
	sprites/actors/ball_enemy_sprite.c \
	sprites/actors/glitter_sprite.c \
	sprites/actors/tank_sprite.c \
	sprites/actors/tank_driver_sprite.c \
	sprites/actors/missile_sprite.c \
	sprites/actors/spark_sprite.c \
	sprites/actors/jumping_enemy_sprite.c \
	sprites/actors/jetpack_enemy_sprite.c \
	sprites/actors/goal_sprite.c \
	sprites/actors/midpoint_sprite.c \
	tasks/extra_task.c \
	tasks/extra_task_handle.c \
	tasks/level_reload_sequence_task.c \
	tasks/fade_task.c \
	tasks/palette_lerp_task.c \
	level/camera.c \
	level/camera_init.c \
	level/block.c \
	level/level_attributes.c \
	timers/global_timers.c \
	vram/vram_command_queue.c \
	vram/vram_level_update.c \
	vram/vram_layout.c \
	vram/vram_animated_tiles.c \
	vram/vram_address.c \
	palette/palette_buffer.c \
	audio/music.c \
	audio/sound_effects.c \
	audio/audio_command_queue.c \
	debug/debug_block.c \
	debug/debug_print.c \
	debug/debug_playfield.c \
	gcc_lib/gcc_lib.c \

LIB_SOURCES := \
	vdp.c \
	math_util.c \
	assert.c \
	vdp_print.c \
	audio.c \
	gamepad.c

SOURCES += $(addprefix $(ICS32_SW_DIR)common/, \
	font.c \
	tinyprintf.c \
	)

CPU_SLOW_SOURCES := \
	debug/debug_custom_assert.c \
	palette/palette_init.c \
	level/level_loading.c

CPU_SLOW_SOURCES += $(addprefix $(ICS32_SW_DIR)common/, \
	font.c \
	tinyprintf.c \
	)

###

LEVEL_DIR = assets/levels/
LEVEL_BIN := $(addprefix $(LEVEL_DIR), \
	level.bin \
	block_map_table.bin \
	block_attributes.bin \
	sprites.bin \
	)
LEVEL_SOURCES := $(LEVEL_BIN:%.bin=%.c)

###

MAPS_DIR = assets/maps/
MAPS_BIN := $(addprefix $(MAPS_DIR), \
	bg_hills_no_clouds.bin \
	bg_hills_clouds.bin \
	)
MAPS_SOURCES := $(MAPS_BIN:%.bin=%.c)

### Graphics assets

GFX_DIR = assets/graphics/

PNGS := $(addprefix $(GFX_DIR), \
	miyamoto.png \
	animated.png \
	spr00.png \
	spr01.png \
	spr02.png \
	spr04.png \
	spr06_07.png \
	fg_bg.png \
	)

PNG_TILES := $(PNGS:$(GFX_DIR)%.png=$(GFX_DIR)%_tiles.bin)
PNG_PALETTES := $(PNGS:$(GFX_DIR)%.png=$(GFX_DIR)%_palette.bin)

GFX_TILES := $(PNG_TILES)
GFX_PALETTES := $(PNG_PALETTES)

ARTEFACTS += $(PNG_TILES) $(PNG_PALETTES)

GFX_STANDALONE_PALETTES := $(addprefix $(GFX_DIR), \
	coin_palette.bin \
	bg_palette.bin \
	fgbg_palette.bin \
	cloud_palette.bin \
	bush_palette.bin \
	blue_palette.bin \
	gray_palette.bin \
	)
GFX_PALETTES += $(GFX_STANDALONE_PALETTES)

GFX_SOURCES := $(GFX_TILES:$(GFX_DIR)%_tiles.bin=$(GFX_DIR)%_tiles.c)
GFX_SOURCES += $(GFX_PALETTES:$(GFX_DIR)%_palette.bin=$(GFX_DIR)%_palette.c)

### Palette assets

PAL_DIR = assets/palettes/

FG_PAL = $(PAL_DIR)hills.pal
REDRAWN_PAL = $(PAL_DIR)redrawn.pal

### Audio assets

AUDIO_ASSETS_DIR = assets/audio/
WAV_FILES := $(addprefix $(AUDIO_ASSETS_DIR), \
	coin.wav \
	jump.wav \
	thud.wav \
	stomp.wav \
	hurt.wav \
	launch.wav \
	explosion.wav \
	powerup.wav \
	dead.wav \
	)

WAV_MUSIC_FILES := $(addprefix $(AUDIO_ASSETS_DIR), \
	track1_left.wav \
	track1_right.wav \
	)

ifeq ($(MUSIC), 1)
WAV_FILES += $(WAV_MUSIC_FILES)
endif

ADPCM_FILES := $(WAV_FILES:%.wav=%.adpcm)

ARTEFACTS += $(ADPCM_FILES)

# Flash assets defined according to what component must access them:
# These must be set before core.mk is included

FLASH_ADPCM_SOURCES := $(ADPCM_FILES:%.adpcm=%.c)
FLASH_CPU_SOURCES := $(GFX_SOURCES) $(LEVEL_SOURCES) $(MAPS_SOURCES) $(CPU_SLOW_SOURCES)

ARTEFACTS += $(FLASH_ADPCM_SOURCES)

### Core rules

HEADER_GEN_NEEDED = 1
GFX_CONVERT_NEEDED = 1

ICS32_DIR = icestation-32/
include $(ICS32_DIR)software/common/core.mk

$(eval $(call persisted_var,MUSIC))
$(eval $(call persisted_var,DEBUG_PRINT))

CFLAGS_INCLUDE_DIRS := $(addprefix -I, \
	./ \
	hero/ \
	sprites/ \
	sprites/actors/ \
	level/ \
	vram/ \
	debug/ \
	audio/ \
	palette/ \
	assets/audio/ \
	gcc_lib/ \
	assets/graphics/ \
	assets/misc/ \
	assets/levels/ \
	assets/maps/ \
	timers/ \
	tasks/ \
	)

EXTRA_CFLAGS := $(CFLAGS_INCLUDE_DIRS)

ifeq ($(MUSIC), 1)
EXTRA_CFLAGS += -DMUSIC_INCLUDED
endif

ifeq ($(DEBUG_PRINT), 1)
EXTRA_CFLAGS += -DDEBUG_PRINT
endif

###

CFLAGS += $(EXTRA_CFLAGS)
RC_CFLAGS += $(EXTRA_CFLAGS)

###

# These headers don't exist at first and must be generated first

hero/hero_drawing.o: $(GFX_DIR)miyamoto_tiles.h
palette/palette_init.o: $(addprefix $(GFX_DIR), \
	spr00_palette.h spr01_palette.h spr02_palette.h \
	coin_palette.h cloud_palette.h bush_palette.h \
	blue_palette.h \
	gray_palette.h \
	miyamoto_palette.h \
	fgbg_palette.h \
	bg_palette.h \
	)
vram/vram_animated_tiles.o: $(GFX_DIR)animated_tiles.h

level/level_loading.o: $(addprefix $(GFX_DIR), spr00_tiles.h spr01_tiles.h spr02_tiles.h spr06_07_tiles.h fg_bg_tiles.h)
level/level_loading.o: $(MAPS_DIR)bg_hills_no_clouds.h $(MAPS_DIR)bg_hills_clouds.h
level/level_attributes.o: $(LEVEL_DIR)level.h $(LEVEL_DIR)sprites.h
level/block.o: $(LEVEL_DIR)block_map_table.h $(LEVEL_DIR)block_attributes.h
game_loop.o: $(LEVEL_DIR)block_map_table.h

ifeq ($(MUSIC), 1)
audio/music.o: $(addprefix $(AUDIO_ASSETS_DIR), track1_left.h track1_right.h)
endif

audio/music.o: MUSIC

debug/debug_print.o: DEBUG_PRINT
palette/palette_init.o: DEBUG_PRINT
level/level_loading.o: DEBUG_PRINT

audio/sound_effects.o: $(WAV_FILES:%.wav=%.o)

debug/debug_custom_assert.o: $(ASSERT_ENABLED_FLAG_PATH)

$(GFX_DIR)%_tiles.bin $(GFX_DIR)%_palette.bin: $(GFX_DIR)%.png $(GFX_CONVERT)
	$(GFX_CONVERT) -f png -o $(GFX_DIR)$*_ $<

### SNES graphics, separate rules for these due to differing palette numbers

$(GFX_DIR)fgbg_palette.bin: $(REDRAWN_PAL) $(GFX_CONVERT)
	$(GFX_CONVERT) -f snes -p $(REDRAWN_PAL) -i 2 --palette-output $@

$(GFX_DIR)bush_palette.bin: $(REDRAWN_PAL) $(GFX_CONVERT)
	$(GFX_CONVERT) -f snes -p $(REDRAWN_PAL) -i 5 --palette-output $@

$(GFX_DIR)coin_palette.bin: $(REDRAWN_PAL) $(GFX_CONVERT)
	$(GFX_CONVERT) -f snes -p $(REDRAWN_PAL) -i 6 --palette-output $@ 

# Map converting

$(MAPS_DIR)%.bin: $(MAPS_DIR)%_snes.bin $(GFX_CONVERT)
	$(GFX_CONVERT) --snes-vram-offset 0000 --map-output $@ $<

$(GFX_DIR)bg_palette.bin: $(FG_PAL) $(GFX_CONVERT)
	$(GFX_CONVERT) -f snes -p $(FG_PAL) -i 0 --palette-output $@

$(GFX_DIR)cloud_palette.bin: $(FG_PAL) $(GFX_CONVERT)
	$(GFX_CONVERT) -f snes -p $(FG_PAL) -i 4 --palette-output $@

$(GFX_DIR)blue_palette.bin: $(REDRAWN_PAL) $(GFX_CONVERT)
	$(GFX_CONVERT) -f snes -p $(REDRAWN_PAL) -i 7 --palette-output $@

$(GFX_DIR)gray_palette.bin: $(REDRAWN_PAL) $(GFX_CONVERT)
	$(GFX_CONVERT) -f snes -p $(REDRAWN_PAL) -i 3 --palette-output $@

# Sprites level data specific, 8bit

$(LEVEL_DIR)sprites.c $(LEVEL_DIR)sprites.h: $(LEVEL_DIR)sprites.bin $(HEADER_GEN)
	$(HEADER_GEN) -t uint8_t -s -i sprites -o $(@D)/$(*F) $<

# Other binary includes

%.c %.h: %.bin $(HEADER_GEN)
	$(HEADER_GEN) -t uint16_t -s -i $(basename $(<F)) -o $(@D)/$(*F) $<

# ADPCM converter tool (adpcm-xq)

CC = gcc

ADPCM_XQ_DIR = utility/adpcm-xq/
ADPCM_XQ_SOURCES := $(addprefix $(ADPCM_XQ_DIR),\
	adpcm-lib.c \
	adpcm-xq.c \
	)
ADPCM_XQ := $(ADPCM_XQ_DIR)adpcm-xq 

$(ADPCM_XQ): $(ADPCM_XQ_SOURCES) $(ADPCM_XQ_DIR)adpcm-lib.h
	$(CC) -O3 $(ADPCM_XQ_SOURCES) -o $@	

### ADPCM conversion:

# Let ffmpeg do the track conversion rather than adpcm-xq:

$(AUDIO_ASSETS_DIR)track%.adpcm: $(AUDIO_ASSETS_DIR)track%.wav
	ffmpeg -y -guess_layout_max 0 -i $^ -f s16le -acodec adpcm_ima_wav -ac 1 $@

%.adpcm: %.wav $(ADPCM_XQ)
	$(ADPCM_XQ) -r -y $< $@

# Alternate rule that would use ffmpeg to do all conversion:
#
#%.adpcm: %.wav 
#	ffmpeg -y -guess_layout_max 0 -i $^ -f s16le -acodec adpcm_ima_wav -ac 1 $@
	
%.c %.h: %.adpcm $(HEADER_GEN)
	$(HEADER_GEN) -t int16_t -s -i $(basename $(<F)) -o $(@D)/$(*F) $<

# Optional music

track1.opus:
	youtube-dl -f bestaudio -x --audio-format opus -o "track1.%(ext)s" -k https://www.youtube.com/watch?v=AhSqF0mYz-A

TRACK1_L_WAV := $(AUDIO_ASSETS_DIR)track1_left.wav
TRACK1_R_WAV := $(AUDIO_ASSETS_DIR)track1_right.wav

$(TRACK1_L_WAV) $(TRACK1_R_WAV): track1.opus
	ffmpeg -y -i $< \
	-ar 22050 \
	-map_channel 0.0.0 $(TRACK1_L_WAV) \
	-ar 22050 \
	-map_channel 0.0.1 $(TRACK1_R_WAV)

