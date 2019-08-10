#ifndef H_SCRIPT
#define H_SCRIPT

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <tice.h>

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum ElemTypes {
	END_SCRIPT,					/* No data */
	BLOCK_END,					/* Pointer to start of block */
	ON_GREEN_FLAG,				/* No data */
	ON_KEY,						/* Key type */
	ON_CLICK,					/* No data */
	ON_CONDITION_START,			/* No data - followed by a predicate and then BLOCK_END */
	ON_MESSAGE,					/* Pointer to message */
	ON_CLONE,					/* No data */
	CUSTOM_BLOCK_START,			/* Lower byte is a color, upper bytes are 0 for custom blocks, 1 for builtins */
	BLOCK_START,				/* Pointer to block definition, or 0x800000 + primitive block ID */
	REPORTER_START,				/* Reporter type / definition */
	PREDICATE_START,			/* Predicate type / definition */
	BLOCK_RING_START,			/* No data */
	C_BLOCK_START,				/* No data */
	REPORTER_RING_START,		/* No data */
	HIDDEN_REPORTER_RING_START,	/* No data */
	PREDICATE_RING_START,		/* No data */
	HIDDEN_PREDICATE_RING_START,/* No data */
	ARGLIST_START,				/* No data */
	BOOLEAN_LITERAL,			/* 0 = false, 1 = true, 2 = "empty" false */
	STRING_LITERAL,				/* Pointer to string */
	FLOAT_LITERAL,				/* Pointer to float */
	VARIABLE,					/* Pointer to variable definition */
	UPVAR,						/* Pointer to variable definition */
	TITLE_TEXT,					/* String - ignored when evaluating */
	NUM_ELEMENTS,				/* Not an acutal element */
};
typedef uint8_t elemType_t;

typedef struct ScriptElem {
	elemType_t type;
	char *data;
} scriptElem_t;

/* Returns the next element after the end of elem */
scriptElem_t *getNext(scriptElem_t *elem);

/* Returns the total number of elems in this elem/script */
/* i.e. 1 for literals, 2 + sum(length of each sub-elem) for blocks */
size_t getLength(scriptElem_t *elem);
size_t getScriptLength(scriptElem_t *elem);

enum Categories {
	NO_CATEGORY,
	MOTION,
	LOOKS,
	SOUND,
	PEN,
	LISTS,
	CONTROL,
	SENSING,
	OPERATORS,
	VARIABLES,
	OTHER
};

uint8_t getCategory(void *data);

/* IDs for primative functions defined in C */
unsigned enum Primitives {
	SAY,
	NOT,
	NUM_PRIMATIVES
};
#define PRIM(p) (void*)(0x800000 + p)

/* Pointers to definitions of primative blocks */
extern scriptElem_t *primitiveBlocks[NUM_PRIMATIVES];

/* Prints information about an element in an easy-to-read format */
#ifndef NDEBUG
void printElemInfo(scriptElem_t *elem);
#else
#define printElemInfo(ignore) ((void*)0)
#endif

/* Comment this out to disable debugging output while drawing */
#define DBG_DRAW

#endif
