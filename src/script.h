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
	END_SCRIPT,				/* No data */
	BLOCK_END,				/* Pointer to start of block */
	ON_GREEN_FLAG,			/* No data */
	ON_KEY,					/* Key type */
	ON_CLICK,				/* No data */
	ON_CONDITION_START,		/* No data - followed by a predicate and then BLOCK_END */
	ON_MESSAGE,				/* Pointer to message */
	ON_CLONE,				/* No data */
	CUSTOM_BLOCK_START,		/* Lower byte is a color, upper bytes are 0 for custom blocks, 1 for builtins */
	BLOCK_START,			/* Pointer to block definition, or 0x800000 + primitive block ID */
	REPORTER_START,			/* Reporter type / definition */
	PREDICATE_START,		/* Predicate type / definition */
	BLOCK_RING,				/* Pointer to script inside ring */
	C_BLOCK,				/* Pointer to script inside C block */
	REPORTER_RING,			/* Pointer to script inside ring */
	HIDDEN_REPORTER_RING,	/* Pointer to script inside ring */
	PREDICATE_RING,			/* Pointer to script inside ring */
	HIDDEN_PREDICATE_RING,	/* Pointer to script inside ring */
	ARGLIST_START,			/* No data */
	BOOLEAN_LITERAL,		/* 0 = false, 1 = true, 2 = "empty" false */
	STRING_LITERAL,			/* Pointer to string */
	FLOAT_LITERAL,			/* Pointer to float */
	VARIABLE,				/* Pointer to variable definition */
	UPVAR,					/* Pointer to variable definition */
	TITLE_TEXT,				/* String - ignored when evaluating */
};
typedef uint8_t elemType_t;

typedef struct ScriptElem {
	elemType_t type;
	char *data;
} scriptElem_t;

scriptElem_t *getNext(scriptElem_t *elem);

/* Returns the number of elems in this elem */
/* 1 for literals, 2 + sum(length of each sub-elem) */
size_t getLength(scriptElem_t *elem);

size_t getScriptLength(scriptElem_t *elem);

enum Categories {
	NO_CATEGORY,
	MOTION,
	LOOKS,
	SOUND,
	PEN,
	CONTROL,
	LISTS,
	SENSING,
	OPERATORS,
	VARIABLES,
	OTHER
};

/* Adding 0x800000 here gives an error 111 */
unsigned enum Primitives {
	PRIM_SAY,
	PRIM_NOT,
	NUM_PRIMATIVES
};

uint8_t getCategory(void *data);

extern scriptElem_t *primitiveBlocks[NUM_PRIMATIVES];

#ifndef NDEBUG
void printElemInfo(scriptElem_t *elem);
#else
#define printElemInfo(ignore) ((void*)0)
#endif

#define DBG_DRAW

#endif
