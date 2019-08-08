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

#define TEXT_HEIGHT 8
#define LEFT_MARGIN 8
#define ARG_SPACING 4
#define PRED_CAP_WIDTH 6

#define COLOR_TRUE 0x45
#define COLOR_FALSE 0xC9
#define COLOR_BOOL_SELECT 0xDE

uint8_t colors[] = {
	0, 0x5B, 0x9A, 0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0, 0, 0, 0, 0,		/* Regular colors */
	0, 0x9D, 0xBC, 0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0, 0, 0, 0, 0,		/* Alt color */
	0, 0x32, 0x53, 0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0, 0, 0, 0, 0,		/* Darker color */
	0, 0x74, 0x93, 0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0, 0, 0, 0, 0,		/* Darket alt color */
};

uint8_t getColor(blockColor_t col) {
	return colors[col];
}

uint24_t getHeight(scriptElem_t *elem, scriptElem_t **next, uint24_t *cache) {
	uint24_t height;
	scriptElem_t *tmp;
	if(!next) next = &tmp;
	*next = elem + 1;

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

		case PREDICATE_START: {
			height = 8;

			while((*next)->type != END_SCRIPT) {
				uint24_t newHeight;
				if((*next)->type == BLOCK_END && (*next)->data == (void*)elem) break;

				newHeight = getHeight(*next, next, cache + (*next - elem));
				if(newHeight > height) height = newHeight;
			}
			next++;

			height += 6;
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

	if(cache) *cache = height;
	return height;
}

uint24_t getWidth(scriptElem_t *elem, scriptElem_t **next, uint24_t *cache) {
	uint24_t width;
	scriptElem_t *tmp;
	if(!next) next = &tmp;
	*next = elem + 1;

	if(cache && *cache) {
		return *cache;
	}

	gfx_SetTextScale(1, 1);
	switch(elem->type) {
		case STRING_LITERAL:
			width = 4 + gfx_GetStringWidth(elem->data);
			break;

		case BOOLEAN_LITERAL:
			width = 3 * LEFT_MARGIN;
			break;

		case TITLE_TEXT:
			width = gfx_GetStringWidth(elem->data);
			break;

		case PREDICATE_START: {
			width = PRED_CAP_WIDTH * 2 - ARG_SPACING;
			while((*next)->type != END_SCRIPT) {
				if((*next)->type == BLOCK_END && (*next)->data == (void*)elem) break;

				width += getWidth(*next, next, cache + (*next - elem)) + ARG_SPACING;
			}
			*next++;
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

	if(cache) *cache = width;
	return width;
}

void drawPredicateBg(int24_t x, int24_t y, uint24_t width, uint24_t height, uint8_t capWidth) {
	uint8_t i;

	for(i = 1; i < capWidth; i++) {
		uint24_t h2 = height * i / capWidth;
		gfx_VertLine(x + i - 1, y - h2 / 2, h2);
		gfx_VertLine(x + width - i, y - h2 / 2, h2);
	}

	gfx_FillRectangle(x + capWidth - 1, y - height / 2, width - 2 * capWidth + 2, height);
}

bool drawElem(scriptElem_t *elem, int24_t x, int24_t y, blockColor_t parentColor, scriptElem_t **next, bool *csrOver, uint24_t *widthCache, uint24_t *heightCache) {
	size_t length = getLength(elem);
	bool freeWidth = false;
	bool freeHeight = false;
	uint24_t width, height;

	#ifdef DBG_DRAW
	dbg_sprintf(dbgout, "draw: ");
	printElemInfo(elem);
	dbg_sprintf(dbgout, " @ (%i,%i)\n", elem->type, elem, x, y);
	#endif

	if(x > LCD_WIDTH || y > LCD_HEIGHT) goto setNext;

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

	gfx_SetTextScale(1, 1);

	width = getWidth(elem, NULL, widthCache);
	height = getHeight(elem, NULL, heightCache);

	if(x + width < 0 || y + height < 0) goto setNext;

	switch(elem->type) {
		case STRING_LITERAL: {
			gfx_SetColor(gfx_black);
			gfx_Rectangle(x, y - (TEXT_HEIGHT / 2) - 2, width, TEXT_HEIGHT + 4);

			gfx_SetColor(gfx_white);
			gfx_FillRectangle(x + 1, y - (TEXT_HEIGHT / 2) - 1, width - 2, TEXT_HEIGHT + 2);

			if(x < 0 || x + width > LCD_WIDTH) {
				gfx_SetTextConfig(gfx_text_clip);
			}

			gfx_SetTextFGColor(gfx_black);
			gfx_PrintStringXY(elem->data, x + 2, y - (TEXT_HEIGHT / 2));

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
				case 2:
					gfx_SetColor(getColor(parentColor | COLOR_DARK));
					break;
			}

			drawPredicateBg(x, y, width, 8, PRED_CAP_WIDTH / 2);

			gfx_SetColor(COLOR_BOOL_SELECT);
			if((uint24_t)elem->data == 0) {
				gfx_FillCircle(x + 8 / 2, y, 8 / 2);
			} else if((uint24_t)elem->data == 1) {
				gfx_FillCircle(x + width - 8 / 2, y, 8 / 2);
			}

			break;
		}

		case TITLE_TEXT: {
			if(parentColor & COLOR_ALT) {
				gfx_SetTextFGColor(gfx_black);
			} else {
				gfx_SetTextFGColor(gfx_white);
			}

			if(x < 0 || x + width > LCD_WIDTH) {
				gfx_SetTextConfig(gfx_text_clip);
			}

			gfx_PrintStringXY(elem->data, x, y - TEXT_HEIGHT / 2);

			gfx_SetTextConfig(gfx_text_noclip);

			break;
		}

		case PREDICATE_START: {
			blockColor_t col;
			int24_t subX = x + PRED_CAP_WIDTH;
			scriptElem_t *checkElem = (scriptElem_t*)elem + 1;

			col = (blockColor_t)elem->data;
			if(col == parentColor) col |= COLOR_ALT;

			gfx_SetColor(getColor(col));

			drawPredicateBg(x, y, width, height, PRED_CAP_WIDTH);

			while(checkElem->type != END_SCRIPT) {
				if(checkElem->type == BLOCK_END && checkElem->data == (void*)elem) break;
				if(subX < (int24_t)LCD_WIDTH) {
					uint24_t subWidth;
					bool error;
					subWidth = getWidth(checkElem, NULL, widthCache + (checkElem - elem));
	
					error = drawElem(checkElem, subX, y, col, &checkElem, NULL, widthCache + (checkElem - elem), heightCache + (checkElem - elem));
	
					subX += subWidth + ARG_SPACING;
				} else {
					checkElem++;
				}
			}

			if(next) *next = checkElem + 1;
			goto success;
		}
	}

	setNext:

	if(next) *next = elem + 1;

	success:

	if(freeWidth) free(widthCache);
	if(freeHeight) free(heightCache);

	return true;
}
