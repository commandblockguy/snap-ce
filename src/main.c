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
	scriptElem_t elem[2 + 3 * layers + 4];
	size_t length;
	uint24_t *widthCache, *heightCache;

	elem[0].type = ON_GREEN_FLAG;
	elem[1].type = BLOCK_START;
	elem[1].data = (void*)(0x800000+PRIM_SAY);
	elem[2].type = TITLE_TEXT;
	elem[2].data = "say";

	for(i = 0; i < layers; i++) {
		elem[3 + 2 * i].type = PREDICATE_START;
		elem[3 + 2 * i].data = (void*)(0x800000+PRIM_NOT);
		elem[3 + 2 * i + 1].type = TITLE_TEXT;
		elem[3 + 2 * i + 1].data = "not";
		elem[3 + 3 * layers - i].type = BLOCK_END;
		elem[3 + 3 * layers - i].data = (void*)&elem[3 + 2 * i];
	}
	elem[3 + 2 * layers].type = BOOLEAN_LITERAL;
	elem[3 + 2 * layers].data = (void*)0;
	elem[3 + 3 * layers + 1].type = BLOCK_END;
	elem[3 + 3 * layers + 1].data = (void*)&elem[1];
	elem[3 + 3 * layers + 2].type = END_SCRIPT;

	length = getScriptLength(elem);
	widthCache  = calloc(length, sizeof(uint24_t));
	heightCache = calloc(length, sizeof(uint24_t));

	gfx_FillScreen(BG_COLOR);
	drawScript(elem, 20, 20, NULL, widthCache, heightCache);
	gfx_SwapDraw();

	free(widthCache);
	free(heightCache);
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
