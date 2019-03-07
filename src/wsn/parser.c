/*
 * Automatically generated from the files:
 *	src/wsn/parser.sid
 * and
 *	src/parser.act
 * by:
 *	sid
 */

/* BEGINNING OF HEADER */

#line 102 "src/parser.act"


	#include <assert.h>
	#include <limits.h>
	#include <string.h>
	#include <stdlib.h>
	#include <stdarg.h>
	#include <stdio.h>
	#include <errno.h>
	#include <ctype.h>

	#include "../txt.h"
	#include "../ast.h"
	#include "../xalloc.h"

	#ifndef FORM
	#error FORM required
	#endif

	#define PASTE(a, b) a ## b
	#define CAT(a, b)   PASTE(a, b)

	#define LX_PREFIX CAT(lx_, FORM)

	#define LX_TOKEN  CAT(LX_PREFIX, _token)
	#define LX_STATE  CAT(LX_PREFIX, _lx)
	#define LX_NEXT   CAT(LX_PREFIX, _next)
	#define LX_INIT   CAT(LX_PREFIX, _init)

	#define FORM_INPUT CAT(FORM, _input)

	/* XXX: get rid of this; use same %entry% for all grammars */
	#define FORM_ENTRY CAT(prod_, FORM)

	/* XXX: workarounds for SID's identifier escaping */
	#define prod_iso_Hebnf FORM_ENTRY
	#define TOK_CI__LITERAL TOK_CI_LITERAL
	#define TOK_CS__LITERAL TOK_CS_LITERAL

	#include "parser.h"
	#include "lexer.h"

	#include "io.h"

	typedef char         map_char;
	typedef const char * map_string;
	typedef struct txt   map_txt;
	typedef unsigned int map_count;

	typedef struct ast_term * map_term;
	typedef struct ast_alt * map_alt;

	struct act_state {
		enum LX_TOKEN lex_tok;
		enum LX_TOKEN lex_tok_save;
	};

	struct lex_state {
		struct LX_STATE lx;
		struct lx_dynbuf buf;

		int (*f)(void *opaque);
		void *opaque;

		/* TODO: use lx's generated conveniences for the pattern buffer */
		char a[512];
		char *p;
	};

	#define CURRENT_TERMINAL (act_state->lex_tok)
	#define ERROR_TERMINAL   (TOK_ERROR)
	#define ADVANCE_LEXER    do { act_state->lex_tok = LX_NEXT(&lex_state->lx); } while (0)
	#define SAVE_LEXER(tok)  do { act_state->lex_tok_save = act_state->lex_tok; \
	                              act_state->lex_tok = tok;                     } while (0)
	#define RESTORE_LEXER    do { act_state->lex_tok = act_state->lex_tok_save; } while (0)

	extern int allow_undefined;

	static const char *
	pattern_buffer(struct lex_state *lex_state)
	{
		const char *s;

		assert(lex_state != NULL);

		/* TODO */
		*lex_state->p++ = '\0';

		s = xstrdup(lex_state->a);
		if (s == NULL) {
			perror("xstrdup");
			exit(EXIT_FAILURE);
		}

		lex_state->p = lex_state->a;

		return s;
	}

	static const char *
	prefix(int base)
	{
		switch (base) {
		case 16: return "%x";
		case 10: return "%d";
		case  8: return "%o";
		case  2: return "%b";
		default: return "";
		}
	}

	static int
	string(const char *p, struct txt *t, int base)
	{
		char *q;

		assert(p != NULL);
		assert(t != NULL);
		assert(t->p != NULL);
		assert(base > 0);

		{
			const char *s;
			size_t z;

			s = prefix(base);
			z = strlen(s);

			assert(0 == strncmp(p, s, z));

			p += z;
		}

		q = t->p;

		for (;;) {
			unsigned long n;
			char *e;

			n = strtoul(p, &e, base);
			if (n == ULONG_MAX) {
				return -1;
			}

			if (n > UCHAR_MAX) {
				errno = ERANGE;
				return -1;
			}

			*q++ = (unsigned char) n;

			if (*e == '\0') {
				break;
			}

			assert(*e == '.');

			p = e + 1;
		}

		t->n = q - t->p;

		return 0;
	}

	static int
	range(const char *p,
		unsigned char *a, unsigned char *b,
		int base)
	{
		unsigned long m, n;
		char *e;

		assert(p != NULL);
		assert(a != NULL);
		assert(b != NULL);
		assert(base > 0);

		{
			const char *s;
			size_t z;

			s = prefix(base);
			z = strlen(s);

			assert(0 == strncmp(p, s, z));

			p += z;
		}

		m = strtoul(p, &e, base);
		if (m == ULONG_MAX) {
			return -1;
		}

		p = e;

		assert(*p == '-');
		p++;

		n = strtoul(p, &e, base);
		if (n == ULONG_MAX) {
			return -1;
		}

		assert(*e == '\0');

		if (m > UCHAR_MAX || n > UCHAR_MAX) {
			errno = ERANGE;
			return -1;
		}

		*a = m;
		*b = n;

		return 0;
	}

	static const char *
	ltrim(const char *s)
	{
		while (isspace((unsigned char) *s)) {
			s++;
		}

		return s;
	}

	static void
	rtrim(char *s)
	{
		char *p = s + strlen(s) - 1;

		assert(strlen(s) > 0);

		while (p >= s && isspace((unsigned char) *p)) {
			*p-- = '\0';
		}
	}

	static const char *
	trim(char *s)
	{
		rtrim(s);
		return ltrim(s);
	}

	static void
	err(const struct lex_state *lex_state, const char *fmt, ...)
	{
		va_list ap;

		assert(lex_state != NULL);

		va_start(ap, fmt);
		fprintf(stderr, "%u:%u: ",
			lex_state->lx.start.line, lex_state->lx.start.col);
		vfprintf(stderr, fmt, ap);
		fprintf(stderr, "\n");
		va_end(ap);
	}

	static void
	err_expected(const struct lex_state *lex_state, const char *token)
	{
		err(lex_state, "Syntax error: expected %s", token);
		exit(EXIT_FAILURE);
	}

	static void
	err_unimplemented(const struct lex_state *lex_state, const char *s)
	{
		err(lex_state, "Unimplemented: %s", s);
		exit(EXIT_FAILURE);
	}

#line 290 "src/wsn/parser.c"


#ifndef ERROR_TERMINAL
#error "-s no-numeric-terminals given and ERROR_TERMINAL is not defined"
#endif

/* BEGINNING OF FUNCTION DECLARATIONS */

static void prod_factor(lex_state, act_state, map_term *);
static void prod_list_Hof_Hterms(lex_state, act_state, map_term *);
static void prod_list_Hof_Hrules(lex_state, act_state, map_rule *);
static void prod_list_Hof_Halts(lex_state, act_state, map_alt *);
static void prod_body(lex_state, act_state);
static void prod_term(lex_state, act_state, map_term *);
static void prod_rule(lex_state, act_state, map_rule *);
extern void prod_wsn(lex_state, act_state, map_rule *);
static void prod_93(lex_state, act_state, map_rule *);
static void prod_94(lex_state, act_state, map_term *, map_alt *);
static void prod_95(lex_state, act_state, map_term *);

/* BEGINNING OF STATIC VARIABLES */


/* BEGINNING OF FUNCTION DEFINITIONS */

static void
prod_factor(lex_state lex_state, act_state act_state, map_term *ZOt)
{
	map_term ZIt;

	switch (CURRENT_TERMINAL) {
	case (TOK_STARTGROUP):
		{
			map_alt ZIa;

			ADVANCE_LEXER;
			prod_list_Hof_Halts (lex_state, act_state, &ZIa);
			switch (CURRENT_TERMINAL) {
			case (TOK_ENDGROUP):
				break;
			case (ERROR_TERMINAL):
				RESTORE_LEXER;
				goto ZL1;
			default:
				goto ZL1;
			}
			ADVANCE_LEXER;
			/* BEGINNING OF ACTION: make-group-term */
			{
#line 617 "src/parser.act"

		(ZIt) = ast_make_group_term((ZIa));
	
#line 344 "src/wsn/parser.c"
			}
			/* END OF ACTION: make-group-term */
		}
		break;
	case (TOK_STARTOPT):
		{
			map_alt ZIa;
			map_count ZImin;
			map_count ZImax;

			ADVANCE_LEXER;
			prod_list_Hof_Halts (lex_state, act_state, &ZIa);
			switch (CURRENT_TERMINAL) {
			case (TOK_ENDOPT):
				break;
			case (ERROR_TERMINAL):
				RESTORE_LEXER;
				goto ZL1;
			default:
				goto ZL1;
			}
			ADVANCE_LEXER;
			/* BEGINNING OF ACTION: make-group-term */
			{
#line 617 "src/parser.act"

		(ZIt) = ast_make_group_term((ZIa));
	
#line 373 "src/wsn/parser.c"
			}
			/* END OF ACTION: make-group-term */
			/* BEGINNING OF ACTION: rep-zero-or-one */
			{
#line 534 "src/parser.act"

		(ZImin) = 0;
		(ZImax) = 1;
	
#line 383 "src/wsn/parser.c"
			}
			/* END OF ACTION: rep-zero-or-one */
			/* BEGINNING OF ACTION: set-repeat */
			{
#line 539 "src/parser.act"

		assert((ZImax) >= (ZImin) || !(ZImax));

		(ZIt)->min = (ZImin);
		(ZIt)->max = (ZImax);
	
#line 395 "src/wsn/parser.c"
			}
			/* END OF ACTION: set-repeat */
		}
		break;
	case (TOK_STARTSTAR):
		{
			map_alt ZIa;
			map_count ZImin;
			map_count ZImax;

			ADVANCE_LEXER;
			prod_list_Hof_Halts (lex_state, act_state, &ZIa);
			switch (CURRENT_TERMINAL) {
			case (TOK_ENDSTAR):
				break;
			case (ERROR_TERMINAL):
				RESTORE_LEXER;
				goto ZL1;
			default:
				goto ZL1;
			}
			ADVANCE_LEXER;
			/* BEGINNING OF ACTION: make-group-term */
			{
#line 617 "src/parser.act"

		(ZIt) = ast_make_group_term((ZIa));
	
#line 424 "src/wsn/parser.c"
			}
			/* END OF ACTION: make-group-term */
			/* BEGINNING OF ACTION: rep-zero-or-more */
			{
#line 529 "src/parser.act"

		(ZImin) = 0;
		(ZImax) = 0;
	
#line 434 "src/wsn/parser.c"
			}
			/* END OF ACTION: rep-zero-or-more */
			/* BEGINNING OF ACTION: set-repeat */
			{
#line 539 "src/parser.act"

		assert((ZImax) >= (ZImin) || !(ZImax));

		(ZIt)->min = (ZImin);
		(ZIt)->max = (ZImax);
	
#line 446 "src/wsn/parser.c"
			}
			/* END OF ACTION: set-repeat */
		}
		break;
	case (TOK_ESC): case (TOK_CHAR): case (TOK_IDENT): case (TOK_EMPTY):
	case (TOK_CS__LITERAL):
		{
			prod_term (lex_state, act_state, &ZIt);
			if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
				RESTORE_LEXER;
				goto ZL1;
			}
		}
		break;
	case (ERROR_TERMINAL):
		return;
	default:
		goto ZL1;
	}
	goto ZL0;
ZL1:;
	SAVE_LEXER ((ERROR_TERMINAL));
	return;
ZL0:;
	*ZOt = ZIt;
}

static void
prod_list_Hof_Hterms(lex_state lex_state, act_state act_state, map_term *ZOl)
{
	map_term ZIl;

	if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
		return;
	}
	{
		prod_factor (lex_state, act_state, &ZIl);
		prod_95 (lex_state, act_state, &ZIl);
		if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
			RESTORE_LEXER;
			goto ZL1;
		}
	}
	goto ZL0;
ZL1:;
	SAVE_LEXER ((ERROR_TERMINAL));
	return;
ZL0:;
	*ZOl = ZIl;
}

static void
prod_list_Hof_Hrules(lex_state lex_state, act_state act_state, map_rule *ZOl)
{
	map_rule ZIl;

	if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
		return;
	}
	{
		prod_rule (lex_state, act_state, &ZIl);
		prod_93 (lex_state, act_state, &ZIl);
		if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
			RESTORE_LEXER;
			goto ZL1;
		}
	}
	goto ZL0;
ZL1:;
	SAVE_LEXER ((ERROR_TERMINAL));
	return;
ZL0:;
	*ZOl = ZIl;
}

static void
prod_list_Hof_Halts(lex_state lex_state, act_state act_state, map_alt *ZOl)
{
	map_alt ZIl;

	if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
		return;
	}
	{
		map_term ZIt;

		prod_list_Hof_Hterms (lex_state, act_state, &ZIt);
		prod_94 (lex_state, act_state, &ZIt, &ZIl);
		if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
			RESTORE_LEXER;
			goto ZL1;
		}
	}
	goto ZL0;
ZL1:;
	SAVE_LEXER ((ERROR_TERMINAL));
	return;
ZL0:;
	*ZOl = ZIl;
}

static void
prod_body(lex_state lex_state, act_state act_state)
{
ZL2_body:;
	switch (CURRENT_TERMINAL) {
	case (TOK_ESC): case (TOK_CHAR):
		{
			map_char ZIc;

			/* BEGINNING OF INLINE: 73 */
			{
				switch (CURRENT_TERMINAL) {
				case (TOK_CHAR):
					{
						/* BEGINNING OF EXTRACT: CHAR */
						{
#line 340 "src/parser.act"

		assert(strlen(lex_state->buf.a) == 1);

		ZIc = lex_state->buf.a[0];
	
#line 570 "src/wsn/parser.c"
						}
						/* END OF EXTRACT: CHAR */
						ADVANCE_LEXER;
					}
					break;
				case (TOK_ESC):
					{
						/* BEGINNING OF EXTRACT: ESC */
						{
#line 334 "src/parser.act"

		assert(strlen(lex_state->buf.a) == 2);

		ZIc = lex_state->buf.a[0];
	
#line 586 "src/wsn/parser.c"
						}
						/* END OF EXTRACT: ESC */
						ADVANCE_LEXER;
					}
					break;
				default:
					goto ZL1;
				}
			}
			/* END OF INLINE: 73 */
			/* BEGINNING OF ACTION: pattern-char */
			{
#line 519 "src/parser.act"

		/* TODO */
		*lex_state->p++ = (ZIc);
	
#line 604 "src/wsn/parser.c"
			}
			/* END OF ACTION: pattern-char */
			/* BEGINNING OF INLINE: body */
			goto ZL2_body;
			/* END OF INLINE: body */
		}
		/*UNREACHED*/
	case (ERROR_TERMINAL):
		return;
	default:
		break;
	}
	return;
ZL1:;
	SAVE_LEXER ((ERROR_TERMINAL));
	return;
}

static void
prod_term(lex_state lex_state, act_state act_state, map_term *ZOt)
{
	map_term ZIt;

	switch (CURRENT_TERMINAL) {
	case (TOK_EMPTY):
		{
			ADVANCE_LEXER;
			/* BEGINNING OF ACTION: make-empty-term */
			{
#line 566 "src/parser.act"

		(ZIt) = ast_make_empty_term();
	
#line 638 "src/wsn/parser.c"
			}
			/* END OF ACTION: make-empty-term */
		}
		break;
	case (TOK_IDENT):
		{
			map_string ZIs;

			/* BEGINNING OF EXTRACT: IDENT */
			{
#line 353 "src/parser.act"

		/*
		 * This rtrim() is for EBNF, which would require n-token lookahead
		 * in order to lex just an ident (as ident may contain whitespace).
		 *
		 * I'm trimming here (for all grammars) because it's simpler than
		 * doing this for just EBNF specifically, and harmless to others.
		 */
		rtrim(lex_state->buf.a);

		ZIs = xstrdup(lex_state->buf.a);
		if (ZIs == NULL) {
			perror("xstrdup");
			exit(EXIT_FAILURE);
		}
	
#line 666 "src/wsn/parser.c"
			}
			/* END OF EXTRACT: IDENT */
			ADVANCE_LEXER;
			/* BEGINNING OF ACTION: make-rule-term */
			{
#line 578 "src/parser.act"

		struct ast_rule *r;

		/*
		 * Regardless of whether a rule exists (yet) by this name, we make
		 * a placeholder rule just so that we have an ast_rule struct
		 * at which to point. This saves passing the grammar around, which
		 * keeps the rule-building productions simpler.
		 */
		r = ast_make_rule((ZIs), NULL);
		if (r == NULL) {
			perror("ast_make_rule");
			goto ZL1;
		}

		(ZIt) = ast_make_rule_term(r);
	
#line 690 "src/wsn/parser.c"
			}
			/* END OF ACTION: make-rule-term */
		}
		break;
	case (TOK_ESC): case (TOK_CHAR): case (TOK_CS__LITERAL):
		{
			map_txt ZIx;

			prod_body (lex_state, act_state);
			switch (CURRENT_TERMINAL) {
			case (TOK_CS__LITERAL):
				/* BEGINNING OF EXTRACT: CS_LITERAL */
				{
#line 376 "src/parser.act"

		ZIx.p = pattern_buffer(lex_state);
		ZIx.n = strlen(ZIx.p);
	
#line 709 "src/wsn/parser.c"
				}
				/* END OF EXTRACT: CS_LITERAL */
				break;
			case (ERROR_TERMINAL):
				RESTORE_LEXER;
				goto ZL1;
			default:
				goto ZL1;
			}
			ADVANCE_LEXER;
			/* BEGINNING OF ACTION: make-cs-literal-term */
			{
#line 599 "src/parser.act"

		(ZIt) = ast_make_literal_term(&(ZIx), 0);
	
#line 726 "src/wsn/parser.c"
			}
			/* END OF ACTION: make-cs-literal-term */
		}
		break;
	case (ERROR_TERMINAL):
		return;
	default:
		goto ZL1;
	}
	goto ZL0;
ZL1:;
	SAVE_LEXER ((ERROR_TERMINAL));
	return;
ZL0:;
	*ZOt = ZIt;
}

static void
prod_rule(lex_state lex_state, act_state act_state, map_rule *ZOr)
{
	map_rule ZIr;

	if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
		return;
	}
	{
		map_string ZIs;
		map_alt ZIa;

		switch (CURRENT_TERMINAL) {
		case (TOK_IDENT):
			/* BEGINNING OF EXTRACT: IDENT */
			{
#line 353 "src/parser.act"

		/*
		 * This rtrim() is for EBNF, which would require n-token lookahead
		 * in order to lex just an ident (as ident may contain whitespace).
		 *
		 * I'm trimming here (for all grammars) because it's simpler than
		 * doing this for just EBNF specifically, and harmless to others.
		 */
		rtrim(lex_state->buf.a);

		ZIs = xstrdup(lex_state->buf.a);
		if (ZIs == NULL) {
			perror("xstrdup");
			exit(EXIT_FAILURE);
		}
	
#line 777 "src/wsn/parser.c"
			}
			/* END OF EXTRACT: IDENT */
			break;
		default:
			goto ZL1;
		}
		ADVANCE_LEXER;
		/* BEGINNING OF INLINE: 87 */
		{
			{
				switch (CURRENT_TERMINAL) {
				case (TOK_EQUALS):
					break;
				default:
					goto ZL3;
				}
				ADVANCE_LEXER;
			}
			goto ZL2;
		ZL3:;
			{
				/* BEGINNING OF ACTION: err-expected-equals */
				{
#line 709 "src/parser.act"

		err_expected(lex_state, "production rule assignment");
	
#line 805 "src/wsn/parser.c"
				}
				/* END OF ACTION: err-expected-equals */
			}
		ZL2:;
		}
		/* END OF INLINE: 87 */
		prod_list_Hof_Halts (lex_state, act_state, &ZIa);
		if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
			RESTORE_LEXER;
			goto ZL1;
		}
		/* BEGINNING OF ACTION: make-rule */
		{
#line 646 "src/parser.act"

		(ZIr) = ast_make_rule((ZIs), (ZIa));
	
#line 823 "src/wsn/parser.c"
		}
		/* END OF ACTION: make-rule */
		/* BEGINNING OF INLINE: 88 */
		{
			{
				switch (CURRENT_TERMINAL) {
				case (TOK_SEP):
					break;
				default:
					goto ZL5;
				}
				ADVANCE_LEXER;
			}
			goto ZL4;
		ZL5:;
			{
				/* BEGINNING OF ACTION: err-expected-sep */
				{
#line 705 "src/parser.act"

		err_expected(lex_state, "production rule separator");
	
#line 846 "src/wsn/parser.c"
				}
				/* END OF ACTION: err-expected-sep */
			}
		ZL4:;
		}
		/* END OF INLINE: 88 */
	}
	goto ZL0;
ZL1:;
	SAVE_LEXER ((ERROR_TERMINAL));
	return;
ZL0:;
	*ZOr = ZIr;
}

void
prod_wsn(lex_state lex_state, act_state act_state, map_rule *ZOl)
{
	map_rule ZIl;

	if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
		return;
	}
	{
		prod_list_Hof_Hrules (lex_state, act_state, &ZIl);
		switch (CURRENT_TERMINAL) {
		case (TOK_EOF):
			break;
		case (ERROR_TERMINAL):
			RESTORE_LEXER;
			goto ZL1;
		default:
			goto ZL1;
		}
		ADVANCE_LEXER;
	}
	goto ZL0;
ZL1:;
	{
		/* BEGINNING OF ACTION: make-empty-rule */
		{
#line 650 "src/parser.act"

		(ZIl) = NULL;
	
#line 892 "src/wsn/parser.c"
		}
		/* END OF ACTION: make-empty-rule */
		/* BEGINNING OF ACTION: err-syntax */
		{
#line 697 "src/parser.act"

		err(lex_state, "Syntax error");
		exit(EXIT_FAILURE);
	
#line 902 "src/wsn/parser.c"
		}
		/* END OF ACTION: err-syntax */
	}
ZL0:;
	*ZOl = ZIl;
}

static void
prod_93(lex_state lex_state, act_state act_state, map_rule *ZIl)
{
	switch (CURRENT_TERMINAL) {
	case (TOK_IDENT):
		{
			map_rule ZIr;

			prod_list_Hof_Hrules (lex_state, act_state, &ZIr);
			if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
				RESTORE_LEXER;
				goto ZL1;
			}
			/* BEGINNING OF ACTION: add-rule-to-list */
			{
#line 665 "src/parser.act"

		if (ast_find_rule((ZIr), (*ZIl)->name)) {
			fprintf(stderr, "production rule <%s> already exists\n", (*ZIl)->name);
			/* TODO: print location of this and previous definition */
			/* TODO: handle as warning; add rule anyway, and bail out at end */
			exit(EXIT_FAILURE);
		}

		assert((*ZIl)->next == NULL);
		(*ZIl)->next = (ZIr);
	
#line 937 "src/wsn/parser.c"
			}
			/* END OF ACTION: add-rule-to-list */
		}
		break;
	case (ERROR_TERMINAL):
		return;
	default:
		break;
	}
	return;
ZL1:;
	SAVE_LEXER ((ERROR_TERMINAL));
	return;
}

static void
prod_94(lex_state lex_state, act_state act_state, map_term *ZIt, map_alt *ZOl)
{
	map_alt ZIl;

	switch (CURRENT_TERMINAL) {
	case (TOK_ALT):
		{
			map_alt ZIa;

			/* BEGINNING OF INLINE: 84 */
			{
				{
					switch (CURRENT_TERMINAL) {
					case (TOK_ALT):
						break;
					default:
						goto ZL3;
					}
					ADVANCE_LEXER;
				}
				goto ZL2;
			ZL3:;
				{
					/* BEGINNING OF ACTION: err-expected-alt */
					{
#line 701 "src/parser.act"

		err_expected(lex_state, "alternative separator");
	
#line 983 "src/wsn/parser.c"
					}
					/* END OF ACTION: err-expected-alt */
				}
			ZL2:;
			}
			/* END OF INLINE: 84 */
			prod_list_Hof_Halts (lex_state, act_state, &ZIa);
			if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
				RESTORE_LEXER;
				goto ZL1;
			}
			/* BEGINNING OF ACTION: make-alt */
			{
#line 642 "src/parser.act"

		(ZIl) = ast_make_alt((*ZIt));
	
#line 1001 "src/wsn/parser.c"
			}
			/* END OF ACTION: make-alt */
			/* BEGINNING OF ACTION: add-alt-to-list */
			{
#line 660 "src/parser.act"

		assert((ZIl)->next == NULL);
		(ZIl)->next = (ZIa);
	
#line 1011 "src/wsn/parser.c"
			}
			/* END OF ACTION: add-alt-to-list */
		}
		break;
	default:
		{
			/* BEGINNING OF ACTION: make-alt */
			{
#line 642 "src/parser.act"

		(ZIl) = ast_make_alt((*ZIt));
	
#line 1024 "src/wsn/parser.c"
			}
			/* END OF ACTION: make-alt */
		}
		break;
	case (ERROR_TERMINAL):
		return;
	}
	goto ZL0;
ZL1:;
	SAVE_LEXER ((ERROR_TERMINAL));
	return;
ZL0:;
	*ZOl = ZIl;
}

static void
prod_95(lex_state lex_state, act_state act_state, map_term *ZIl)
{
	switch (CURRENT_TERMINAL) {
	case (TOK_STARTGROUP): case (TOK_STARTOPT): case (TOK_STARTSTAR): case (TOK_ESC):
	case (TOK_CHAR): case (TOK_IDENT): case (TOK_EMPTY): case (TOK_CS__LITERAL):
		{
			map_term ZIt;

			prod_list_Hof_Hterms (lex_state, act_state, &ZIt);
			if ((CURRENT_TERMINAL) == (ERROR_TERMINAL)) {
				RESTORE_LEXER;
				goto ZL1;
			}
			/* BEGINNING OF ACTION: add-term-to-list */
			{
#line 655 "src/parser.act"

		assert((*ZIl)->next == NULL);
		(*ZIl)->next = (ZIt);
	
#line 1061 "src/wsn/parser.c"
			}
			/* END OF ACTION: add-term-to-list */
		}
		break;
	case (ERROR_TERMINAL):
		return;
	default:
		break;
	}
	return;
ZL1:;
	SAVE_LEXER ((ERROR_TERMINAL));
	return;
}

/* BEGINNING OF TRAILER */

#line 843 "src/parser.act"


	static int
	lgetc(struct LX_STATE *lx)
	{
		const struct lex_state *lex_state;

		assert(lx != NULL);
		assert(lx->getc_opaque != NULL);

		lex_state = lx->getc_opaque;

		assert(lex_state->f != NULL);

		return lex_state->f(lex_state->opaque);
	}

	struct ast_rule *
	FORM_INPUT(int (*f)(void *opaque), void *opaque)
	{
		struct act_state  act_state_s;
		struct act_state *act_state;
		struct lex_state  lex_state_s;
		struct lex_state *lex_state;

		struct LX_STATE *lx;
		struct ast_rule *g;

		/* for dialects which don't use these */
		(void) string;
		(void) range;
		(void) ltrim;
		(void) rtrim;
		(void) trim;

		assert(f != NULL);

		g = NULL;

		lex_state    = &lex_state_s;
		lex_state->p = lex_state->a;

		lx = &lex_state->lx;

		LX_INIT(lx);

		lx->lgetc       = lgetc;
		lx->getc_opaque = lex_state;

		lex_state->f       = f;
		lex_state->opaque  = opaque;

		lex_state->buf.a   = NULL;
		lex_state->buf.len = 0;

		/* XXX: unneccessary since we're lexing from a string */
		lx->buf_opaque = &lex_state->buf;
		lx->push       = CAT(LX_PREFIX, _dynpush);
		lx->clear      = CAT(LX_PREFIX, _dynclear);
		lx->free       = CAT(LX_PREFIX, _dynfree);

	/* XXX */
	lx->free = NULL;

		/* This is a workaround for ADVANCE_LEXER assuming a pointer */
		act_state = &act_state_s;

		ADVANCE_LEXER;
		FORM_ENTRY(lex_state, act_state, &g);

		/* TODO: handle error */

		/* substitute placeholder rules for the real thing */
		{
			const struct ast_rule *p;
			const struct ast_alt *q;
			struct ast_term *t;
			struct ast_rule *r;
			int ok;

			ok = 1;

			for (p = g; p != NULL; p = p->next) {
				for (q = p->alts; q != NULL; q = q->next) {
					for (t = q->terms; t != NULL; t = t->next) {
						if (t->type != TYPE_RULE) {
							continue;
						}

						r = ast_find_rule(g, t->u.rule->name);
						if (r != NULL) {
							free((char *) t->u.rule->name);
							ast_free_rule((void *) t->u.rule);
							t->u.rule = r;
							continue;
						}

						if (!allow_undefined) {
							fprintf(stderr, "production rule <%s> not defined\n", t->u.rule->name);
							/* TODO: print location of this and previous definition */
							/* XXX: would leak the ast_rule here */

							ok = 0;
							continue;
						}

						{
							const char *token;

							token = t->u.rule->name;

							ast_free_rule(t->u.rule);

							t->type    = TYPE_TOKEN;
							t->u.token = token;
						}
					}
				}
			}

			if (!ok) {
				exit(EXIT_FAILURE);
			}
		}

		return g;
	}

#line 1208 "src/wsn/parser.c"

/* END OF FILE */
