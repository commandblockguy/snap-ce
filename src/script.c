#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <debug.h>

#include "script.h"

size_t getLength(scriptElem_t *elem) {
	scriptElem_t *checkElem;

	switch(elem->type) {
		case ON_CONDITION_START:
		case CUSTOM_BLOCK_START:
		case BLOCK_START:
		case REPORTER_START:
		case PREDICATE_START:
		case BLOCK_RING_START:
		case REPORTER_RING_START:
		case PREDICATE_RING_START:
		case C_BLOCK_START:
		case ARGLIST_START:
			for(checkElem = elem;; checkElem++) {
				if(checkElem->type == END_SCRIPT) {
					return checkElem - elem;
				}
				if(checkElem->type == BLOCK_END && (scriptElem_t*)checkElem->data == elem) {
					return checkElem - elem;
				}
			}
		case END_SCRIPT:
		case BLOCK_END:
			return 0;
		default:
			return 1;
	}
}

#ifndef NDEBUG

char *elemNames[] = {
	"END_SCRIPT",				/* No data */
	"BLOCK_END",				/* Pointer to start of block */
	"ON_GREEN_FLAG",			/* No data */
	"ON_KEY",					/* Key type */
	"ON_CLICK",					/* No data */
	"ON_CONDITION_START",		/* No data - followed by a predicate and then BLOCK_END */
	"ON_MESSAGE",				/* Pointer to message */
	"ON_CLONE",					/* No data */
	"CUSTOM_BLOCK_START",		/* Lower byte is a color, upper bytes are 0 for custom blocks, 1 for builtins */
	"BLOCK_START",				/* Block type / definition */
	"REPORTER_START",			/* Reporter type / definition */
	"PREDICATE_START",			/* Predicate type / definition */
	"BLOCK_RING_START",			/* No data */
	"REPORTER_RING_START",		/* Bool: hidden (part of a unevaluated statement) */
	"PREDICATE_RING_START",		/* Bool: hidden (part of a unevaluated statement) */
	"C_BLOCK_START",			/* No data */
	"ARGLIST_START",			/* No data */
	"BOOLEAN_LITERAL",			/* 0 = false, 1 = true, 2 = "empty" false */
	"STRING_LITERAL",			/* Pointer to string */
	"FLOAT_LITERAL",			/* Pointer to float */
	"VARIABLE",					/* Pointer to variable definition */
	"UPVAR",					/* Pointer to variable definition */
	"TITLE_TEXT",				/* String - ignored when evaluating */
};

void printElemInfo(scriptElem_t *elem) {
	dbg_sprintf(dbgout, "[%u (%s) @ %p: 0x%p", elem->type, elemNames[elem->type], elem, elem->data);
	if(elem->type == TITLE_TEXT || elem->type == STRING_LITERAL)
		dbg_sprintf(dbgout, ": \"%s\"", elem->data);
	dbg_sprintf(dbgout, "]");
}

#endif
