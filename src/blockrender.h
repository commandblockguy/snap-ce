#ifndef H_BLOCKRENDER
#define H_BLOCKRENDER

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "script.h"

uint24_t getHeight(scriptElem_t *elem, scriptElem_t **next, uint24_t *cache);

uint24_t getWidth(scriptElem_t *elem, scriptElem_t **next, uint24_t *cache);

/* Bits 0-3: block type */
/* Bit 4: whether to use alternative color in zebra case */
/* Bit 5: 1 if darker color */
#define COLOR_ALT  (1 << 4)
#define COLOR_DARK (1 << 5)
typedef uint8_t blockColor_t;

/* For blocks and scripts, y refers to the top */
/* Otherwise, it refers to the center */
/* Returns false if error */
//scriptElem_t *drawElem(scriptElem_t *elem, int24_t x, int24_t y, uint24_t width, uint24_t height, blockColor_t parentColor);

bool drawElem(scriptElem_t *elem, int24_t x, int24_t y, blockColor_t parentColor, scriptElem_t **next, bool *csrOver, uint24_t *widthCache, uint24_t *heightCache);

void drawScript(scriptElem_t *elem, int24_t x, int24_t y);

//#define DBG_DRAW

#endif
