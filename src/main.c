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
	scriptElem_t elem[3 + 3 * layers + 15];
	size_t length;
	uint24_t width, height;
	uint24_t *widthCache, *heightCache;

	elem[0].type = ON_GREEN_FLAG;

	elem[1].type = BLOCK_START;
	elem[1].data = PRIM(SAY);

	elem[2].type = TITLE_TEXT;
	elem[2].data = "say";

	for(i = 0; i < layers; i++) {
		/* Add the predicate */
		elem[3 + 2 * i].type = PREDICATE_START;
		elem[3 + 2 * i].data = PRIM(NOT);
		/* Add text for the predicate */
		elem[3 + 2 * i + 1].type = TITLE_TEXT;
		elem[3 + 2 * i + 1].data = "not";
		/* Add the corresponding block end to close the predicate */
		elem[3 + 3 * layers - i].type = BLOCK_END;
		elem[3 + 3 * layers - i].data = (void*)&elem[3 + 2 * i];
	}
	/* Add assorted elems */
	elem[3 + 2 * layers].type = BOOLEAN_LITERAL;
	elem[3 + 2 * layers].data = (void*)0;

	elem[3 + 3 * layers + 1].type = BLOCK_END;
	elem[3 + 3 * layers + 1].data = (void*)&elem[1];

	elem[3 + 3 * layers + 2].type = BLOCK_START;
	elem[3 + 3 * layers + 2].data = PRIM(SAY);

	elem[3 + 3 * layers + 3].type = BLOCK_RING_START;

	elem[3 + 3 * layers + 4].type = BLOCK_START;
	elem[3 + 3 * layers + 4].data = PRIM(SAY);

	elem[3 + 3 * layers + 5].type = TITLE_TEXT;
	elem[3 + 3 * layers + 5].data = "say";

	elem[3 + 3 * layers + 6].type = STRING_LITERAL;
	elem[3 + 3 * layers + 6].data = "Hello";

	elem[3 + 3 * layers + 7].type = BLOCK_END;
	elem[3 + 3 * layers + 7].data = (void*)&elem[3 + 3 * layers + 4];

	elem[3 + 3 * layers + 8].type = BLOCK_START;
	elem[3 + 3 * layers + 8].data = PRIM(SAY);

	elem[3 + 3 * layers + 9].type = TITLE_TEXT;
	elem[3 + 3 * layers + 9].data = "say";

	elem[3 + 3 * layers + 10].type = STRING_LITERAL;
	elem[3 + 3 * layers + 10].data = "World!";

	elem[3 + 3 * layers + 11].type = BLOCK_END;
	elem[3 + 3 * layers + 11].data = (void*)&elem[3 + 3 * layers + 8];

	elem[3 + 3 * layers + 12].type = BLOCK_END;
	elem[3 + 3 * layers + 12].data = (void*)&elem[3 + 3 * layers + 3];

	elem[3 + 3 * layers + 13].type = BLOCK_END;
	elem[3 + 3 * layers + 13].data = (void*)&elem[3 + 3 * layers + 2];

	elem[3 + 3 * layers + 14].type = END_SCRIPT;

	/* Allocate memory to be used as a cache so we don't have to recalculate everything */
	length = getScriptLength(elem);
	widthCache  = calloc(length, sizeof(uint24_t));
	heightCache = calloc(length, sizeof(uint24_t));

	/* Draw everything */
	gfx_FillScreen(BG_COLOR);
	drawScript(elem, 20, 20, NULL, widthCache, heightCache);
	gfx_SwapDraw();

	free(widthCache);
	free(heightCache);
}

void main(void) {
	/* Initialize graphics */
	dbg_sprintf(dbgout, "Program Started\n");
	gfx_Begin();
	gfx_SetDrawBuffer();
	gfx_SetTextTransparentColor(1);
	gfx_SetTextBGColor(1);
	gfx_FillScreen(BG_COLOR);

	/* Reset the timer */
	timer_Control = TIMER1_DISABLE;
	timer_1_Counter = 0;
	timer_Control = TIMER1_ENABLE | TIMER1_32K | TIMER1_NOINT | TIMER1_UP;

	/* Test something or other */
	test();

	/* Print the timer to the console */
	timer_Control = TIMER1_DISABLE;
	dbg_sprintf(dbgout, "%u\n", timer_1_Counter);

	/* Wait for any key */
	while(!os_GetCSC());

	/* Cleanup */
	gfx_End();
}
