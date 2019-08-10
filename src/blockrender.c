#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <graphx.h>

#include <debug.h>

#include "script.h"
#include "blockrender.h"

#include "gfx/gfx_group.h"

#define TEXT_HEIGHT		  8
#define LEFT_MARGIN		  6 // Distance from the left side of a block before text starts
#define RIGHT_MARGIN	  3 // Distance between the last argument in a block and the right edge
#define ARG_SPACING		  4 // Space between two arguments of a block
#define PRED_CAP_WIDTH	  5 // The width of the triangular part at both ends of a predicate
#define NOTCH_DEPTH		  3 // Height of the notch on a block
#define NOTCH_OFFSET	 10 // Pixels from the left of a block before the notch starts
#define NOTCH_SIZE		(10 + 2 * NOTCH_DEPTH) // Width of the notch, including the sloped sides
#define HAT_HEIGHT		  9 // Height of the curved section of a hat block
#define HAT_MIN_WIDTH	 69 // Width of the curved section of a hat block

#define COLOR_TRUE 0x45  // Color used to represent true  in Boolean literals
#define COLOR_FALSE 0xC9 // Color used to represent false in Boolean literals
#define COLOR_BOOL_SELECT 0xDE // Color of the circle in the Boolean literal's toggle

#define COLOR_LIGHT_ALT 0xD6

uint24_t getMaxHeight(scriptElem_t *elem, scriptElem_t **next, uint24_t *cache);
uint24_t getTotalHeight(scriptElem_t *elem, scriptElem_t **next, uint24_t *cache);
uint24_t getMaxWidth(scriptElem_t *elem, scriptElem_t **next, uint24_t *cache);
uint24_t getTotalHeight(scriptElem_t *elem, scriptElem_t **next, uint24_t *cache);

/* Get a graphx color from a blockColor */
uint8_t getColor(blockColor_t col) {
	/* colors is a 16x4 sprite */
	return colors->data[col];
}

/* Get a lighter color with the same hue */
uint8_t getLightColor(blockColor_t col) {
	/* If already the alt color, get the generic light color for alt colors */
	if(col & COLOR_ALT) return COLOR_LIGHT_ALT;
	/* Otherwise, get the value of the alt color */
	return getColor(col | COLOR_ALT);
}

/* Get a darker color with the same hue */
uint8_t getDarkColor(blockColor_t col) {
	return getColor(col | COLOR_DARK);
}

/* Finds the tallest elem in a block */
uint24_t getMaxHeight(scriptElem_t *elem, scriptElem_t **next, uint24_t *cache) {
	uint24_t height = 8; /* If there are no elements taller than 8px, use 8px */

	/* Ensure that we don't exit the script somehow */
	while((*next)->type != END_SCRIPT) {
		uint24_t newHeight;
		/* Break if we are at the end of the block */
		if((*next)->type == BLOCK_END && (*next)->data == (void*)elem) break;

		/* Get the height of the next element */
		newHeight = getHeight(*next, next, cache ? cache + (*next - elem) : NULL);

		/* Update the maximum height */
		if(newHeight > height) height = newHeight;
	}

	/* Point at the element after BLOCK_END */
	*next++;

	return height;
}

uint24_t getHeight(scriptElem_t *elem, scriptElem_t **next, uint24_t *cache) {
	uint24_t height;

	/* If we don't care about the next element, change next to point to some temp memory */
	scriptElem_t *tmp;
	if(!next) next = &tmp;
	*next = elem + 1;

	/* If there is already a cached version, just return that */
	if(cache && *cache) {
		return *cache;
	}

	switch(elem->type) {
		case STRING_LITERAL:
		case FLOAT_LITERAL:
			height = 4 + TEXT_HEIGHT;
			break;

		case ON_GREEN_FLAG:
		case ON_KEY:
		case ON_CLICK:
		case ON_CLONE:
			height = 24;
			break;

		case BOOLEAN_LITERAL:
			height = 8;
			break;

		case TITLE_TEXT:
			height = TEXT_HEIGHT;
			break;

		case BLOCK_START:
		case PREDICATE_START: {
			/* Find the tallest subelement and add a border */
			height = getMaxHeight(elem, next, cache) + 6;
			break;
		}

		case BLOCK_RING_START: {
			/* Get the total height of the inner blocks */
			height = getTotalHeight(elem, next, cache) + 6;
			if(height < 14) height = 14;
			break;
		}

		default:
			height = 0;
			break;
	}

	#ifdef DBG_DRAW
	dbg_sprintf(dbgout, "height of ");
	printElemInfo(elem);
	dbg_sprintf(dbgout, ": %u\n", height);
	#endif

	/* Update the cache */
	if(cache) *cache = height;
	return height;
}

/* Finds the total height of all elements with some space between */
uint24_t getTotalHeight(scriptElem_t *elem, scriptElem_t **next, uint24_t *cache) {
	uint24_t height = 0;

	/* Ensure that we don't exit the script somehow */
	while((*next)->type != END_SCRIPT) {
		/* Break if we are at the end of the block */
		if((*next)->type == BLOCK_END && (*next)->data == (void*)elem) break;

		/* Add the width of the subelement to the total */
		height += getHeight(*next, next, cache ? cache + (*next - elem) : NULL);
	}

	/* Point at the element after BLOCK_END */
	*next++;

	return height;
}

/* Finds the total width of all elements with some space between */
uint24_t getTotalWidth(scriptElem_t *elem, scriptElem_t **next, uint24_t *cache) {
	uint24_t width = 0;

	/* Ensure that we don't exit the script somehow */
	while((*next)->type != END_SCRIPT) {
		/* Break if we are at the end of the block */
		if((*next)->type == BLOCK_END && (*next)->data == (void*)elem) break;

		/* Add the width of the subelement and the argument spacing to the total */
		width += getWidth(*next, next, cache ? cache + (*next - elem) : NULL) + ARG_SPACING;
	}

	/* Point at the element after BLOCK_END */
	*next++;

	/* We don't need argument spacing after the last argument */
	return width - ARG_SPACING;
}

uint24_t getWidth(scriptElem_t *elem, scriptElem_t **next, uint24_t *cache) {
	uint24_t width;

	/* If we don't care about the next element, change next to point to some temp memory */
	scriptElem_t *tmp;
	if(!next) next = &tmp;
	*next = elem + 1;

	/* If there is already a cached version, just return that */
	if(cache && *cache) {
		return *cache;
	}

	/* Reset the text scale */
	gfx_SetTextScale(1, 1);

	switch(elem->type) {
		case STRING_LITERAL:
			width = 4 + gfx_GetStringWidth(elem->data);
			break;

		case BOOLEAN_LITERAL:
			/* This should probably be its own constant but whatever */
			width = 3 * LEFT_MARGIN;
			break;

		case TITLE_TEXT:
			width = gfx_GetStringWidth(elem->data);
			break;

		case BLOCK_START: {
			/* Get the total width of all subelements, plus a margin */
			width = LEFT_MARGIN + RIGHT_MARGIN + getTotalWidth(elem, next, cache);
			break;
		}

		case PREDICATE_START: {
			/* Get the total width of all subelements, plus a margin */
			width = (PRED_CAP_WIDTH + 1) * 2 + getTotalWidth(elem, next, cache);
			break;
		}

		case ON_GREEN_FLAG: {
			width = LEFT_MARGIN + RIGHT_MARGIN + gfx_GetStringWidth("when  clicked") + flag->width;
			break;
		}

		case BLOCK_RING_START: {
			width = getMaxWidth(elem, next, cache) + 6;
			break;
		}

		default:
			width = 0;
			break;
	}

	#ifdef DBG_DRAW
	dbg_sprintf(dbgout, "width of ");
	printElemInfo(elem);
	dbg_sprintf(dbgout, ": %u\n", width);
	#endif

	/* Update the cache */
	if(cache) *cache = width;
	return width;
}

/* Finds the widest elem in a block */
uint24_t getMaxWidth(scriptElem_t *elem, scriptElem_t **next, uint24_t *cache) {
	uint24_t width = 0;

	/* Ensure that we don't exit the script somehow */
	while((*next)->type != END_SCRIPT) {
		uint24_t newWidth;
		/* Break if we are at the end of the block */
		if((*next)->type == BLOCK_END && (*next)->data == (void*)elem) break;

		/* Get the width of the next element */
		newWidth = getWidth(*next, next, cache ? cache + (*next - elem) : NULL);

		/* Update the maximum width */
		if(newWidth > width) width = newWidth;
	}

	/* Point at the element after BLOCK_END */
	*next++;

	return width;
}

/* Draw the funky hexagonal shape */
/* capWidth is included in width */
/* Credit to runer to making this actually symmetrical */
void drawPredicateBg(int24_t x, int24_t y, uint24_t width, uint24_t height, uint24_t capWidth) {
	uint24_t halfHeight = height / 2;
	uint24_t doubleCapWidth = capWidth * 2;
	int24_t xl = x;
	int24_t xr = x + width;
	uint24_t i;

	/* Draw the caps */
	for(i = 0; i < capWidth; i++) {
		uint24_t halfSliceHeight = ((i * 2 + 1) * halfHeight + capWidth) / doubleCapWidth;
		uint24_t sliceHeight = halfSliceHeight * 2 + (height & 1);
		int24_t sliceY = y - halfSliceHeight;
		gfx_VertLine(xl++, sliceY, sliceHeight);
		gfx_VertLine(--xr, sliceY, sliceHeight);
	}

	/* Draw the center of the hexagon */
	gfx_FillRectangle(xl, y - halfHeight, xr - xl, height);
}

void drawReporterBg(int24_t x, int24_t y, uint24_t width, uint24_t height) {
	uint24_t i;

	for(i = 0; i < 3; i++) {
		gfx_HorizLine(x + 3 - i, y + i, width - 6 + 2 * i);
		gfx_HorizLine(x + 3 - i, y + height - i - 1, width - 6 + 2 * i);
	}

	gfx_FillRectangle(x, y + 3, width, height - 6);
}

/* Draw all of the subelements of an element */
/* Caches should be non-NULL */
bool drawRecursiveElem(scriptElem_t *elem, int24_t x, int24_t y, blockColor_t col, scriptElem_t **next, bool *csrOver, uint24_t *widthCache, uint24_t *heightCache) {
	int24_t subX = x;
	int24_t subY = y;
	/* Get the first subelement */
	scriptElem_t *checkElem = elem + 1;

	/* Ensure that we don't exit the script somehow */
	while(checkElem->type != END_SCRIPT) {
		/* Break if we are at the end of the block */
		if(checkElem->type == BLOCK_END && checkElem->data == (void*)elem) break;
		/* Only render stuff that's on-screen */
		if(subX < (int24_t)LCD_WIDTH && subY < (int24_t)LCD_HEIGHT) {
			uint24_t subWidth, subHeight;
			uint8_t type = checkElem->type;
			bool error;

			/* Get the (usually cached) width and height of the subelement */
			subWidth = getWidth(checkElem, NULL, widthCache ? widthCache + (checkElem - elem) : NULL);
			subHeight = getHeight(checkElem, NULL, heightCache ? heightCache + (checkElem - elem) : NULL);
	
			/* Actually draw the subelement */
			error = drawElem(
				checkElem, subX, subY, col, &checkElem, NULL,
				widthCache ? widthCache + (checkElem - elem) : NULL,
				heightCache ? heightCache + (checkElem - elem) : NULL
			);

			if(!error) return false;

			if(type == BLOCK_START) {
				/* Add the height of the element to our y position */
				subY += subHeight;
			} else {
				/* Add the width of the element, plus the argument spacing, to our x position */
				subX += subWidth + ARG_SPACING;
			}
		} else if(next) {
			/* We are not rendering stuff but we still want to find the next element */
			checkElem++;
		} else {
			return true;
		}
	}

	/* Point at the element after BLOCK_END */
	if(next) *next = checkElem + 1;

	return true;
}

bool drawElem(scriptElem_t *elem, int24_t x, int24_t y, blockColor_t parentColor, scriptElem_t **next, bool *csrOver, uint24_t *widthCache, uint24_t *heightCache) {
	/* Get the length of the element so we can allocate caches */
	size_t length = getLength(elem);
	bool freeWidth = false;
	bool freeHeight = false;
	uint24_t width, height;

	#ifdef DBG_DRAW
	dbg_sprintf(dbgout, "draw: ");
	printElemInfo(elem);
	dbg_sprintf(dbgout, " @ (%i,%i)\n", elem->type, elem, x, y);
	#endif

	/* If the element starts with its top-left corner off to the bottom right, return */
	/* Yeah, yeah, goto is bad. Whatever. */
	if(x > LCD_WIDTH || y > LCD_HEIGHT) goto setNext;

	/* Allocate caches if necessary */
	if(!widthCache) {
		widthCache = calloc(sizeof(widthCache[0]), length);
		if(!widthCache) return false;
		freeWidth = true;
	}
	if(!heightCache) {
		heightCache = calloc(sizeof(heightCache[0]), length);
		if(!heightCache) return false;
		freeHeight = true;
	}

	/* Reset the text scale */
	gfx_SetTextScale(1, 1);

	/* Get the width and height of the element */
	width = getWidth(elem, NULL, widthCache);
	height = getHeight(elem, NULL, heightCache);

	/* If the bottom right corner of the element is to the top left of the screen, return  */
	if(x + width < 0 || y + height < 0) goto setNext;

	switch(elem->type) {
		case ON_GREEN_FLAG: {
			blockColor_t col = CONTROL;
			uint24_t subX;
			int i;
			/* Set the graphx color */
			gfx_SetColor(getColor(col));

			/* Draw the hat */
			gfx_TransparentSprite(hat, x, y);
			/* Draw the box */
			gfx_FillRectangle(x, y + HAT_HEIGHT, width, height - HAT_HEIGHT - 1);
			/* Make the block look rounded */
			gfx_HorizLine(x + 1, y + height - 1, width - 2);

			/* Draw the notch */
			for(i = 0; i < NOTCH_DEPTH; i++) {
				gfx_HorizLine(x + NOTCH_OFFSET + i, y + height + i, NOTCH_SIZE - 2 * i);
			}

			/* Add a white-ish border to the non-hat top surface for a smoother transition */
			gfx_SetColor(0xCD);
			gfx_HorizLine(x + HAT_MIN_WIDTH - 2, y + HAT_HEIGHT - 1, width - HAT_MIN_WIDTH + 1);

			/* Draw text */
			gfx_SetTextFGColor(gfx_white);
			gfx_PrintStringXY("when ", x + LEFT_MARGIN, y + HAT_HEIGHT + (height - HAT_HEIGHT - TEXT_HEIGHT) / 2);

			subX = gfx_GetTextX();

			/* Add the flag */
			gfx_TransparentSprite(flag, subX, y + HAT_HEIGHT - 1);

			/* Move the text cursor to the right of the flag */
			subX += flag->width;

			gfx_PrintStringXY(" clicked", subX, gfx_GetTextY());

			break;
		}

		case STRING_LITERAL: {
			/* Draw the box border */
			gfx_SetColor(gfx_black);
			gfx_Rectangle(x, y - (TEXT_HEIGHT / 2) - 2, width, TEXT_HEIGHT + 4);

			/* Fill the inside of the box */
			gfx_SetColor(gfx_white);
			gfx_FillRectangle(x + 1, y - (TEXT_HEIGHT / 2) - 1, width - 2, TEXT_HEIGHT + 2);

			/* If the text will go offscreen, turn on text clipping */
			if(x < 0 || x + width > LCD_WIDTH) {
				gfx_SetTextConfig(gfx_text_clip);
			}

			/* Draw the actual text */
			gfx_SetTextFGColor(gfx_black);
			gfx_PrintStringXY(elem->data, x + 2, y - (TEXT_HEIGHT / 2));

			/* Reset the text config */
			gfx_SetTextConfig(gfx_text_noclip);

			break;
		}

		case BOOLEAN_LITERAL: {
			switch((uint24_t)elem->data) {
				case 0:
					gfx_SetColor(COLOR_FALSE);
					break;
				case 1:
					gfx_SetColor(COLOR_TRUE);
					break;
				case 2: /* This is an empty predicate "slot" */
					gfx_SetColor(getColor(parentColor | COLOR_DARK));
					break;
			}

			/* Draw the hexagon */
			drawPredicateBg(x, y, width, 8, PRED_CAP_WIDTH / 2);

			/* Draw the circle at the end of the selector */
			gfx_SetColor(COLOR_BOOL_SELECT);
			if((uint24_t)elem->data == 0) {
				gfx_FillCircle(x + 8 / 2, y, 8 / 2);
			} else if((uint24_t)elem->data == 1) {
				gfx_FillCircle(x + width - 8 / 2, y, 8 / 2);
			}

			break;
		}

		case TITLE_TEXT: {
			/* Use black text on lighter blocks only */
			if(parentColor & COLOR_ALT) {
				gfx_SetTextFGColor(gfx_black);
			} else {
				gfx_SetTextFGColor(gfx_white);
			}

			/* If the text will go offscreen, turn on text clipping */
			/* Seems somewhat broken, but only for one color? TODO: investigate */
			if(x < 0 || x + width > LCD_WIDTH) {
				gfx_SetTextConfig(gfx_text_clip);
			}

			/* Draw the actual text */
			gfx_PrintStringXY(elem->data, x, y - TEXT_HEIGHT / 2);

			/* Reset the text config */
			gfx_SetTextConfig(gfx_text_noclip);

			break;
		}

		case BLOCK_START: {
			/* Allocate a temporary sprite to store the inverted notch contents in */
			gfx_UninitedSprite(tmpSprite, NOTCH_SIZE, NOTCH_DEPTH);
			int i;

			/* Get the graphx color based on the appropriate category and parent color */
			blockColor_t col = getCategory(elem->data);
			if(col == parentColor) col |= COLOR_ALT;
			gfx_SetColor(getColor(col));

			/* Set the sprite size */
			tmpSprite->width  = NOTCH_SIZE;
			tmpSprite->height = NOTCH_DEPTH;

			/* Copy the background from where the inverted notch will be */
			gfx_GetSprite(tmpSprite, x + NOTCH_OFFSET, y);

			/* Draw a rounded rectangle */
			gfx_SetColor(getLightColor(col));
			gfx_HorizLine(x + 1, y, width - 2);
			gfx_VertLine(x, y + 1, height - 2);

			gfx_SetColor(getDarkColor(col));
			gfx_HorizLine(x + 1, y + height - 1, NOTCH_OFFSET - 1);
			gfx_HorizLine(x + NOTCH_OFFSET + NOTCH_SIZE, y + height - 1, width - 1 - (NOTCH_OFFSET + NOTCH_SIZE));
			gfx_VertLine(x + width - 1, y + 1, height - 2);

			gfx_SetColor(getColor(col));
			gfx_FillRectangle(x + 1, y + 1, width - 2, height - 2);
			gfx_HorizLine(x + NOTCH_OFFSET, y + height - 1, NOTCH_SIZE);

			/* Replace the background, leaving a rectangular notch */
			gfx_Sprite(tmpSprite, x + NOTCH_OFFSET, y);

			for(i = 0; i < NOTCH_DEPTH; i++) {
				gfx_SetColor(getColor(col));
				/* Draw the corners of the inverted notch */
				gfx_HorizLine(x + NOTCH_OFFSET + 1, y + i, i - 1);
				gfx_HorizLine(x + NOTCH_OFFSET + NOTCH_SIZE - i, y + i, i);

				/* Draw the regular notch */
				if(i == NOTCH_DEPTH - 1) gfx_SetColor(getDarkColor(col));
				gfx_HorizLine(x + NOTCH_OFFSET + i, y + height + i, NOTCH_SIZE - 2 * i - 1);
			
				gfx_SetColor(getLightColor(col));
				gfx_SetPixel(x + NOTCH_OFFSET + i - 1, y + i);

				gfx_SetColor(getDarkColor(col));
				gfx_SetPixel(x + NOTCH_OFFSET + NOTCH_SIZE - 1 - i, y + height + i);
			}

			/* Draw the block's subelements */
			drawRecursiveElem(elem, x + LEFT_MARGIN, y + height / 2, col, next, csrOver, widthCache, heightCache);
			
			goto success;
		}

		case PREDICATE_START: {
			/* Get the graphx color based on the appropriate category and parent color */
			blockColor_t col = getCategory(elem->data);
			if(col == parentColor) col |= COLOR_ALT;
			gfx_SetColor(getColor(col));

			/* Draw the hexagon */
			drawPredicateBg(x, y, width, height, PRED_CAP_WIDTH);

			/* Draw the predicate's subelements */
			drawRecursiveElem(elem, x + PRED_CAP_WIDTH + 1, y, col, next, csrOver, widthCache, heightCache);
			
			goto success;
		}

		case BLOCK_RING_START: {
			blockColor_t col = OTHER;
			if(col == parentColor) col |= COLOR_ALT;
			gfx_SetColor(getColor(col));

			drawReporterBg(x, y - height / 2, width, height);

			drawRecursiveElem(elem, x + 3, y - height / 2 + 3, col, next, csrOver, widthCache, heightCache);

			goto success;
		}
	}

	setNext:

	if(next) *next = getNext(elem);

	success:

	if(freeWidth) free(widthCache);
	if(freeHeight) free(heightCache);

	return true;
}

bool drawScript(scriptElem_t *elem, int24_t x, int24_t y, bool *csrOver, uint24_t *widthCache, uint24_t *heightCache) {
	uint24_t subY = y;
	scriptElem_t *checkElem = elem;
	size_t length = 0;
	bool freeWidth = false;
	bool freeHeight = false;

	/* If the element starts with its top-left corner off to the bottom right, return */
	if(x > LCD_WIDTH || y > LCD_HEIGHT) return true;

	/* Allocate caches if necessary */
	if(!widthCache) {
		if(!length) length = getScriptLength(elem);
		widthCache = calloc(sizeof(widthCache[0]), length);
		if(!widthCache) return false;
		freeWidth = true;
	}
	if(!heightCache) {
		if(!length) length = getScriptLength(elem);
		heightCache = calloc(sizeof(heightCache[0]), length);
		if(!heightCache) return false;
		freeHeight = true;
	}

	/* Iterate through all blocks in the script */
	/* Ensure that we don't exit the script */
	while(checkElem->type != END_SCRIPT) {
		/* Break if we are at the end of the block */
		if(subY < (int24_t)LCD_HEIGHT) {
			uint24_t subHeight;
			bool error;

			/* Get the height of the block */
			subHeight = getHeight(checkElem, NULL, heightCache + (checkElem - elem));
	
			/* Draw the block */
			error = drawElem(checkElem, x, subY, 0, &checkElem, NULL, widthCache + (checkElem - elem), heightCache + (checkElem - elem));
	
			if(!error) {
				if(freeWidth) free(widthCache);
				if(freeHeight) free(heightCache);
				return false;
			}

			/* Add the height to the y position */
			subY += subHeight;
		} else {
			/* We are not rendering stuff but we still want to find the next element */
			checkElem++;
		}
	}

	/* Let's not leak memory */
	if(freeWidth) free(widthCache);
	if(freeHeight) free(heightCache);

	return true;
}
