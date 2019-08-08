/*
 *--------------------------------------
 * Program Name: Snap! CE
 * Author: commandblockguy
 * License:
 * Description:
 *--------------------------------------
*/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <graphx.h>

#include "script.h"
#include "blockrender.h"

#include <debug.h>

#define BG_COLOR 0x4A

void test() {
	int i;
	#define layers 5
	scriptElem_t elem[2 + 3 * layers + 3];
	uint24_t width;
	size_t length;
	uint24_t *widthCache, *heightCache;

	elem[0].type = BLOCK_START;
	elem[0].data = (void*)2;
	elem[1].type = TITLE_TEXT;
	elem[1].data = "say";

	for(i = 0; i < layers; i++) {
		elem[2 + 2 * i].type = PREDICATE_START;
		elem[2 + 2 * i].data = (void*)1;
		elem[2 + 2 * i + 1].type = TITLE_TEXT;
		elem[2 + 2 * i + 1].data = "not";
		elem[2 + 3 * layers - i].type = BLOCK_END;
		elem[2 + 3 * layers - i].data = (void*)&elem[2 + 2 * i];
	}
	elem[2 + 2 * layers].type = BOOLEAN_LITERAL;
	elem[2 + 2 * layers].data = (void*)0;
	elem[3 * layers + 3].type = BLOCK_END;
	elem[3 * layers + 3].data = (void*)elem;
	elem[3 + 3 * layers + 1].type = END_SCRIPT;

	length = getLength(elem);
	widthCache  = calloc(length, sizeof(uint24_t));
	heightCache = calloc(length, sizeof(uint24_t));

	width = getWidth(elem, NULL, widthCache);

	dbg_sprintf(dbgout, "%i\n", width);

	gfx_FillScreen(BG_COLOR);
	drawElem(elem, LCD_WIDTH / 2 - width / 2, LCD_HEIGHT / 2, 0, NULL, NULL, widthCache, heightCache);
	gfx_SwapDraw();
}

void main(void) {
	dbg_sprintf(dbgout, "Program Started\n");
	gfx_Begin();
	gfx_SetDrawBuffer();
	gfx_SetTextTransparentColor(1);
	gfx_SetTextBGColor(1);
	gfx_FillScreen(BG_COLOR);

	timer_Control = TIMER1_DISABLE;
	timer_1_Counter = 0;
	timer_Control = TIMER1_ENABLE | TIMER1_32K | TIMER1_NOINT | TIMER1_UP;

	test();

	timer_Control = TIMER1_DISABLE;
	dbg_sprintf(dbgout, "%u\n", timer_1_Counter);

	while(!os_GetCSC());

	gfx_End();
}
