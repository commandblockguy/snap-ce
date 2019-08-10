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

/* Get the height of an element */
/* next will be set to the pointer to the next element, if non-null */
/* Cache is a height cache, which should be, if non-NULL, at least the length of the element */
uint24_t getHeight(scriptElem_t *elem, scriptElem_t **next, uint24_t *cache);

/* Get the width of an element */
/* next will be set to the pointer to the next element, if non-null */
/* Cache is a width cache, which should be, if non-NULL, at least the length of the element */
/* For blocks with C blocks, this includes the width of the elements inside the C block */
uint24_t getWidth(scriptElem_t *elem, scriptElem_t **next, uint24_t *cache);

/* Bits 0-3: block category */
/* Bit 4: whether to use alternative color in zebra case */
/* Bit 5: 1 if darker color */
#define COLOR_ALT  (1 << 4)
#define COLOR_DARK (1 << 5)
typedef uint8_t blockColor_t;

/* For blocks and scripts, y refers to the top */
/* Otherwise, it refers to the center */
/* Returns false if error */
/* Parent color */
/* next will be set to the pointer to the next element, if non-null */
/* csrOver is currently unused */
/* widthCache and heightCache are caches, which, if non-null, should be at least the length of the element */
bool drawElem(scriptElem_t *elem, int24_t x, int24_t y, blockColor_t parentColor, scriptElem_t **next, bool *csrOver, uint24_t *widthCache, uint24_t *heightCache);
bool drawScript(scriptElem_t *elem, int24_t x, int24_t y, bool *csrOver, uint24_t *widthCache, uint24_t *heightCache);

#endif
