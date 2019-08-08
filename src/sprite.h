#ifndef H_SPRITE
#define H_SPRITE

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <graphx.h>

typedef struct Sprite {
	int24_t x;
	int24_t y;
	uint8_t rotation;
	uint8_t size;
	bool shown;
	uint8_t currentCostume;
	uint8_t numCostumes;
	gfx_sprite_t *costumes;
	bool penDown;
	uint24_t penHue;
	char *sayText;
	bool thinking;
} sprite_t;

#endif