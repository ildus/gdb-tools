/* A Bison parser, made by GNU Bison 3.0.4.  */

/* Bison implementation for Yacc-like parsers in C

   Copyright (C) 1984, 1989-1990, 2000-2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */

/* As a special exception, you may create a larger work that contains
   part or all of the Bison parser skeleton and distribute that work
   under terms of your choice, so long as that work isn't itself a
   parser generator using the skeleton or a modified version thereof
   as a parser skeleton.  Alternatively, if you modify or redistribute
   the parser skeleton itself, you may (at your option) remove this
   special exception, which will cause the skeleton and the resulting
   Bison output files to be licensed under the GNU General Public
   License without this special exception.

   This special exception was added by the Free Software Foundation in
   version 2.2 of Bison.  */

/* C LALR(1) parser skeleton written by Richard Stallman, by
   simplifying the original so-called "semantic" parser.  */

/* All symbols defined below should begin with yy or YY, to avoid
   infringing on user name space.  This should be done even for local
   variables, as they might otherwise be expanded by user macros.
   There are some unavoidable exceptions within include files to
   define necessary library symbols; they are noted "INFRINGES ON
   USER NAME SPACE" below.  */

/* Identify Bison output.  */
#define YYBISON 1

/* Bison version.  */
#define YYBISON_VERSION "3.0.4"

/* Skeleton name.  */
#define YYSKELETON_NAME "yacc.c"

/* Pure parsers.  */
#define YYPURE 0

/* Push parsers.  */
#define YYPUSH 0

/* Pull parsers.  */
#define YYPULL 1




/* Copy the first part of user declarations.  */
#line 1 "parse.y" /* yacc.c:339  */

/*   DUEL - A Very High Level Debugging Langauge.  */
/*   Public domain code			           */
/*   Written by Michael Golan mg@cs.princeton.edu  */
/*$Header: /tmp_mnt/n/fs/grad2/mg/duel/RCS/parse.y,v 1.14 93/03/17 11:04:12 mg Exp $*/

/* this module contains the duel parser, in yacc, plus a simple lexer.
 * the lexer is slow, but duel expressions are tiny.
 * the parsing generate an AST with essentially no type checking.
 * names are only looked up when the refer explicitly to types. This forces
 * the use of "T" before user types. You can't parse (x)(y) correctly, if
 * you want the node to contain "cast" or "func", without knowing the x is
 * not a type. (It is interesting to note that (x *)(y) is clearly a cast,
 * but it can not be parsed without a context sensitive grammer!).
 *
 * Version 1.1 now accept (x) as a type cast, so (print)("hi") fails, but
 * (uint)z is ok. Also accepted is (uint*)z. T is still required in sizeof
 * and in variable declarations. A side effect was making "sizeof x" illegal
 * (since then sizeof(x)-1 was parsed sizeof((x)-1) with (x) a cast), so
 * now sizeof(x) must be used. Note that in C, sizoef(x)++ is acceptable,
 * and the '++' operate on x (which is optimized out!) This can be confusing.
 *
 * yacc is also not smart enough to recognize e.g. "if(e) e ; else e" as
 * a special case (redundent ';'). I hacked this in the lexer. It should
 * reduce the trouble with C->duel coding. (It can also be done for {e1}e2,
 * in some speical cases, e.g. if e2 is a keyword, or a name or a unary op,
 * but this can confuse some people, e.g. in {i}[5], so I left it alone.)
 * Finally, the %/ operator is accepted as "#/" and "%%" as "#", to those
 * who wish to keep gdb with # comments.
 * memory: nodes are alloc'ed dynamically. a parsing error loose so-far
 * allocated nodes, which is normally acceptable (yyerror can probably hack
 * into the yacc stack to release them.)
 */

/*
 * $Log:	parse.y,v $
 * Revision 1.14  93/03/17  11:04:12  mg
 * fixed (t*)x bug, was parsed as (t**)x
 *
 * Revision 1.13  93/03/12  06:15:09  mg
 * modified unary's a bit - cosmetics
 * support (x)y as type cast
 * support (x*)y as type cast
 * replace sizeof exp  with sizeof(exp) to prevent clash with above
 * more cosmetics, including yyerror abort, tuint for uint.
 * takes anything after |> to be comment (pipe command really)
 *
 *
 * Revision 1.12  93/02/27  06:06:09  mg
 * added signed char parsing.
 *
 * Revision 1.11  93/02/23  19:15:38  mg
 * improved escaped char support
 *
 * Revision 1.10  93/02/03  21:49:34  mg
 * bug fix - yyerror calls now abort parsing (eg called from lex)
 *
 * Revision 1.9  93/01/12  21:53:07  mg
 * cleanup and set for release
 *
 * Revision 1.8  93/01/07  00:14:33  mg
 * add &&/ ||/
 * fixed parsing of trailing ';' was a mess.
 * ignore ';' before 'else' and '}' w/warning.
 *
 * Revision 1.7  93/01/03  07:31:01  mg
 * error reporting
 *
 * Revision 1.6  92/12/24  23:35:50  mg
 * began src pos support
 *
 * Revision 1.5  92/10/19  15:08:02  mg
 * frames() added; bug fixed
 *
 * Revision 1.4  92/10/14  02:06:32  mg
 * misc/change casting parsing/variable def.
 *
 * Revision 1.3  92/09/16  11:09:39  mg
 * add typedef/struct support, const strings
 * cleanup s/r conflict by setting ELSE to a token. explained some stuff in
 * comments.
 *
 * Revision 1.2  92/09/15  06:10:46  mg
 * cosmetics and new ops: x@y, for() while() ..x and x..
 * generic '.' and '_'  support. x@y. '..x' and 'x..'.  while(), for(), ?:
 *
 */

#include "duel.h"

static char *inputstr ;		/* pointer to string being parsed */
static char *lexptr ;           /* current lexer pointer into input str */
static tnode *root ;		/* result of parsing stored here */

/* pick unique names for globals of yacc. gdb has other parsers! */
#define	yyparse	duel_yyparse
#define	yylex	duel_yylex
#define	yyerror	duel_yyerror
#define	yylval	duel_yylval
#define	yychar	duel_yychar
#define	yydebug	duel_yydebug
#define	yypact	duel_yypact
#define	yyr1	duel_yyr1
#define	yyr2	duel_yyr2
#define	yydef	duel_yydef
#define	yychk	duel_yychk
#define	yypgo	duel_yypgo
#define	yyact	duel_yyact
#define	yyexca	duel_yyexca
#define yyerrflag duel_yyerrflag
#define yynerrs	duel_yynerrs
#define	yyps	duel_yyps
#define	yypv	duel_yypv
#define	yys	duel_yys
#define	yystate	duel_yystate
#define	yytmp	duel_yytmp
#define	yyv	duel_yyv
#define	yyval	duel_yyval
#define	yylloc	duel_yylloc

typedef struct {                /* token info for operators */
        int src_pos ;            /* source position */
        topcode opcode ;        /* opcode          */
       } topinfo ;

typedef struct {                /* token info for symbols */
        int src_pos ;            /* source position */
        char *name ;             /* symbol          */
       } tnameinfo ;

/* these are used as operators to mknode_... when source location is unknown*/
static topinfo seq_op  = { -1,';' } ; /* sequencing operator, src pos unkown */
static topinfo decl_op = { -1,OP_DECL } ; /* declare var op, src pos unkown */

/* local prototypes. */
LPROC  yyerror(char *msg);
LFUNC  int yylex (void);

LPROC push_type(char desc) ;
LPROC push_type_int(char desc,tnode *n)  ;
LFUNC bool pop_type(char *desc,int *size);

LFUNC tnode* mknode_op(top_kind,topinfo opinfo,tnode*,tnode*,tnode*,tnode*);
LFUNC tnode* mknode_const(int src_pos,tctype *ctype);
LFUNC tnode* mknode_ctype(tctype *ctype);
LFUNC tnode* mknode_name(tnameinfo nameinfo);
LFUNC tnode* mknode_modified_ctype(tctype *base);

#define mknode_post_unary(op,n) (mknode_op(OPK_POST_UNARY,op,n, 0, 0,0))
#define mknode_unary(op,n)      (mknode_op(OPK_UNARY,     op,n, 0, 0,0))
#define mknode_sunary(op,n)     (mknode_op(OPK_SUNARY,    op,n, 0, 0,0))
#define mknode_bin(op,n1,n2)    (mknode_op(OPK_BIN,       op,n1,n2,0,0))
#define mknode_sbin(op,n1,n2)   (mknode_op(OPK_SBIN,      op,n1,n2,0,0))
#define mknode_tri(op,n1,n2,n3) (mknode_op(OPK_TRI,       op,n1,n2,n3,0))

static tctype *decl_tbase ; /* used for variables decl */

/* #define	YYDEBUG	1 */


#line 227 "y.tab.c" /* yacc.c:339  */

# ifndef YY_NULLPTR
#  if defined __cplusplus && 201103L <= __cplusplus
#   define YY_NULLPTR nullptr
#  else
#   define YY_NULLPTR 0
#  endif
# endif

/* Enabling verbose error messages.  */
#ifdef YYERROR_VERBOSE
# undef YYERROR_VERBOSE
# define YYERROR_VERBOSE 1
#else
# define YYERROR_VERBOSE 0
#endif


/* Debug traces.  */
#ifndef YYDEBUG
# define YYDEBUG 0
#endif
#if YYDEBUG
extern int yydebug;
#endif

/* Token type.  */
#ifndef YYTOKENTYPE
# define YYTOKENTYPE
  enum yytokentype
  {
    T_CONST = 258,
    T_SYM = 259,
    T_ASSIGN = 260,
    T_DEFVAR = 261,
    T_CHAR = 262,
    T_INT = 263,
    T_SHORT = 264,
    T_LONG = 265,
    T_UNSIGNED = 266,
    T_FLOAT = 267,
    T_DOUBLE = 268,
    T_VOID = 269,
    T_STRUCT = 270,
    T_UNION = 271,
    T_ENUM = 272,
    T_SIZEOF = 273,
    T_TYPEDEF_INDICATOR = 274,
    T_SIGNED = 275,
    T_IF = 276,
    T_ELSE = 277,
    T_FOR = 278,
    T_WHILE = 279,
    T_OR = 280,
    T_AND = 281,
    T_RSH = 282,
    T_LSH = 283,
    T_INC = 284,
    T_DEC = 285,
    T_COUNT = 286,
    T_FRAME = 287,
    T_TO = 288,
    T_DFS = 289,
    T_BFS = 290,
    T_ARROW = 291,
    T_OSEL = 292,
    T_CSEL = 293,
    T_IMP = 294,
    T_ANDL = 295,
    T_ORL = 296,
    T_EQ = 297,
    T_NE = 298,
    T_EQQ = 299,
    T_NEQ = 300,
    T_LE = 301,
    T_GE = 302,
    T_LSQ = 303,
    T_GTQ = 304,
    T_LEQ = 305,
    T_GEQ = 306,
    STMT = 307,
    UNARY = 308,
    T_POS = 309
  };
#endif
/* Tokens.  */
#define T_CONST 258
#define T_SYM 259
#define T_ASSIGN 260
#define T_DEFVAR 261
#define T_CHAR 262
#define T_INT 263
#define T_SHORT 264
#define T_LONG 265
#define T_UNSIGNED 266
#define T_FLOAT 267
#define T_DOUBLE 268
#define T_VOID 269
#define T_STRUCT 270
#define T_UNION 271
#define T_ENUM 272
#define T_SIZEOF 273
#define T_TYPEDEF_INDICATOR 274
#define T_SIGNED 275
#define T_IF 276
#define T_ELSE 277
#define T_FOR 278
#define T_WHILE 279
#define T_OR 280
#define T_AND 281
#define T_RSH 282
#define T_LSH 283
#define T_INC 284
#define T_DEC 285
#define T_COUNT 286
#define T_FRAME 287
#define T_TO 288
#define T_DFS 289
#define T_BFS 290
#define T_ARROW 291
#define T_OSEL 292
#define T_CSEL 293
#define T_IMP 294
#define T_ANDL 295
#define T_ORL 296
#define T_EQ 297
#define T_NE 298
#define T_EQQ 299
#define T_NEQ 300
#define T_LE 301
#define T_GE 302
#define T_LSQ 303
#define T_GTQ 304
#define T_LEQ 305
#define T_GEQ 306
#define STMT 307
#define UNARY 308
#define T_POS 309

/* Value type.  */
#if ! defined YYSTYPE && ! defined YYSTYPE_IS_DECLARED

union YYSTYPE
{
#line 163 "parse.y" /* yacc.c:355  */

    tnode   *node ;                 /* node pointer for constructed exp tree */
    tctype  *ctype;                 /* type for type nodes                   */
    tnameinfo nameinfo ;            /* a name/symbol + src position */
    topinfo opinfo;                 /* keyword/operator + source position    */
  

#line 380 "y.tab.c" /* yacc.c:355  */
};

typedef union YYSTYPE YYSTYPE;
# define YYSTYPE_IS_TRIVIAL 1
# define YYSTYPE_IS_DECLARED 1
#endif


extern YYSTYPE yylval;

int yyparse (void);



/* Copy the second part of user declarations.  */

#line 397 "y.tab.c" /* yacc.c:358  */

#ifdef short
# undef short
#endif

#ifdef YYTYPE_UINT8
typedef YYTYPE_UINT8 yytype_uint8;
#else
typedef unsigned char yytype_uint8;
#endif

#ifdef YYTYPE_INT8
typedef YYTYPE_INT8 yytype_int8;
#else
typedef signed char yytype_int8;
#endif

#ifdef YYTYPE_UINT16
typedef YYTYPE_UINT16 yytype_uint16;
#else
typedef unsigned short int yytype_uint16;
#endif

#ifdef YYTYPE_INT16
typedef YYTYPE_INT16 yytype_int16;
#else
typedef short int yytype_int16;
#endif

#ifndef YYSIZE_T
# ifdef __SIZE_TYPE__
#  define YYSIZE_T __SIZE_TYPE__
# elif defined size_t
#  define YYSIZE_T size_t
# elif ! defined YYSIZE_T
#  include <stddef.h> /* INFRINGES ON USER NAME SPACE */
#  define YYSIZE_T size_t
# else
#  define YYSIZE_T unsigned int
# endif
#endif

#define YYSIZE_MAXIMUM ((YYSIZE_T) -1)

#ifndef YY_
# if defined YYENABLE_NLS && YYENABLE_NLS
#  if ENABLE_NLS
#   include <libintl.h> /* INFRINGES ON USER NAME SPACE */
#   define YY_(Msgid) dgettext ("bison-runtime", Msgid)
#  endif
# endif
# ifndef YY_
#  define YY_(Msgid) Msgid
# endif
#endif

#ifndef YY_ATTRIBUTE
# if (defined __GNUC__                                               \
      && (2 < __GNUC__ || (__GNUC__ == 2 && 96 <= __GNUC_MINOR__)))  \
     || defined __SUNPRO_C && 0x5110 <= __SUNPRO_C
#  define YY_ATTRIBUTE(Spec) __attribute__(Spec)
# else
#  define YY_ATTRIBUTE(Spec) /* empty */
# endif
#endif

#ifndef YY_ATTRIBUTE_PURE
# define YY_ATTRIBUTE_PURE   YY_ATTRIBUTE ((__pure__))
#endif

#ifndef YY_ATTRIBUTE_UNUSED
# define YY_ATTRIBUTE_UNUSED YY_ATTRIBUTE ((__unused__))
#endif

#if !defined _Noreturn \
     && (!defined __STDC_VERSION__ || __STDC_VERSION__ < 201112)
# if defined _MSC_VER && 1200 <= _MSC_VER
#  define _Noreturn __declspec (noreturn)
# else
#  define _Noreturn YY_ATTRIBUTE ((__noreturn__))
# endif
#endif

/* Suppress unused-variable warnings by "using" E.  */
#if ! defined lint || defined __GNUC__
# define YYUSE(E) ((void) (E))
#else
# define YYUSE(E) /* empty */
#endif

#if defined __GNUC__ && 407 <= __GNUC__ * 100 + __GNUC_MINOR__
/* Suppress an incorrect diagnostic about yylval being uninitialized.  */
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN \
    _Pragma ("GCC diagnostic push") \
    _Pragma ("GCC diagnostic ignored \"-Wuninitialized\"")\
    _Pragma ("GCC diagnostic ignored \"-Wmaybe-uninitialized\"")
# define YY_IGNORE_MAYBE_UNINITIALIZED_END \
    _Pragma ("GCC diagnostic pop")
#else
# define YY_INITIAL_VALUE(Value) Value
#endif
#ifndef YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
# define YY_IGNORE_MAYBE_UNINITIALIZED_END
#endif
#ifndef YY_INITIAL_VALUE
# define YY_INITIAL_VALUE(Value) /* Nothing. */
#endif


#if ! defined yyoverflow || YYERROR_VERBOSE

/* The parser invokes alloca or malloc; define the necessary symbols.  */

# ifdef YYSTACK_USE_ALLOCA
#  if YYSTACK_USE_ALLOCA
#   ifdef __GNUC__
#    define YYSTACK_ALLOC __builtin_alloca
#   elif defined __BUILTIN_VA_ARG_INCR
#    include <alloca.h> /* INFRINGES ON USER NAME SPACE */
#   elif defined _AIX
#    define YYSTACK_ALLOC __alloca
#   elif defined _MSC_VER
#    include <malloc.h> /* INFRINGES ON USER NAME SPACE */
#    define alloca _alloca
#   else
#    define YYSTACK_ALLOC alloca
#    if ! defined _ALLOCA_H && ! defined EXIT_SUCCESS
#     include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
      /* Use EXIT_SUCCESS as a witness for stdlib.h.  */
#     ifndef EXIT_SUCCESS
#      define EXIT_SUCCESS 0
#     endif
#    endif
#   endif
#  endif
# endif

# ifdef YYSTACK_ALLOC
   /* Pacify GCC's 'empty if-body' warning.  */
#  define YYSTACK_FREE(Ptr) do { /* empty */; } while (0)
#  ifndef YYSTACK_ALLOC_MAXIMUM
    /* The OS might guarantee only one guard page at the bottom of the stack,
       and a page size can be as small as 4096 bytes.  So we cannot safely
       invoke alloca (N) if N exceeds 4096.  Use a slightly smaller number
       to allow for a few compiler-allocated temporary stack slots.  */
#   define YYSTACK_ALLOC_MAXIMUM 4032 /* reasonable circa 2006 */
#  endif
# else
#  define YYSTACK_ALLOC YYMALLOC
#  define YYSTACK_FREE YYFREE
#  ifndef YYSTACK_ALLOC_MAXIMUM
#   define YYSTACK_ALLOC_MAXIMUM YYSIZE_MAXIMUM
#  endif
#  if (defined __cplusplus && ! defined EXIT_SUCCESS \
       && ! ((defined YYMALLOC || defined malloc) \
             && (defined YYFREE || defined free)))
#   include <stdlib.h> /* INFRINGES ON USER NAME SPACE */
#   ifndef EXIT_SUCCESS
#    define EXIT_SUCCESS 0
#   endif
#  endif
#  ifndef YYMALLOC
#   define YYMALLOC malloc
#   if ! defined malloc && ! defined EXIT_SUCCESS
void *malloc (YYSIZE_T); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
#  ifndef YYFREE
#   define YYFREE free
#   if ! defined free && ! defined EXIT_SUCCESS
void free (void *); /* INFRINGES ON USER NAME SPACE */
#   endif
#  endif
# endif
#endif /* ! defined yyoverflow || YYERROR_VERBOSE */


#if (! defined yyoverflow \
     && (! defined __cplusplus \
         || (defined YYSTYPE_IS_TRIVIAL && YYSTYPE_IS_TRIVIAL)))

/* A type that is properly aligned for any stack member.  */
union yyalloc
{
  yytype_int16 yyss_alloc;
  YYSTYPE yyvs_alloc;
};

/* The size of the maximum gap between one aligned stack and the next.  */
# define YYSTACK_GAP_MAXIMUM (sizeof (union yyalloc) - 1)

/* The size of an array large to enough to hold all stacks, each with
   N elements.  */
# define YYSTACK_BYTES(N) \
     ((N) * (sizeof (yytype_int16) + sizeof (YYSTYPE)) \
      + YYSTACK_GAP_MAXIMUM)

# define YYCOPY_NEEDED 1

/* Relocate STACK from its old location to the new one.  The
   local variables YYSIZE and YYSTACKSIZE give the old and new number of
   elements in the stack, and YYPTR gives the new location of the
   stack.  Advance YYPTR to a properly aligned location for the next
   stack.  */
# define YYSTACK_RELOCATE(Stack_alloc, Stack)                           \
    do                                                                  \
      {                                                                 \
        YYSIZE_T yynewbytes;                                            \
        YYCOPY (&yyptr->Stack_alloc, Stack, yysize);                    \
        Stack = &yyptr->Stack_alloc;                                    \
        yynewbytes = yystacksize * sizeof (*Stack) + YYSTACK_GAP_MAXIMUM; \
        yyptr += yynewbytes / sizeof (*yyptr);                          \
      }                                                                 \
    while (0)

#endif

#if defined YYCOPY_NEEDED && YYCOPY_NEEDED
/* Copy COUNT objects from SRC to DST.  The source and destination do
   not overlap.  */
# ifndef YYCOPY
#  if defined __GNUC__ && 1 < __GNUC__
#   define YYCOPY(Dst, Src, Count) \
      __builtin_memcpy (Dst, Src, (Count) * sizeof (*(Src)))
#  else
#   define YYCOPY(Dst, Src, Count)              \
      do                                        \
        {                                       \
          YYSIZE_T yyi;                         \
          for (yyi = 0; yyi < (Count); yyi++)   \
            (Dst)[yyi] = (Src)[yyi];            \
        }                                       \
      while (0)
#  endif
# endif
#endif /* !YYCOPY_NEEDED */

/* YYFINAL -- State number of the termination state.  */
#define YYFINAL  77
/* YYLAST -- Last index in YYTABLE.  */
#define YYLAST   1420

/* YYNTOKENS -- Number of terminals.  */
#define YYNTOKENS  81
/* YYNNTS -- Number of nonterminals.  */
#define YYNNTS  18
/* YYNRULES -- Number of rules.  */
#define YYNRULES  129
/* YYNSTATES -- Number of states.  */
#define YYNSTATES  238

/* YYTRANSLATE[YYX] -- Symbol number corresponding to YYX as returned
   by yylex, with out-of-bounds checking.  */
#define YYUNDEFTOK  2
#define YYMAXUTOK   309

#define YYTRANSLATE(YYX)                                                \
  ((unsigned int) (YYX) <= YYMAXUTOK ? yytranslate[YYX] : YYUNDEFTOK)

/* YYTRANSLATE[TOKEN-NUM] -- Symbol number corresponding to TOKEN-NUM
   as returned by yylex, without out-of-bounds checking.  */
static const yytype_uint8 yytranslate[] =
{
       0,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    48,     2,    46,     2,    38,    31,     2,
      42,    43,    36,    34,    26,    35,    39,    37,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,    80,    25,
      32,    27,    33,    28,    47,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,    40,     2,    41,    30,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,    44,    29,    45,    49,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     2,     2,     2,     2,     2,     1,     2,     3,     4,
       5,     6,     7,     8,     9,    10,    11,    12,    13,    14,
      15,    16,    17,    18,    19,    20,    21,    22,    23,    24,
      50,    51,    52,    53,    54,    55,    56,    57,    58,    59,
      60,    61,    62,    63,    64,    65,    66,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    77,    78,    79
};

#if YYDEBUG
  /* YYRLINE[YYN] -- Source line where rule number YYN was defined.  */
static const yytype_uint16 yyrline[] =
{
       0,   211,   211,   214,   215,   216,   217,   219,   220,   222,
     223,   226,   226,   228,   229,   232,   236,   237,   238,   239,
     240,   251,   252,   254,   256,   259,   261,   263,   265,   271,
     272,   273,   274,   275,   276,   277,   278,   279,   280,   281,
     282,   283,   284,   285,   288,   289,   291,   292,   293,   294,
     295,   296,   297,   298,   299,   302,   313,   317,   326,   327,
     328,   329,   330,   331,   332,   333,   334,   335,   336,   337,
     338,   339,   340,   341,   342,   343,   344,   345,   346,   347,
     348,   349,   352,   356,   357,   358,   363,   364,   365,   366,
     367,   370,   371,   374,   375,   378,   379,   385,   387,   392,
     393,   394,   395,   396,   413,   423,   424,   425,   426,   427,
     428,   429,   430,   431,   432,   433,   434,   435,   436,   437,
     438,   439,   440,   441,   442,   443,   444,   447,   450,   455
};
#endif

#if YYDEBUG || YYERROR_VERBOSE || 0
/* YYTNAME[SYMBOL-NUM] -- String name of the symbol SYMBOL-NUM.
   First, the terminals, then, starting at YYNTOKENS, nonterminals.  */
static const char *const yytname[] =
{
  "$end", "error", "$undefined", "T_CONST", "T_SYM", "T_ASSIGN",
  "T_DEFVAR", "T_CHAR", "T_INT", "T_SHORT", "T_LONG", "T_UNSIGNED",
  "T_FLOAT", "T_DOUBLE", "T_VOID", "T_STRUCT", "T_UNION", "T_ENUM",
  "T_SIZEOF", "T_TYPEDEF_INDICATOR", "T_SIGNED", "T_IF", "T_ELSE", "T_FOR",
  "T_WHILE", "';'", "','", "'='", "'?'", "'|'", "'^'", "'&'", "'<'", "'>'",
  "'+'", "'-'", "'*'", "'/'", "'%'", "'.'", "'['", "']'", "'('", "')'",
  "'{'", "'}'", "'#'", "'@'", "'!'", "'~'", "T_OR", "T_AND", "T_RSH",
  "T_LSH", "T_INC", "T_DEC", "T_COUNT", "T_FRAME", "T_TO", "T_DFS",
  "T_BFS", "T_ARROW", "T_OSEL", "T_CSEL", "T_IMP", "T_ANDL", "T_ORL",
  "T_EQ", "T_NE", "T_EQQ", "T_NEQ", "T_LE", "T_GE", "T_LSQ", "T_GTQ",
  "T_LEQ", "T_GEQ", "STMT", "UNARY", "T_POS", "':'", "$accept", "start",
  "duel_inp", "duel_exp", "all_decls", "vars_decl", "$@1", "var_decl",
  "name_decl1", "name_decl", "exp", "sm_exp", "oexp", "nameexp", "type",
  "type_mod", "typebase", "name", YY_NULLPTR
};
#endif

# ifdef YYPRINT
/* YYTOKNUM[NUM] -- (External) token number corresponding to the
   (internal) symbol number NUM (which must be that of a token).  */
static const yytype_uint16 yytoknum[] =
{
       0,   256,   257,   258,   259,   260,   261,   262,   263,   264,
     265,   266,   267,   268,   269,   270,   271,   272,   273,   274,
     275,   276,   277,   278,   279,    59,    44,    61,    63,   124,
      94,    38,    60,    62,    43,    45,    42,    47,    37,    46,
      91,    93,    40,    41,   123,   125,    35,    64,    33,   126,
     280,   281,   282,   283,   284,   285,   286,   287,   288,   289,
     290,   291,   292,   293,   294,   295,   296,   297,   298,   299,
     300,   301,   302,   303,   304,   305,   306,   307,   308,   309,
      58
};
# endif

#define YYPACT_NINF -105

#define yypact_value_is_default(Yystate) \
  (!!((Yystate) == (-105)))

#define YYTABLE_NINF -1

#define yytable_value_is_error(Yytable_value) \
  (!!((Yytable_value) == (-1)))

  /* YYPACT[STATE-NUM] -- Index in YYTABLE of the portion describing
     STATE-NUM.  */
static const yytype_int16 yypact[] =
{
     843,  -105,  -105,  -105,  -105,    15,    46,    23,  -105,  -105,
    -105,    39,    39,    39,   -15,    39,    44,    16,    34,    64,
     892,   892,   892,   843,   892,   892,   892,   892,   892,   892,
      67,   892,   892,   892,   110,  -105,  -105,    86,  -105,   667,
      87,   108,  -105,  -105,  -105,  -105,   109,  -105,  -105,   111,
      47,  -105,  -105,  -105,   843,  -105,  -105,   892,   892,   892,
     148,   148,   148,     1,    73,   -17,   -14,   -10,   148,   148,
     148,   148,   148,   892,  1302,  1076,  1028,  -105,   843,   892,
     892,   892,   892,   892,   892,   892,   892,   892,   892,   892,
     892,   892,   892,   892,   892,   892,    39,   892,   892,   892,
     892,   892,  -105,  -105,   941,   892,   892,   892,   892,   892,
     892,   892,   892,   892,   892,   892,   892,   892,   892,   892,
     892,   892,     0,  -105,  -105,  -105,   113,   249,    75,   300,
     667,    98,   351,   892,  -105,   892,   -17,   -17,    84,   -17,
     892,  -105,   402,  -105,  -105,   769,   718,   769,   191,  1123,
    1169,  1214,  1331,  1331,  1358,  1358,   148,   148,   148,  -105,
     453,    88,  -105,  -105,  1028,  1076,   932,   932,  1302,  -105,
    -105,  -105,   504,   667,  1259,  1259,  1259,  1259,  1331,  1331,
    1331,  1331,  1331,  1331,   667,   769,     0,     0,   106,  -105,
      90,  -105,  -105,  -105,  -105,   104,   892,   892,   148,    84,
     -19,   130,     7,   148,  -105,   892,  -105,  -105,  -105,    90,
       9,     0,   133,   892,   564,   616,   667,    95,    97,   892,
     980,    99,  -105,   101,   667,   892,   892,   100,  -105,   148,
     102,  -105,   667,   107,  -105,  -105,   892,   667
};

  /* YYDEFACT[STATE-NUM] -- Default reduction number in state STATE-NUM.
     Performed when YYTABLE does not specify something else to do.  Zero
     means the default is an error.  */
static const yytype_uint8 yydefact[] =
{
       0,    95,   129,   105,   108,   119,   111,   109,   123,   124,
     125,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     2,     6,     3,     9,    92,
       7,    96,    11,    97,   120,   112,   115,   107,   110,   121,
     113,   126,   127,   128,     0,   104,   106,     0,    94,     0,
      30,    31,    29,     0,     0,   103,    97,     0,    32,    33,
      37,    38,    34,     0,    87,    35,    36,     1,     4,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,    94,     0,     0,     0,     0,
       0,     0,    39,    40,    88,     0,     0,     0,     0,     0,
       0,     0,     0,     0,     0,     0,     0,     0,     0,     0,
       8,     0,     0,   116,   122,   114,   117,     0,     0,     0,
      93,     0,     0,     0,    53,     0,   103,   103,    98,   103,
       0,    54,     0,     5,    10,    84,    89,    83,     0,    78,
      79,    77,    73,    74,    61,    62,    58,    59,    60,    49,
       0,     0,    46,    47,    81,    80,    64,    63,    86,    44,
      45,    48,     0,    90,    65,    66,    67,    68,    69,    70,
      75,    76,    71,    72,    91,    85,     0,     0,    12,    13,
      15,    20,   118,    42,    41,     0,     0,    28,    55,   101,
       0,     0,     0,    56,    43,     0,    50,    52,    51,    18,
       0,     0,     0,     0,    21,     0,    27,    99,     0,     0,
      82,    16,    14,     0,    23,    22,    94,     0,   102,    57,
       0,    19,    24,     0,   100,    17,    26,    25
};

  /* YYPGOTO[NTERM-NUM].  */
static const yytype_int8 yypgoto[] =
{
    -105,  -105,  -105,    66,  -105,    69,  -105,  -105,   -62,  -104,
     -20,    79,   -92,   -82,   103,   -91,   -13,     5
};

  /* YYDEFGOTO[NTERM-NUM].  */
static const yytype_int16 yydefgoto[] =
{
      -1,    34,    35,    36,    37,    38,   122,   188,   189,   190,
      39,    40,   131,    41,    64,   138,    42,    43
};

  /* YYTABLE[YYPACT[STATE-NUM]] -- What to do in state STATE-NUM.  If
     positive, shift that token.  If negative, reduce the rule whose
     number is the opposite.  If YYTABLE_NINF, syntax error.  */
static const yytype_int16 yytable[] =
{
      60,    61,    62,   161,     2,    68,    69,    70,    71,    72,
      65,    74,    75,    76,   162,   133,    51,    52,    53,   136,
      55,   201,   139,    44,   217,   137,   133,    54,    66,   140,
      47,    48,    49,    50,   127,   141,   186,   129,   130,   132,
     191,    65,   187,     2,   134,   199,   200,   201,   202,   212,
     219,    56,   221,   142,    45,   125,    46,   126,    57,   145,
     146,   147,   148,   149,   150,   151,   152,   153,   154,   155,
     156,   157,   158,   159,   160,   130,    58,   163,   164,   165,
     166,   167,   209,   210,   168,   169,   170,   171,   172,   173,
     174,   175,   176,   177,   178,   179,   180,   181,   182,   183,
     184,   185,    63,    67,   191,   191,    59,     1,     2,    73,
      77,    78,   120,   184,   121,   198,   135,   123,   194,   124,
     203,   192,    14,   196,   201,    17,   213,    18,    19,   191,
     212,   207,   211,   218,   233,    20,   223,   227,   228,    21,
      22,   230,   231,   234,   143,   235,    23,   144,    24,   222,
     236,     0,    25,    26,     0,     0,     0,   128,    27,    28,
      29,    30,    31,     0,     0,     0,     0,     0,     0,    32,
      33,     0,     0,     0,     0,   214,   215,   216,     0,     0,
       0,     0,     0,     0,     0,   220,     0,    93,    94,     0,
      95,     0,     0,   224,    96,    97,    79,     0,     0,   229,
       0,     0,   102,   103,     0,   232,   130,   105,   106,   107,
     108,     0,     0,     0,     0,     0,   237,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,     0,    95,     0,     0,     0,    96,    97,     0,
       0,    98,    99,   100,   101,   102,   103,     0,     0,   104,
     105,   106,   107,   108,    79,   109,     0,     0,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,     0,     0,
       0,   205,     0,     0,     0,    80,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
       0,    95,   193,     0,     0,    96,    97,     0,     0,    98,
      99,   100,   101,   102,   103,    79,     0,   104,   105,   106,
     107,   108,     0,   109,     0,     0,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,    80,    81,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,     0,    95,   195,     0,     0,    96,    97,     0,     0,
      98,    99,   100,   101,   102,   103,    79,     0,   104,   105,
     106,   107,   108,     0,   109,     0,     0,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,    80,    81,    82,
      83,    84,    85,    86,    87,    88,    89,    90,    91,    92,
      93,    94,     0,    95,   197,     0,     0,    96,    97,     0,
       0,    98,    99,   100,   101,   102,   103,    79,     0,   104,
     105,   106,   107,   108,     0,   109,     0,     0,   110,   111,
     112,   113,   114,   115,   116,   117,   118,   119,    80,    81,
      82,    83,    84,    85,    86,    87,    88,    89,    90,    91,
      92,    93,    94,     0,    95,   204,     0,     0,    96,    97,
       0,     0,    98,    99,   100,   101,   102,   103,    79,     0,
     104,   105,   106,   107,   108,     0,   109,     0,     0,   110,
     111,   112,   113,   114,   115,   116,   117,   118,   119,    80,
      81,    82,    83,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,   206,    95,     0,     0,     0,    96,
      97,     0,     0,    98,    99,   100,   101,   102,   103,    79,
       0,   104,   105,   106,   107,   108,     0,   109,     0,     0,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,     0,    95,     0,     0,     0,
      96,    97,     0,     0,    98,    99,   100,   101,   102,   103,
       0,     0,   104,   105,   106,   107,   108,   208,   109,    79,
       0,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,     0,     0,     0,     0,     0,   225,     0,     0,     0,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,     0,    95,     0,     0,     0,
      96,    97,     0,     0,    98,    99,   100,   101,   102,   103,
       0,    79,   104,   105,   106,   107,   108,     0,   109,     0,
       0,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,   226,    80,    81,    82,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,     0,    95,     0,
       0,     0,    96,    97,     0,     0,    98,    99,   100,   101,
     102,   103,    79,     0,   104,   105,   106,   107,   108,     0,
     109,     0,     0,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,    80,    81,    82,    83,    84,    85,    86,
      87,    88,    89,    90,    91,    92,    93,    94,     0,    95,
       0,     0,     0,    96,    97,     0,     0,    98,    99,   100,
     101,   102,   103,    79,     0,   104,   105,   106,   107,   108,
       0,   109,     0,     0,   110,   111,   112,   113,   114,   115,
     116,   117,   118,   119,    80,    81,    82,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,     0,
      95,     0,     0,     0,    96,    97,     0,     0,    98,    99,
     100,   101,   102,   103,    79,     0,   104,   105,   106,   107,
     108,     0,     0,     0,     0,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,     0,    81,    82,    83,    84,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
       0,    95,     0,     0,     0,    96,    97,     0,     0,    98,
      99,   100,   101,   102,   103,     0,     0,   104,   105,   106,
     107,   108,     0,     0,     0,     0,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,     1,     2,     0,     0,
       3,     4,     5,     6,     7,     8,     9,    10,    11,    12,
      13,    14,    15,    16,    17,     0,    18,    19,     0,     0,
       0,     0,     0,     0,    20,     0,     0,     0,    21,    22,
       0,     0,     0,     0,     0,    23,     0,    24,     0,     0,
       0,    25,    26,     0,     0,     1,     2,    27,    28,    29,
      30,    31,     0,     0,     0,     0,     0,     0,    32,    33,
      14,     0,     0,    17,     0,    18,    19,     0,     0,     0,
       0,     0,     0,    20,     0,     0,     0,    21,    22,     0,
       0,     0,     0,     0,    23,     0,    24,     0,     0,     0,
      25,    26,     0,     0,     1,     2,    27,    28,    29,    30,
      31,     0,     0,     0,     0,     0,     0,    32,    33,    14,
       0,     0,    17,     0,    18,    19,    88,    89,    90,    91,
      92,    93,    94,     0,    95,     0,    21,    22,    96,    97,
       0,     0,     0,    23,     0,    24,   102,   103,     0,    25,
      26,   105,   106,   107,   108,    27,    28,    29,    30,    -1,
       0,     0,     0,     0,     0,     0,    32,    33,    82,    83,
      84,    85,    86,    87,    88,    89,    90,    91,    92,    93,
      94,     0,    95,     0,     0,     0,    96,    97,     0,     0,
      98,    99,   100,   101,   102,   103,     0,     0,   104,   105,
     106,   107,   108,     0,     0,     0,     0,   110,   111,   112,
     113,   114,   115,   116,   117,   118,   119,    83,    84,    85,
      86,    87,    88,    89,    90,    91,    92,    93,    94,     0,
      95,     0,     0,     0,    96,    97,     0,     0,     0,    99,
     100,   101,   102,   103,     0,     0,   104,   105,   106,   107,
     108,     0,     0,     0,     0,   110,   111,   112,   113,   114,
     115,   116,   117,   118,   119,    83,    84,    85,    86,    87,
      88,    89,    90,    91,    92,    93,    94,     0,    95,     0,
       0,     0,    96,    97,     0,     0,     0,     0,   100,   101,
     102,   103,     0,     0,   104,   105,   106,   107,   108,     0,
       0,     0,     0,   110,   111,   112,   113,   114,   115,   116,
     117,   118,   119,    84,    85,    86,    87,    88,    89,    90,
      91,    92,    93,    94,     0,    95,     0,     0,     0,    96,
      97,     0,     0,     0,     0,   100,   101,   102,   103,     0,
       0,   104,   105,   106,   107,   108,     0,     0,     0,     0,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
      85,    86,    87,    88,    89,    90,    91,    92,    93,    94,
       0,    95,     0,     0,     0,    96,    97,     0,     0,     0,
       0,   100,   101,   102,   103,     0,     0,   104,   105,   106,
     107,   108,     0,     0,     0,     0,   110,   111,   112,   113,
     114,   115,   116,   117,   118,   119,    86,    87,    88,    89,
      90,    91,    92,    93,    94,     0,    95,     0,     0,     0,
      96,    97,     0,     0,     0,     0,   100,   101,   102,   103,
       0,     0,   104,   105,   106,   107,   108,     0,     0,     0,
       0,   110,   111,   112,   113,   114,   115,   116,   117,   118,
     119,    86,    87,    88,    89,    90,    91,    92,    93,    94,
       0,    95,     0,     0,     0,    96,    97,     0,     0,     0,
       0,   100,   101,   102,   103,     0,     0,   104,   105,   106,
     107,   108,     0,     0,     0,     0,     0,     0,     0,     0,
     114,   115,   116,   117,   118,   119,    88,    89,    90,    91,
      92,    93,    94,     0,    95,     0,     0,     0,    96,    97,
       0,     0,     0,     0,   100,   101,   102,   103,     0,     0,
      -1,   105,   106,   107,   108,    88,    89,    90,    91,    92,
      93,    94,     0,    95,     0,     0,     0,    96,    97,     0,
       0,     0,     0,   100,   101,   102,   103,     0,     0,   104,
     105,   106,   107,   108,    90,    91,    92,    93,    94,     0,
      95,     0,     0,     0,    96,    97,     0,     0,     0,     0,
       0,     0,   102,   103,     0,     0,     0,   105,   106,   107,
     108
};

static const yytype_int16 yycheck[] =
{
      20,    21,    22,    95,     4,    25,    26,    27,    28,    29,
      23,    31,    32,    33,    96,    25,    11,    12,    13,    36,
      15,    40,    36,     8,    43,    42,    25,    42,    23,    43,
       7,     8,     9,    10,    54,    45,    36,    57,    58,    59,
     122,    54,    42,     4,    43,   136,   137,    40,   139,    40,
      43,     7,    43,    73,     8,     8,    10,    10,    42,    79,
      80,    81,    82,    83,    84,    85,    86,    87,    88,    89,
      90,    91,    92,    93,    94,    95,    42,    97,    98,    99,
     100,   101,   186,   187,   104,   105,   106,   107,   108,   109,
     110,   111,   112,   113,   114,   115,   116,   117,   118,   119,
     120,   121,    23,    24,   186,   187,    42,     3,     4,    42,
       0,    25,    25,   133,     6,   135,    43,     8,    43,     8,
     140,     8,    18,    25,    40,    21,    22,    23,    24,   211,
      40,    43,    26,     3,   226,    31,     3,    42,    41,    35,
      36,    42,    41,    43,    78,    43,    42,    78,    44,   211,
      43,    -1,    48,    49,    -1,    -1,    -1,    54,    54,    55,
      56,    57,    58,    -1,    -1,    -1,    -1,    -1,    -1,    65,
      66,    -1,    -1,    -1,    -1,   195,   196,   197,    -1,    -1,
      -1,    -1,    -1,    -1,    -1,   205,    -1,    39,    40,    -1,
      42,    -1,    -1,   213,    46,    47,     5,    -1,    -1,   219,
      -1,    -1,    54,    55,    -1,   225,   226,    59,    60,    61,
      62,    -1,    -1,    -1,    -1,    -1,   236,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    -1,    42,    -1,    -1,    -1,    46,    47,    -1,
      -1,    50,    51,    52,    53,    54,    55,    -1,    -1,    58,
      59,    60,    61,    62,     5,    64,    -1,    -1,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    -1,    -1,
      -1,    80,    -1,    -1,    -1,    26,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      -1,    42,    43,    -1,    -1,    46,    47,    -1,    -1,    50,
      51,    52,    53,    54,    55,     5,    -1,    58,    59,    60,
      61,    62,    -1,    64,    -1,    -1,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    26,    27,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    -1,    42,    43,    -1,    -1,    46,    47,    -1,    -1,
      50,    51,    52,    53,    54,    55,     5,    -1,    58,    59,
      60,    61,    62,    -1,    64,    -1,    -1,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    26,    27,    28,
      29,    30,    31,    32,    33,    34,    35,    36,    37,    38,
      39,    40,    -1,    42,    43,    -1,    -1,    46,    47,    -1,
      -1,    50,    51,    52,    53,    54,    55,     5,    -1,    58,
      59,    60,    61,    62,    -1,    64,    -1,    -1,    67,    68,
      69,    70,    71,    72,    73,    74,    75,    76,    26,    27,
      28,    29,    30,    31,    32,    33,    34,    35,    36,    37,
      38,    39,    40,    -1,    42,    43,    -1,    -1,    46,    47,
      -1,    -1,    50,    51,    52,    53,    54,    55,     5,    -1,
      58,    59,    60,    61,    62,    -1,    64,    -1,    -1,    67,
      68,    69,    70,    71,    72,    73,    74,    75,    76,    26,
      27,    28,    29,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    41,    42,    -1,    -1,    -1,    46,
      47,    -1,    -1,    50,    51,    52,    53,    54,    55,     5,
      -1,    58,    59,    60,    61,    62,    -1,    64,    -1,    -1,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    -1,    42,    -1,    -1,    -1,
      46,    47,    -1,    -1,    50,    51,    52,    53,    54,    55,
      -1,    -1,    58,    59,    60,    61,    62,    63,    64,     5,
      -1,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    -1,    -1,    -1,    -1,    -1,    22,    -1,    -1,    -1,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    -1,    42,    -1,    -1,    -1,
      46,    47,    -1,    -1,    50,    51,    52,    53,    54,    55,
      -1,     5,    58,    59,    60,    61,    62,    -1,    64,    -1,
      -1,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    25,    26,    27,    28,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    -1,    42,    -1,
      -1,    -1,    46,    47,    -1,    -1,    50,    51,    52,    53,
      54,    55,     5,    -1,    58,    59,    60,    61,    62,    -1,
      64,    -1,    -1,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    26,    27,    28,    29,    30,    31,    32,
      33,    34,    35,    36,    37,    38,    39,    40,    -1,    42,
      -1,    -1,    -1,    46,    47,    -1,    -1,    50,    51,    52,
      53,    54,    55,     5,    -1,    58,    59,    60,    61,    62,
      -1,    64,    -1,    -1,    67,    68,    69,    70,    71,    72,
      73,    74,    75,    76,    26,    27,    28,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    -1,
      42,    -1,    -1,    -1,    46,    47,    -1,    -1,    50,    51,
      52,    53,    54,    55,     5,    -1,    58,    59,    60,    61,
      62,    -1,    -1,    -1,    -1,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    -1,    27,    28,    29,    30,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      -1,    42,    -1,    -1,    -1,    46,    47,    -1,    -1,    50,
      51,    52,    53,    54,    55,    -1,    -1,    58,    59,    60,
      61,    62,    -1,    -1,    -1,    -1,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,     3,     4,    -1,    -1,
       7,     8,     9,    10,    11,    12,    13,    14,    15,    16,
      17,    18,    19,    20,    21,    -1,    23,    24,    -1,    -1,
      -1,    -1,    -1,    -1,    31,    -1,    -1,    -1,    35,    36,
      -1,    -1,    -1,    -1,    -1,    42,    -1,    44,    -1,    -1,
      -1,    48,    49,    -1,    -1,     3,     4,    54,    55,    56,
      57,    58,    -1,    -1,    -1,    -1,    -1,    -1,    65,    66,
      18,    -1,    -1,    21,    -1,    23,    24,    -1,    -1,    -1,
      -1,    -1,    -1,    31,    -1,    -1,    -1,    35,    36,    -1,
      -1,    -1,    -1,    -1,    42,    -1,    44,    -1,    -1,    -1,
      48,    49,    -1,    -1,     3,     4,    54,    55,    56,    57,
      58,    -1,    -1,    -1,    -1,    -1,    -1,    65,    66,    18,
      -1,    -1,    21,    -1,    23,    24,    34,    35,    36,    37,
      38,    39,    40,    -1,    42,    -1,    35,    36,    46,    47,
      -1,    -1,    -1,    42,    -1,    44,    54,    55,    -1,    48,
      49,    59,    60,    61,    62,    54,    55,    56,    57,    58,
      -1,    -1,    -1,    -1,    -1,    -1,    65,    66,    28,    29,
      30,    31,    32,    33,    34,    35,    36,    37,    38,    39,
      40,    -1,    42,    -1,    -1,    -1,    46,    47,    -1,    -1,
      50,    51,    52,    53,    54,    55,    -1,    -1,    58,    59,
      60,    61,    62,    -1,    -1,    -1,    -1,    67,    68,    69,
      70,    71,    72,    73,    74,    75,    76,    29,    30,    31,
      32,    33,    34,    35,    36,    37,    38,    39,    40,    -1,
      42,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,    51,
      52,    53,    54,    55,    -1,    -1,    58,    59,    60,    61,
      62,    -1,    -1,    -1,    -1,    67,    68,    69,    70,    71,
      72,    73,    74,    75,    76,    29,    30,    31,    32,    33,
      34,    35,    36,    37,    38,    39,    40,    -1,    42,    -1,
      -1,    -1,    46,    47,    -1,    -1,    -1,    -1,    52,    53,
      54,    55,    -1,    -1,    58,    59,    60,    61,    62,    -1,
      -1,    -1,    -1,    67,    68,    69,    70,    71,    72,    73,
      74,    75,    76,    30,    31,    32,    33,    34,    35,    36,
      37,    38,    39,    40,    -1,    42,    -1,    -1,    -1,    46,
      47,    -1,    -1,    -1,    -1,    52,    53,    54,    55,    -1,
      -1,    58,    59,    60,    61,    62,    -1,    -1,    -1,    -1,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      31,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      -1,    42,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,
      -1,    52,    53,    54,    55,    -1,    -1,    58,    59,    60,
      61,    62,    -1,    -1,    -1,    -1,    67,    68,    69,    70,
      71,    72,    73,    74,    75,    76,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    -1,    42,    -1,    -1,    -1,
      46,    47,    -1,    -1,    -1,    -1,    52,    53,    54,    55,
      -1,    -1,    58,    59,    60,    61,    62,    -1,    -1,    -1,
      -1,    67,    68,    69,    70,    71,    72,    73,    74,    75,
      76,    32,    33,    34,    35,    36,    37,    38,    39,    40,
      -1,    42,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,
      -1,    52,    53,    54,    55,    -1,    -1,    58,    59,    60,
      61,    62,    -1,    -1,    -1,    -1,    -1,    -1,    -1,    -1,
      71,    72,    73,    74,    75,    76,    34,    35,    36,    37,
      38,    39,    40,    -1,    42,    -1,    -1,    -1,    46,    47,
      -1,    -1,    -1,    -1,    52,    53,    54,    55,    -1,    -1,
      58,    59,    60,    61,    62,    34,    35,    36,    37,    38,
      39,    40,    -1,    42,    -1,    -1,    -1,    46,    47,    -1,
      -1,    -1,    -1,    52,    53,    54,    55,    -1,    -1,    58,
      59,    60,    61,    62,    36,    37,    38,    39,    40,    -1,
      42,    -1,    -1,    -1,    46,    47,    -1,    -1,    -1,    -1,
      -1,    -1,    54,    55,    -1,    -1,    -1,    59,    60,    61,
      62
};

  /* YYSTOS[STATE-NUM] -- The (internal number of the) accessing
     symbol of state STATE-NUM.  */
static const yytype_uint8 yystos[] =
{
       0,     3,     4,     7,     8,     9,    10,    11,    12,    13,
      14,    15,    16,    17,    18,    19,    20,    21,    23,    24,
      31,    35,    36,    42,    44,    48,    49,    54,    55,    56,
      57,    58,    65,    66,    82,    83,    84,    85,    86,    91,
      92,    94,    97,    98,     8,     8,    10,     7,     8,     9,
      10,    98,    98,    98,    42,    98,     7,    42,    42,    42,
      91,    91,    91,    92,    95,    97,    98,    92,    91,    91,
      91,    91,    91,    42,    91,    91,    91,     0,    25,     5,
      26,    27,    28,    29,    30,    31,    32,    33,    34,    35,
      36,    37,    38,    39,    40,    42,    46,    47,    50,    51,
      52,    53,    54,    55,    58,    59,    60,    61,    62,    64,
      67,    68,    69,    70,    71,    72,    73,    74,    75,    76,
      25,     6,    87,     8,     8,     8,    10,    91,    95,    91,
      91,    93,    91,    25,    43,    43,    36,    42,    96,    36,
      43,    45,    91,    84,    86,    91,    91,    91,    91,    91,
      91,    91,    91,    91,    91,    91,    91,    91,    91,    91,
      91,    93,    94,    91,    91,    91,    91,    91,    91,    91,
      91,    91,    91,    91,    91,    91,    91,    91,    91,    91,
      91,    91,    91,    91,    91,    91,    36,    42,    88,    89,
      90,    94,     8,    43,    43,    43,    25,    43,    91,    96,
      96,    40,    96,    91,    43,    80,    41,    43,    63,    90,
      90,    26,    40,    22,    91,    91,    91,    43,     3,    43,
      91,    43,    89,     3,    91,    22,    25,    42,    41,    91,
      42,    41,    91,    93,    43,    43,    43,    91
};

  /* YYR1[YYN] -- Symbol number of symbol that rule YYN derives.  */
static const yytype_uint8 yyr1[] =
{
       0,    81,    82,    83,    83,    83,    83,    84,    84,    85,
      85,    87,    86,    88,    88,    89,    90,    90,    90,    90,
      90,    91,    91,    91,    91,    91,    91,    91,    91,    91,
      91,    91,    91,    91,    91,    91,    91,    91,    91,    91,
      91,    91,    91,    91,    91,    91,    91,    91,    91,    91,
      91,    91,    91,    91,    91,    91,    91,    91,    91,    91,
      91,    91,    91,    91,    91,    91,    91,    91,    91,    91,
      91,    91,    91,    91,    91,    91,    91,    91,    91,    91,
      91,    91,    91,    91,    91,    91,    91,    91,    91,    91,
      91,    92,    92,    93,    93,    91,    91,    94,    95,    96,
      96,    96,    96,    96,    97,    97,    97,    97,    97,    97,
      97,    97,    97,    97,    97,    97,    97,    97,    97,    97,
      97,    97,    97,    97,    97,    97,    97,    97,    97,    98
};

  /* YYR2[YYN] -- Number of symbols on the right hand side of rule YYN.  */
static const yytype_uint8 yyr2[] =
{
       0,     2,     1,     1,     2,     3,     1,     1,     2,     1,
       3,     0,     3,     1,     3,     1,     3,     5,     2,     4,
       1,     5,     6,     6,     7,     9,     8,     5,     4,     2,
       2,     2,     2,     2,     2,     2,     2,     2,     2,     2,
       2,     4,     4,     4,     3,     3,     3,     3,     3,     3,
       4,     4,     4,     3,     3,     4,     4,     6,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     3,     3,     3,     3,     3,     3,     3,     3,
       3,     3,     5,     3,     3,     3,     3,     2,     2,     3,
       3,     3,     1,     1,     0,     1,     1,     1,     2,     3,
       5,     2,     4,     0,     2,     1,     2,     2,     1,     1,
       2,     1,     2,     2,     3,     2,     3,     3,     4,     1,
       2,     2,     3,     1,     1,     1,     2,     2,     2,     1
};


#define yyerrok         (yyerrstatus = 0)
#define yyclearin       (yychar = YYEMPTY)
#define YYEMPTY         (-2)
#define YYEOF           0

#define YYACCEPT        goto yyacceptlab
#define YYABORT         goto yyabortlab
#define YYERROR         goto yyerrorlab


#define YYRECOVERING()  (!!yyerrstatus)

#define YYBACKUP(Token, Value)                                  \
do                                                              \
  if (yychar == YYEMPTY)                                        \
    {                                                           \
      yychar = (Token);                                         \
      yylval = (Value);                                         \
      YYPOPSTACK (yylen);                                       \
      yystate = *yyssp;                                         \
      goto yybackup;                                            \
    }                                                           \
  else                                                          \
    {                                                           \
      yyerror (YY_("syntax error: cannot back up")); \
      YYERROR;                                                  \
    }                                                           \
while (0)

/* Error token number */
#define YYTERROR        1
#define YYERRCODE       256



/* Enable debugging if requested.  */
#if YYDEBUG

# ifndef YYFPRINTF
#  include <stdio.h> /* INFRINGES ON USER NAME SPACE */
#  define YYFPRINTF fprintf
# endif

# define YYDPRINTF(Args)                        \
do {                                            \
  if (yydebug)                                  \
    YYFPRINTF Args;                             \
} while (0)

/* This macro is provided for backward compatibility. */
#ifndef YY_LOCATION_PRINT
# define YY_LOCATION_PRINT(File, Loc) ((void) 0)
#endif


# define YY_SYMBOL_PRINT(Title, Type, Value, Location)                    \
do {                                                                      \
  if (yydebug)                                                            \
    {                                                                     \
      YYFPRINTF (stderr, "%s ", Title);                                   \
      yy_symbol_print (stderr,                                            \
                  Type, Value); \
      YYFPRINTF (stderr, "\n");                                           \
    }                                                                     \
} while (0)


/*----------------------------------------.
| Print this symbol's value on YYOUTPUT.  |
`----------------------------------------*/

static void
yy_symbol_value_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  FILE *yyo = yyoutput;
  YYUSE (yyo);
  if (!yyvaluep)
    return;
# ifdef YYPRINT
  if (yytype < YYNTOKENS)
    YYPRINT (yyoutput, yytoknum[yytype], *yyvaluep);
# endif
  YYUSE (yytype);
}


/*--------------------------------.
| Print this symbol on YYOUTPUT.  |
`--------------------------------*/

static void
yy_symbol_print (FILE *yyoutput, int yytype, YYSTYPE const * const yyvaluep)
{
  YYFPRINTF (yyoutput, "%s %s (",
             yytype < YYNTOKENS ? "token" : "nterm", yytname[yytype]);

  yy_symbol_value_print (yyoutput, yytype, yyvaluep);
  YYFPRINTF (yyoutput, ")");
}

/*------------------------------------------------------------------.
| yy_stack_print -- Print the state stack from its BOTTOM up to its |
| TOP (included).                                                   |
`------------------------------------------------------------------*/

static void
yy_stack_print (yytype_int16 *yybottom, yytype_int16 *yytop)
{
  YYFPRINTF (stderr, "Stack now");
  for (; yybottom <= yytop; yybottom++)
    {
      int yybot = *yybottom;
      YYFPRINTF (stderr, " %d", yybot);
    }
  YYFPRINTF (stderr, "\n");
}

# define YY_STACK_PRINT(Bottom, Top)                            \
do {                                                            \
  if (yydebug)                                                  \
    yy_stack_print ((Bottom), (Top));                           \
} while (0)


/*------------------------------------------------.
| Report that the YYRULE is going to be reduced.  |
`------------------------------------------------*/

static void
yy_reduce_print (yytype_int16 *yyssp, YYSTYPE *yyvsp, int yyrule)
{
  unsigned long int yylno = yyrline[yyrule];
  int yynrhs = yyr2[yyrule];
  int yyi;
  YYFPRINTF (stderr, "Reducing stack by rule %d (line %lu):\n",
             yyrule - 1, yylno);
  /* The symbols being reduced.  */
  for (yyi = 0; yyi < yynrhs; yyi++)
    {
      YYFPRINTF (stderr, "   $%d = ", yyi + 1);
      yy_symbol_print (stderr,
                       yystos[yyssp[yyi + 1 - yynrhs]],
                       &(yyvsp[(yyi + 1) - (yynrhs)])
                                              );
      YYFPRINTF (stderr, "\n");
    }
}

# define YY_REDUCE_PRINT(Rule)          \
do {                                    \
  if (yydebug)                          \
    yy_reduce_print (yyssp, yyvsp, Rule); \
} while (0)

/* Nonzero means print parse trace.  It is left uninitialized so that
   multiple parsers can coexist.  */
int yydebug;
#else /* !YYDEBUG */
# define YYDPRINTF(Args)
# define YY_SYMBOL_PRINT(Title, Type, Value, Location)
# define YY_STACK_PRINT(Bottom, Top)
# define YY_REDUCE_PRINT(Rule)
#endif /* !YYDEBUG */


/* YYINITDEPTH -- initial size of the parser's stacks.  */
#ifndef YYINITDEPTH
# define YYINITDEPTH 200
#endif

/* YYMAXDEPTH -- maximum size the stacks can grow to (effective only
   if the built-in stack extension method is used).

   Do not make this value too large; the results are undefined if
   YYSTACK_ALLOC_MAXIMUM < YYSTACK_BYTES (YYMAXDEPTH)
   evaluated with infinite-precision integer arithmetic.  */

#ifndef YYMAXDEPTH
# define YYMAXDEPTH 10000
#endif


#if YYERROR_VERBOSE

# ifndef yystrlen
#  if defined __GLIBC__ && defined _STRING_H
#   define yystrlen strlen
#  else
/* Return the length of YYSTR.  */
static YYSIZE_T
yystrlen (const char *yystr)
{
  YYSIZE_T yylen;
  for (yylen = 0; yystr[yylen]; yylen++)
    continue;
  return yylen;
}
#  endif
# endif

# ifndef yystpcpy
#  if defined __GLIBC__ && defined _STRING_H && defined _GNU_SOURCE
#   define yystpcpy stpcpy
#  else
/* Copy YYSRC to YYDEST, returning the address of the terminating '\0' in
   YYDEST.  */
static char *
yystpcpy (char *yydest, const char *yysrc)
{
  char *yyd = yydest;
  const char *yys = yysrc;

  while ((*yyd++ = *yys++) != '\0')
    continue;

  return yyd - 1;
}
#  endif
# endif

# ifndef yytnamerr
/* Copy to YYRES the contents of YYSTR after stripping away unnecessary
   quotes and backslashes, so that it's suitable for yyerror.  The
   heuristic is that double-quoting is unnecessary unless the string
   contains an apostrophe, a comma, or backslash (other than
   backslash-backslash).  YYSTR is taken from yytname.  If YYRES is
   null, do not copy; instead, return the length of what the result
   would have been.  */
static YYSIZE_T
yytnamerr (char *yyres, const char *yystr)
{
  if (*yystr == '"')
    {
      YYSIZE_T yyn = 0;
      char const *yyp = yystr;

      for (;;)
        switch (*++yyp)
          {
          case '\'':
          case ',':
            goto do_not_strip_quotes;

          case '\\':
            if (*++yyp != '\\')
              goto do_not_strip_quotes;
            /* Fall through.  */
          default:
            if (yyres)
              yyres[yyn] = *yyp;
            yyn++;
            break;

          case '"':
            if (yyres)
              yyres[yyn] = '\0';
            return yyn;
          }
    do_not_strip_quotes: ;
    }

  if (! yyres)
    return yystrlen (yystr);

  return yystpcpy (yyres, yystr) - yyres;
}
# endif

/* Copy into *YYMSG, which is of size *YYMSG_ALLOC, an error message
   about the unexpected token YYTOKEN for the state stack whose top is
   YYSSP.

   Return 0 if *YYMSG was successfully written.  Return 1 if *YYMSG is
   not large enough to hold the message.  In that case, also set
   *YYMSG_ALLOC to the required number of bytes.  Return 2 if the
   required number of bytes is too large to store.  */
static int
yysyntax_error (YYSIZE_T *yymsg_alloc, char **yymsg,
                yytype_int16 *yyssp, int yytoken)
{
  YYSIZE_T yysize0 = yytnamerr (YY_NULLPTR, yytname[yytoken]);
  YYSIZE_T yysize = yysize0;
  enum { YYERROR_VERBOSE_ARGS_MAXIMUM = 5 };
  /* Internationalized format string. */
  const char *yyformat = YY_NULLPTR;
  /* Arguments of yyformat. */
  char const *yyarg[YYERROR_VERBOSE_ARGS_MAXIMUM];
  /* Number of reported tokens (one for the "unexpected", one per
     "expected"). */
  int yycount = 0;

  /* There are many possibilities here to consider:
     - If this state is a consistent state with a default action, then
       the only way this function was invoked is if the default action
       is an error action.  In that case, don't check for expected
       tokens because there are none.
     - The only way there can be no lookahead present (in yychar) is if
       this state is a consistent state with a default action.  Thus,
       detecting the absence of a lookahead is sufficient to determine
       that there is no unexpected or expected token to report.  In that
       case, just report a simple "syntax error".
     - Don't assume there isn't a lookahead just because this state is a
       consistent state with a default action.  There might have been a
       previous inconsistent state, consistent state with a non-default
       action, or user semantic action that manipulated yychar.
     - Of course, the expected token list depends on states to have
       correct lookahead information, and it depends on the parser not
       to perform extra reductions after fetching a lookahead from the
       scanner and before detecting a syntax error.  Thus, state merging
       (from LALR or IELR) and default reductions corrupt the expected
       token list.  However, the list is correct for canonical LR with
       one exception: it will still contain any token that will not be
       accepted due to an error action in a later state.
  */
  if (yytoken != YYEMPTY)
    {
      int yyn = yypact[*yyssp];
      yyarg[yycount++] = yytname[yytoken];
      if (!yypact_value_is_default (yyn))
        {
          /* Start YYX at -YYN if negative to avoid negative indexes in
             YYCHECK.  In other words, skip the first -YYN actions for
             this state because they are default actions.  */
          int yyxbegin = yyn < 0 ? -yyn : 0;
          /* Stay within bounds of both yycheck and yytname.  */
          int yychecklim = YYLAST - yyn + 1;
          int yyxend = yychecklim < YYNTOKENS ? yychecklim : YYNTOKENS;
          int yyx;

          for (yyx = yyxbegin; yyx < yyxend; ++yyx)
            if (yycheck[yyx + yyn] == yyx && yyx != YYTERROR
                && !yytable_value_is_error (yytable[yyx + yyn]))
              {
                if (yycount == YYERROR_VERBOSE_ARGS_MAXIMUM)
                  {
                    yycount = 1;
                    yysize = yysize0;
                    break;
                  }
                yyarg[yycount++] = yytname[yyx];
                {
                  YYSIZE_T yysize1 = yysize + yytnamerr (YY_NULLPTR, yytname[yyx]);
                  if (! (yysize <= yysize1
                         && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
                    return 2;
                  yysize = yysize1;
                }
              }
        }
    }

  switch (yycount)
    {
# define YYCASE_(N, S)                      \
      case N:                               \
        yyformat = S;                       \
      break
      YYCASE_(0, YY_("syntax error"));
      YYCASE_(1, YY_("syntax error, unexpected %s"));
      YYCASE_(2, YY_("syntax error, unexpected %s, expecting %s"));
      YYCASE_(3, YY_("syntax error, unexpected %s, expecting %s or %s"));
      YYCASE_(4, YY_("syntax error, unexpected %s, expecting %s or %s or %s"));
      YYCASE_(5, YY_("syntax error, unexpected %s, expecting %s or %s or %s or %s"));
# undef YYCASE_
    }

  {
    YYSIZE_T yysize1 = yysize + yystrlen (yyformat);
    if (! (yysize <= yysize1 && yysize1 <= YYSTACK_ALLOC_MAXIMUM))
      return 2;
    yysize = yysize1;
  }

  if (*yymsg_alloc < yysize)
    {
      *yymsg_alloc = 2 * yysize;
      if (! (yysize <= *yymsg_alloc
             && *yymsg_alloc <= YYSTACK_ALLOC_MAXIMUM))
        *yymsg_alloc = YYSTACK_ALLOC_MAXIMUM;
      return 1;
    }

  /* Avoid sprintf, as that infringes on the user's name space.
     Don't have undefined behavior even if the translation
     produced a string with the wrong number of "%s"s.  */
  {
    char *yyp = *yymsg;
    int yyi = 0;
    while ((*yyp = *yyformat) != '\0')
      if (*yyp == '%' && yyformat[1] == 's' && yyi < yycount)
        {
          yyp += yytnamerr (yyp, yyarg[yyi++]);
          yyformat += 2;
        }
      else
        {
          yyp++;
          yyformat++;
        }
  }
  return 0;
}
#endif /* YYERROR_VERBOSE */

/*-----------------------------------------------.
| Release the memory associated to this symbol.  |
`-----------------------------------------------*/

static void
yydestruct (const char *yymsg, int yytype, YYSTYPE *yyvaluep)
{
  YYUSE (yyvaluep);
  if (!yymsg)
    yymsg = "Deleting";
  YY_SYMBOL_PRINT (yymsg, yytype, yyvaluep, yylocationp);

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  YYUSE (yytype);
  YY_IGNORE_MAYBE_UNINITIALIZED_END
}




/* The lookahead symbol.  */
int yychar;

/* The semantic value of the lookahead symbol.  */
YYSTYPE yylval;
/* Number of syntax errors so far.  */
int yynerrs;


/*----------.
| yyparse.  |
`----------*/

int
yyparse (void)
{
    int yystate;
    /* Number of tokens to shift before error messages enabled.  */
    int yyerrstatus;

    /* The stacks and their tools:
       'yyss': related to states.
       'yyvs': related to semantic values.

       Refer to the stacks through separate pointers, to allow yyoverflow
       to reallocate them elsewhere.  */

    /* The state stack.  */
    yytype_int16 yyssa[YYINITDEPTH];
    yytype_int16 *yyss;
    yytype_int16 *yyssp;

    /* The semantic value stack.  */
    YYSTYPE yyvsa[YYINITDEPTH];
    YYSTYPE *yyvs;
    YYSTYPE *yyvsp;

    YYSIZE_T yystacksize;

  int yyn;
  int yyresult;
  /* Lookahead token as an internal (translated) token number.  */
  int yytoken = 0;
  /* The variables used to return semantic value and location from the
     action routines.  */
  YYSTYPE yyval;

#if YYERROR_VERBOSE
  /* Buffer for error messages, and its allocated size.  */
  char yymsgbuf[128];
  char *yymsg = yymsgbuf;
  YYSIZE_T yymsg_alloc = sizeof yymsgbuf;
#endif

#define YYPOPSTACK(N)   (yyvsp -= (N), yyssp -= (N))

  /* The number of symbols on the RHS of the reduced rule.
     Keep to zero when no symbol should be popped.  */
  int yylen = 0;

  yyssp = yyss = yyssa;
  yyvsp = yyvs = yyvsa;
  yystacksize = YYINITDEPTH;

  YYDPRINTF ((stderr, "Starting parse\n"));

  yystate = 0;
  yyerrstatus = 0;
  yynerrs = 0;
  yychar = YYEMPTY; /* Cause a token to be read.  */
  goto yysetstate;

/*------------------------------------------------------------.
| yynewstate -- Push a new state, which is found in yystate.  |
`------------------------------------------------------------*/
 yynewstate:
  /* In all cases, when you get here, the value and location stacks
     have just been pushed.  So pushing a state here evens the stacks.  */
  yyssp++;

 yysetstate:
  *yyssp = yystate;

  if (yyss + yystacksize - 1 <= yyssp)
    {
      /* Get the current used size of the three stacks, in elements.  */
      YYSIZE_T yysize = yyssp - yyss + 1;

#ifdef yyoverflow
      {
        /* Give user a chance to reallocate the stack.  Use copies of
           these so that the &'s don't force the real ones into
           memory.  */
        YYSTYPE *yyvs1 = yyvs;
        yytype_int16 *yyss1 = yyss;

        /* Each stack pointer address is followed by the size of the
           data in use in that stack, in bytes.  This used to be a
           conditional around just the two extra args, but that might
           be undefined if yyoverflow is a macro.  */
        yyoverflow (YY_("memory exhausted"),
                    &yyss1, yysize * sizeof (*yyssp),
                    &yyvs1, yysize * sizeof (*yyvsp),
                    &yystacksize);

        yyss = yyss1;
        yyvs = yyvs1;
      }
#else /* no yyoverflow */
# ifndef YYSTACK_RELOCATE
      goto yyexhaustedlab;
# else
      /* Extend the stack our own way.  */
      if (YYMAXDEPTH <= yystacksize)
        goto yyexhaustedlab;
      yystacksize *= 2;
      if (YYMAXDEPTH < yystacksize)
        yystacksize = YYMAXDEPTH;

      {
        yytype_int16 *yyss1 = yyss;
        union yyalloc *yyptr =
          (union yyalloc *) YYSTACK_ALLOC (YYSTACK_BYTES (yystacksize));
        if (! yyptr)
          goto yyexhaustedlab;
        YYSTACK_RELOCATE (yyss_alloc, yyss);
        YYSTACK_RELOCATE (yyvs_alloc, yyvs);
#  undef YYSTACK_RELOCATE
        if (yyss1 != yyssa)
          YYSTACK_FREE (yyss1);
      }
# endif
#endif /* no yyoverflow */

      yyssp = yyss + yysize - 1;
      yyvsp = yyvs + yysize - 1;

      YYDPRINTF ((stderr, "Stack size increased to %lu\n",
                  (unsigned long int) yystacksize));

      if (yyss + yystacksize - 1 <= yyssp)
        YYABORT;
    }

  YYDPRINTF ((stderr, "Entering state %d\n", yystate));

  if (yystate == YYFINAL)
    YYACCEPT;

  goto yybackup;

/*-----------.
| yybackup.  |
`-----------*/
yybackup:

  /* Do appropriate processing given the current state.  Read a
     lookahead token if we need one and don't already have one.  */

  /* First try to decide what to do without reference to lookahead token.  */
  yyn = yypact[yystate];
  if (yypact_value_is_default (yyn))
    goto yydefault;

  /* Not known => get a lookahead token if don't already have one.  */

  /* YYCHAR is either YYEMPTY or YYEOF or a valid lookahead symbol.  */
  if (yychar == YYEMPTY)
    {
      YYDPRINTF ((stderr, "Reading a token: "));
      yychar = yylex ();
    }

  if (yychar <= YYEOF)
    {
      yychar = yytoken = YYEOF;
      YYDPRINTF ((stderr, "Now at end of input.\n"));
    }
  else
    {
      yytoken = YYTRANSLATE (yychar);
      YY_SYMBOL_PRINT ("Next token is", yytoken, &yylval, &yylloc);
    }

  /* If the proper action on seeing token YYTOKEN is to reduce or to
     detect an error, take that action.  */
  yyn += yytoken;
  if (yyn < 0 || YYLAST < yyn || yycheck[yyn] != yytoken)
    goto yydefault;
  yyn = yytable[yyn];
  if (yyn <= 0)
    {
      if (yytable_value_is_error (yyn))
        goto yyerrlab;
      yyn = -yyn;
      goto yyreduce;
    }

  /* Count tokens shifted since error; after three, turn off error
     status.  */
  if (yyerrstatus)
    yyerrstatus--;

  /* Shift the lookahead token.  */
  YY_SYMBOL_PRINT ("Shifting", yytoken, &yylval, &yylloc);

  /* Discard the shifted token.  */
  yychar = YYEMPTY;

  yystate = yyn;
  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END

  goto yynewstate;


/*-----------------------------------------------------------.
| yydefault -- do the default action for the current state.  |
`-----------------------------------------------------------*/
yydefault:
  yyn = yydefact[yystate];
  if (yyn == 0)
    goto yyerrlab;
  goto yyreduce;


/*-----------------------------.
| yyreduce -- Do a reduction.  |
`-----------------------------*/
yyreduce:
  /* yyn is the number of a rule to reduce with.  */
  yylen = yyr2[yyn];

  /* If YYLEN is nonzero, implement the default value of the action:
     '$$ = $1'.

     Otherwise, the following line sets YYVAL to garbage.
     This behavior is undocumented and Bison
     users should not rely upon it.  Assigning to YYVAL
     unconditionally makes the parser a bit smaller, and it avoids a
     GCC warning that YYVAL may be used uninitialized.  */
  yyval = yyvsp[1-yylen];


  YY_REDUCE_PRINT (yyn);
  switch (yyn)
    {
        case 2:
#line 211 "parse.y" /* yacc.c:1646  */
    { root=(yyvsp[0].node) ; }
#line 1878 "y.tab.c" /* yacc.c:1646  */
    break;

  case 5:
#line 216 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_sbin((yyvsp[-1].opinfo),(yyvsp[-2].node),(yyvsp[0].node));}
#line 1884 "y.tab.c" /* yacc.c:1646  */
    break;

  case 8:
#line 220 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_sbin((yyvsp[0].opinfo),(yyvsp[-1].node),0); }
#line 1890 "y.tab.c" /* yacc.c:1646  */
    break;

  case 10:
#line 223 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_sbin((yyvsp[-1].opinfo),(yyvsp[-2].node),(yyvsp[0].node)); }
#line 1896 "y.tab.c" /* yacc.c:1646  */
    break;

  case 11:
#line 226 "parse.y" /* yacc.c:1646  */
    { decl_tbase=(yyvsp[0].ctype) ; }
#line 1902 "y.tab.c" /* yacc.c:1646  */
    break;

  case 12:
#line 226 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=(yyvsp[0].node) ; }
#line 1908 "y.tab.c" /* yacc.c:1646  */
    break;

  case 14:
#line 229 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_sbin(seq_op,(yyvsp[-2].node),(yyvsp[0].node)); }
#line 1914 "y.tab.c" /* yacc.c:1646  */
    break;

  case 15:
#line 232 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_sbin(decl_op,(yyvsp[0].node),
 				   mknode_modified_ctype(decl_tbase)); }
#line 1921 "y.tab.c" /* yacc.c:1646  */
    break;

  case 16:
#line 236 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=(yyvsp[-1].node) ; }
#line 1927 "y.tab.c" /* yacc.c:1646  */
    break;

  case 17:
#line 237 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=(yyvsp[-3].node) ; push_type('('); }
#line 1933 "y.tab.c" /* yacc.c:1646  */
    break;

  case 18:
#line 238 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=(yyvsp[0].node) ; push_type('*'); }
#line 1939 "y.tab.c" /* yacc.c:1646  */
    break;

  case 19:
#line 239 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=(yyvsp[-3].node) ; push_type_int('[',(yyvsp[-1].node)); }
#line 1945 "y.tab.c" /* yacc.c:1646  */
    break;

  case 21:
#line 251 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_tri((yyvsp[-4].opinfo),(yyvsp[-2].node),(yyvsp[0].node),0); }
#line 1951 "y.tab.c" /* yacc.c:1646  */
    break;

  case 22:
#line 253 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_tri((yyvsp[-5].opinfo),(yyvsp[-3].node),(yyvsp[-1].node),0); }
#line 1957 "y.tab.c" /* yacc.c:1646  */
    break;

  case 23:
#line 255 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_tri((yyvsp[-5].opinfo),(yyvsp[-3].node),0,(yyvsp[0].node)); }
#line 1963 "y.tab.c" /* yacc.c:1646  */
    break;

  case 24:
#line 257 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_tri((yyvsp[-6].opinfo),(yyvsp[-4].node),(yyvsp[-2].node),(yyvsp[0].node)); }
#line 1969 "y.tab.c" /* yacc.c:1646  */
    break;

  case 25:
#line 260 "parse.y" /* yacc.c:1646  */
    {  (yyval.node)=mknode_op(OPK_QUAD,(yyvsp[-8].opinfo),(yyvsp[-6].node),(yyvsp[-4].node),(yyvsp[-2].node),(yyvsp[0].node)); }
#line 1975 "y.tab.c" /* yacc.c:1646  */
    break;

  case 26:
#line 262 "parse.y" /* yacc.c:1646  */
    {  (yyval.node)=mknode_op(OPK_QUAD,(yyvsp[-7].opinfo),(yyvsp[-5].node),(yyvsp[-3].node),(yyvsp[-1].node),0); }
#line 1981 "y.tab.c" /* yacc.c:1646  */
    break;

  case 27:
#line 264 "parse.y" /* yacc.c:1646  */
    {  (yyval.node)=mknode_sbin((yyvsp[-4].opinfo),(yyvsp[-2].node),(yyvsp[0].node)); }
#line 1987 "y.tab.c" /* yacc.c:1646  */
    break;

  case 28:
#line 266 "parse.y" /* yacc.c:1646  */
    {  (yyval.node)=mknode_sbin((yyvsp[-3].opinfo),(yyvsp[-1].node),0); }
#line 1993 "y.tab.c" /* yacc.c:1646  */
    break;

  case 29:
#line 271 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_unary( (yyvsp[-1].opinfo),(yyvsp[0].node)); }
#line 1999 "y.tab.c" /* yacc.c:1646  */
    break;

  case 30:
#line 272 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_unary( (yyvsp[-1].opinfo),(yyvsp[0].node)); }
#line 2005 "y.tab.c" /* yacc.c:1646  */
    break;

  case 31:
#line 273 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_unary( (yyvsp[-1].opinfo),(yyvsp[0].node)); }
#line 2011 "y.tab.c" /* yacc.c:1646  */
    break;

  case 32:
#line 274 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_unary( (yyvsp[-1].opinfo),(yyvsp[0].node)); }
#line 2017 "y.tab.c" /* yacc.c:1646  */
    break;

  case 33:
#line 275 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_unary( (yyvsp[-1].opinfo),(yyvsp[0].node)); }
#line 2023 "y.tab.c" /* yacc.c:1646  */
    break;

  case 34:
#line 276 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_sunary((yyvsp[-1].opinfo),(yyvsp[0].node)); }
#line 2029 "y.tab.c" /* yacc.c:1646  */
    break;

  case 35:
#line 277 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_sunary((yyvsp[-1].opinfo),(yyvsp[0].node)); }
#line 2035 "y.tab.c" /* yacc.c:1646  */
    break;

  case 36:
#line 278 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_sunary((yyvsp[-1].opinfo),(yyvsp[0].node)); }
#line 2041 "y.tab.c" /* yacc.c:1646  */
    break;

  case 37:
#line 279 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_unary( (yyvsp[-1].opinfo),(yyvsp[0].node)); }
#line 2047 "y.tab.c" /* yacc.c:1646  */
    break;

  case 38:
#line 280 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_unary( (yyvsp[-1].opinfo),(yyvsp[0].node)); }
#line 2053 "y.tab.c" /* yacc.c:1646  */
    break;

  case 39:
#line 281 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_post_unary((yyvsp[0].opinfo),(yyvsp[-1].node)); }
#line 2059 "y.tab.c" /* yacc.c:1646  */
    break;

  case 40:
#line 282 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_post_unary((yyvsp[0].opinfo),(yyvsp[-1].node)); }
#line 2065 "y.tab.c" /* yacc.c:1646  */
    break;

  case 41:
#line 283 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_sunary((yyvsp[-3].opinfo),(yyvsp[-1].node)); }
#line 2071 "y.tab.c" /* yacc.c:1646  */
    break;

  case 42:
#line 284 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_unary((yyvsp[-3].opinfo),(yyvsp[-1].node)); }
#line 2077 "y.tab.c" /* yacc.c:1646  */
    break;

  case 43:
#line 285 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_unary( (yyvsp[-3].opinfo),(yyvsp[-1].node)); }
#line 2083 "y.tab.c" /* yacc.c:1646  */
    break;

  case 44:
#line 288 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_sbin((yyvsp[-1].opinfo),(yyvsp[-2].node),(yyvsp[0].node)); }
#line 2089 "y.tab.c" /* yacc.c:1646  */
    break;

  case 45:
#line 289 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_sbin((yyvsp[-1].opinfo),(yyvsp[-2].node),(yyvsp[0].node)); }
#line 2095 "y.tab.c" /* yacc.c:1646  */
    break;

  case 46:
#line 291 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_sbin((yyvsp[-1].opinfo),(yyvsp[-2].node),(yyvsp[0].node)); }
#line 2101 "y.tab.c" /* yacc.c:1646  */
    break;

  case 47:
#line 292 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_sbin((yyvsp[-1].opinfo),(yyvsp[-2].node),(yyvsp[0].node)); }
#line 2107 "y.tab.c" /* yacc.c:1646  */
    break;

  case 48:
#line 293 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_sbin((yyvsp[-1].opinfo),(yyvsp[-2].node),(yyvsp[0].node)); }
#line 2113 "y.tab.c" /* yacc.c:1646  */
    break;

  case 49:
#line 294 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_sbin((yyvsp[-1].opinfo),(yyvsp[-2].node),(yyvsp[0].node)); }
#line 2119 "y.tab.c" /* yacc.c:1646  */
    break;

  case 50:
#line 295 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_bin( (yyvsp[-2].opinfo),(yyvsp[-3].node),(yyvsp[-1].node)); }
#line 2125 "y.tab.c" /* yacc.c:1646  */
    break;

  case 51:
#line 296 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_sbin((yyvsp[-2].opinfo),(yyvsp[-3].node),(yyvsp[-1].node)); }
#line 2131 "y.tab.c" /* yacc.c:1646  */
    break;

  case 52:
#line 297 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_op(OPK_FUNC,(yyvsp[-2].opinfo),(yyvsp[-3].node),(yyvsp[-1].node),0,0); }
#line 2137 "y.tab.c" /* yacc.c:1646  */
    break;

  case 53:
#line 298 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_unary((yyvsp[-2].opinfo),(yyvsp[-1].node)); }
#line 2143 "y.tab.c" /* yacc.c:1646  */
    break;

  case 54:
#line 299 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_unary((yyvsp[-2].opinfo),(yyvsp[-1].node)); }
#line 2149 "y.tab.c" /* yacc.c:1646  */
    break;

  case 55:
#line 303 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_op(OPK_CAST,(yyvsp[-3].opinfo),(yyvsp[-2].node),(yyvsp[0].node),0,0); }
#line 2155 "y.tab.c" /* yacc.c:1646  */
    break;

  case 56:
#line 313 "parse.y" /* yacc.c:1646  */
    {
                  tctype *t=duel_get_target_typedef((yyvsp[-2].nameinfo).name);
                  if(t==NULL) yyerror("not a typedef name");
                  (yyval.node)=mknode_op(OPK_CAST,(yyvsp[-3].opinfo),mknode_ctype(t),(yyvsp[0].node),0,0); }
#line 2164 "y.tab.c" /* yacc.c:1646  */
    break;

  case 57:
#line 317 "parse.y" /* yacc.c:1646  */
    {
                 tctype *t=duel_get_target_typedef((yyvsp[-4].nameinfo).name);
                 if(t==NULL) yyerror("not a typedef name");
                 push_type('*');
                 (yyval.node)=mknode_op(OPK_CAST,(yyvsp[-5].opinfo),mknode_modified_ctype(t),(yyvsp[0].node),0,0); }
#line 2174 "y.tab.c" /* yacc.c:1646  */
    break;

  case 58:
#line 326 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_bin((yyvsp[-1].opinfo),(yyvsp[-2].node),(yyvsp[0].node)); }
#line 2180 "y.tab.c" /* yacc.c:1646  */
    break;

  case 59:
#line 327 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_bin((yyvsp[-1].opinfo),(yyvsp[-2].node),(yyvsp[0].node)); }
#line 2186 "y.tab.c" /* yacc.c:1646  */
    break;

  case 60:
#line 328 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_bin((yyvsp[-1].opinfo),(yyvsp[-2].node),(yyvsp[0].node)); }
#line 2192 "y.tab.c" /* yacc.c:1646  */
    break;

  case 61:
#line 329 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_bin((yyvsp[-1].opinfo),(yyvsp[-2].node),(yyvsp[0].node)); }
#line 2198 "y.tab.c" /* yacc.c:1646  */
    break;

  case 62:
#line 330 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_bin((yyvsp[-1].opinfo),(yyvsp[-2].node),(yyvsp[0].node)); }
#line 2204 "y.tab.c" /* yacc.c:1646  */
    break;

  case 63:
#line 331 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_bin((yyvsp[-1].opinfo),(yyvsp[-2].node),(yyvsp[0].node)); }
#line 2210 "y.tab.c" /* yacc.c:1646  */
    break;

  case 64:
#line 332 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_bin((yyvsp[-1].opinfo),(yyvsp[-2].node),(yyvsp[0].node)); }
#line 2216 "y.tab.c" /* yacc.c:1646  */
    break;

  case 65:
#line 333 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_bin((yyvsp[-1].opinfo),(yyvsp[-2].node),(yyvsp[0].node)); }
#line 2222 "y.tab.c" /* yacc.c:1646  */
    break;

  case 66:
#line 334 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_bin((yyvsp[-1].opinfo),(yyvsp[-2].node),(yyvsp[0].node)); }
#line 2228 "y.tab.c" /* yacc.c:1646  */
    break;

  case 67:
#line 335 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_bin((yyvsp[-1].opinfo),(yyvsp[-2].node),(yyvsp[0].node)); }
#line 2234 "y.tab.c" /* yacc.c:1646  */
    break;

  case 68:
#line 336 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_bin((yyvsp[-1].opinfo),(yyvsp[-2].node),(yyvsp[0].node)); }
#line 2240 "y.tab.c" /* yacc.c:1646  */
    break;

  case 69:
#line 337 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_bin((yyvsp[-1].opinfo),(yyvsp[-2].node),(yyvsp[0].node)); }
#line 2246 "y.tab.c" /* yacc.c:1646  */
    break;

  case 70:
#line 338 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_bin((yyvsp[-1].opinfo),(yyvsp[-2].node),(yyvsp[0].node)); }
#line 2252 "y.tab.c" /* yacc.c:1646  */
    break;

  case 71:
#line 339 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_bin((yyvsp[-1].opinfo),(yyvsp[-2].node),(yyvsp[0].node)); }
#line 2258 "y.tab.c" /* yacc.c:1646  */
    break;

  case 72:
#line 340 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_bin((yyvsp[-1].opinfo),(yyvsp[-2].node),(yyvsp[0].node)); }
#line 2264 "y.tab.c" /* yacc.c:1646  */
    break;

  case 73:
#line 341 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_bin((yyvsp[-1].opinfo),(yyvsp[-2].node),(yyvsp[0].node)); }
#line 2270 "y.tab.c" /* yacc.c:1646  */
    break;

  case 74:
#line 342 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_bin((yyvsp[-1].opinfo),(yyvsp[-2].node),(yyvsp[0].node)); }
#line 2276 "y.tab.c" /* yacc.c:1646  */
    break;

  case 75:
#line 343 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_bin((yyvsp[-1].opinfo),(yyvsp[-2].node),(yyvsp[0].node)); }
#line 2282 "y.tab.c" /* yacc.c:1646  */
    break;

  case 76:
#line 344 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_bin((yyvsp[-1].opinfo),(yyvsp[-2].node),(yyvsp[0].node)); }
#line 2288 "y.tab.c" /* yacc.c:1646  */
    break;

  case 77:
#line 345 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_bin((yyvsp[-1].opinfo),(yyvsp[-2].node),(yyvsp[0].node)); }
#line 2294 "y.tab.c" /* yacc.c:1646  */
    break;

  case 78:
#line 346 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_bin((yyvsp[-1].opinfo),(yyvsp[-2].node),(yyvsp[0].node)); }
#line 2300 "y.tab.c" /* yacc.c:1646  */
    break;

  case 79:
#line 347 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_bin((yyvsp[-1].opinfo),(yyvsp[-2].node),(yyvsp[0].node)); }
#line 2306 "y.tab.c" /* yacc.c:1646  */
    break;

  case 80:
#line 348 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_sbin((yyvsp[-1].opinfo),(yyvsp[-2].node),(yyvsp[0].node)); }
#line 2312 "y.tab.c" /* yacc.c:1646  */
    break;

  case 81:
#line 349 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_sbin((yyvsp[-1].opinfo),(yyvsp[-2].node),(yyvsp[0].node)); }
#line 2318 "y.tab.c" /* yacc.c:1646  */
    break;

  case 82:
#line 353 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_tri((yyvsp[-3].opinfo),(yyvsp[-4].node),(yyvsp[-2].node),(yyvsp[0].node)); }
#line 2324 "y.tab.c" /* yacc.c:1646  */
    break;

  case 83:
#line 356 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_bin((yyvsp[-1].opinfo),(yyvsp[-2].node),(yyvsp[0].node)); }
#line 2330 "y.tab.c" /* yacc.c:1646  */
    break;

  case 84:
#line 357 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_op(OPK_ASSIGN,(yyvsp[-1].opinfo), (yyvsp[-2].node),(yyvsp[0].node),0,0);  }
#line 2336 "y.tab.c" /* yacc.c:1646  */
    break;

  case 85:
#line 358 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_sbin((yyvsp[-1].opinfo),(yyvsp[-2].node),(yyvsp[0].node));  }
#line 2342 "y.tab.c" /* yacc.c:1646  */
    break;

  case 86:
#line 363 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_sbin((yyvsp[-1].opinfo),(yyvsp[-2].node),(yyvsp[0].node)); }
#line 2348 "y.tab.c" /* yacc.c:1646  */
    break;

  case 87:
#line 364 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_sbin((yyvsp[-1].opinfo), 0,(yyvsp[0].node)); }
#line 2354 "y.tab.c" /* yacc.c:1646  */
    break;

  case 88:
#line 365 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_sbin((yyvsp[0].opinfo),(yyvsp[-1].node), 0); }
#line 2360 "y.tab.c" /* yacc.c:1646  */
    break;

  case 89:
#line 366 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_sbin((yyvsp[-1].opinfo),(yyvsp[-2].node),(yyvsp[0].node)); }
#line 2366 "y.tab.c" /* yacc.c:1646  */
    break;

  case 90:
#line 367 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_sbin((yyvsp[-1].opinfo),(yyvsp[-2].node),(yyvsp[0].node)); }
#line 2372 "y.tab.c" /* yacc.c:1646  */
    break;

  case 91:
#line 370 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_sbin((yyvsp[-1].opinfo),(yyvsp[-2].node),(yyvsp[0].node)); }
#line 2378 "y.tab.c" /* yacc.c:1646  */
    break;

  case 94:
#line 375 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=0 ; }
#line 2384 "y.tab.c" /* yacc.c:1646  */
    break;

  case 97:
#line 385 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_name((yyvsp[0].nameinfo)) ; }
#line 2390 "y.tab.c" /* yacc.c:1646  */
    break;

  case 98:
#line 387 "parse.y" /* yacc.c:1646  */
    { (yyval.node)=mknode_modified_ctype((yyvsp[-1].ctype)); }
#line 2396 "y.tab.c" /* yacc.c:1646  */
    break;

  case 100:
#line 393 "parse.y" /* yacc.c:1646  */
    {  push_type('('); }
#line 2402 "y.tab.c" /* yacc.c:1646  */
    break;

  case 101:
#line 394 "parse.y" /* yacc.c:1646  */
    {  push_type('*'); }
#line 2408 "y.tab.c" /* yacc.c:1646  */
    break;

  case 102:
#line 395 "parse.y" /* yacc.c:1646  */
    {  push_type_int('[',(yyvsp[-1].node)); }
#line 2414 "y.tab.c" /* yacc.c:1646  */
    break;

  case 104:
#line 413 "parse.y" /* yacc.c:1646  */
    {
                    (yyval.ctype)=duel_get_target_typedef((yyvsp[0].nameinfo).name);
		    if((yyval.ctype)==NULL) {
		       tvalue v;
		       if(duel_get_target_variable((yyvsp[0].nameinfo).name,-1,&v)) (yyval.ctype)=v.ctype;
		       else  yyerror("not a typedef name");
		    }
		}
#line 2427 "y.tab.c" /* yacc.c:1646  */
    break;

  case 105:
#line 423 "parse.y" /* yacc.c:1646  */
    { (yyval.ctype) = ctype_char;  }
#line 2433 "y.tab.c" /* yacc.c:1646  */
    break;

  case 106:
#line 424 "parse.y" /* yacc.c:1646  */
    { (yyval.ctype) = ctype_schar; }
#line 2439 "y.tab.c" /* yacc.c:1646  */
    break;

  case 107:
#line 425 "parse.y" /* yacc.c:1646  */
    { (yyval.ctype) = ctype_uchar; }
#line 2445 "y.tab.c" /* yacc.c:1646  */
    break;

  case 108:
#line 426 "parse.y" /* yacc.c:1646  */
    { (yyval.ctype) = ctype_int;   }
#line 2451 "y.tab.c" /* yacc.c:1646  */
    break;

  case 109:
#line 427 "parse.y" /* yacc.c:1646  */
    { (yyval.ctype) = ctype_uint;  }
#line 2457 "y.tab.c" /* yacc.c:1646  */
    break;

  case 110:
#line 428 "parse.y" /* yacc.c:1646  */
    { (yyval.ctype) = ctype_uint;  }
#line 2463 "y.tab.c" /* yacc.c:1646  */
    break;

  case 111:
#line 429 "parse.y" /* yacc.c:1646  */
    { (yyval.ctype) = ctype_long;  }
#line 2469 "y.tab.c" /* yacc.c:1646  */
    break;

  case 112:
#line 430 "parse.y" /* yacc.c:1646  */
    { (yyval.ctype) = ctype_long;  }
#line 2475 "y.tab.c" /* yacc.c:1646  */
    break;

  case 113:
#line 431 "parse.y" /* yacc.c:1646  */
    { (yyval.ctype) = ctype_ulong; }
#line 2481 "y.tab.c" /* yacc.c:1646  */
    break;

  case 114:
#line 432 "parse.y" /* yacc.c:1646  */
    { (yyval.ctype) = ctype_ulong; }
#line 2487 "y.tab.c" /* yacc.c:1646  */
    break;

  case 115:
#line 433 "parse.y" /* yacc.c:1646  */
    { (yyval.ctype) = ctype_longlong; }
#line 2493 "y.tab.c" /* yacc.c:1646  */
    break;

  case 116:
#line 434 "parse.y" /* yacc.c:1646  */
    { (yyval.ctype) = ctype_longlong; }
#line 2499 "y.tab.c" /* yacc.c:1646  */
    break;

  case 117:
#line 435 "parse.y" /* yacc.c:1646  */
    { (yyval.ctype) = ctype_ulonglong; }
#line 2505 "y.tab.c" /* yacc.c:1646  */
    break;

  case 118:
#line 436 "parse.y" /* yacc.c:1646  */
    { (yyval.ctype) = ctype_ulonglong; }
#line 2511 "y.tab.c" /* yacc.c:1646  */
    break;

  case 119:
#line 437 "parse.y" /* yacc.c:1646  */
    { (yyval.ctype) = ctype_short; }
#line 2517 "y.tab.c" /* yacc.c:1646  */
    break;

  case 120:
#line 438 "parse.y" /* yacc.c:1646  */
    { (yyval.ctype) = ctype_short; }
#line 2523 "y.tab.c" /* yacc.c:1646  */
    break;

  case 121:
#line 439 "parse.y" /* yacc.c:1646  */
    { (yyval.ctype) = ctype_ushort; }
#line 2529 "y.tab.c" /* yacc.c:1646  */
    break;

  case 122:
#line 440 "parse.y" /* yacc.c:1646  */
    { (yyval.ctype) = ctype_ushort; }
#line 2535 "y.tab.c" /* yacc.c:1646  */
    break;

  case 123:
#line 441 "parse.y" /* yacc.c:1646  */
    { (yyval.ctype) = ctype_float ; }
#line 2541 "y.tab.c" /* yacc.c:1646  */
    break;

  case 124:
#line 442 "parse.y" /* yacc.c:1646  */
    { (yyval.ctype) = ctype_double; }
#line 2547 "y.tab.c" /* yacc.c:1646  */
    break;

  case 125:
#line 443 "parse.y" /* yacc.c:1646  */
    { (yyval.ctype) = ctype_void;   }
#line 2553 "y.tab.c" /* yacc.c:1646  */
    break;

  case 126:
#line 445 "parse.y" /* yacc.c:1646  */
    { (yyval.ctype) = duel_get_target_struct_tag((yyvsp[0].nameinfo).name);
		     if((yyval.ctype)==NULL) yyerror("not a struct tag"); }
#line 2560 "y.tab.c" /* yacc.c:1646  */
    break;

  case 127:
#line 448 "parse.y" /* yacc.c:1646  */
    { (yyval.ctype) = duel_get_target_union_tag((yyvsp[0].nameinfo).name);
		     if((yyval.ctype)==NULL) yyerror("not a union tag"); }
#line 2567 "y.tab.c" /* yacc.c:1646  */
    break;

  case 128:
#line 451 "parse.y" /* yacc.c:1646  */
    { (yyval.ctype) = duel_get_target_enum_tag((yyvsp[0].nameinfo).name);
		     if((yyval.ctype)==NULL) yyerror("not an enum tag"); }
#line 2574 "y.tab.c" /* yacc.c:1646  */
    break;


#line 2578 "y.tab.c" /* yacc.c:1646  */
      default: break;
    }
  /* User semantic actions sometimes alter yychar, and that requires
     that yytoken be updated with the new translation.  We take the
     approach of translating immediately before every use of yytoken.
     One alternative is translating here after every semantic action,
     but that translation would be missed if the semantic action invokes
     YYABORT, YYACCEPT, or YYERROR immediately after altering yychar or
     if it invokes YYBACKUP.  In the case of YYABORT or YYACCEPT, an
     incorrect destructor might then be invoked immediately.  In the
     case of YYERROR or YYBACKUP, subsequent parser actions might lead
     to an incorrect destructor call or verbose syntax error message
     before the lookahead is translated.  */
  YY_SYMBOL_PRINT ("-> $$ =", yyr1[yyn], &yyval, &yyloc);

  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);

  *++yyvsp = yyval;

  /* Now 'shift' the result of the reduction.  Determine what state
     that goes to, based on the state we popped back to and the rule
     number reduced by.  */

  yyn = yyr1[yyn];

  yystate = yypgoto[yyn - YYNTOKENS] + *yyssp;
  if (0 <= yystate && yystate <= YYLAST && yycheck[yystate] == *yyssp)
    yystate = yytable[yystate];
  else
    yystate = yydefgoto[yyn - YYNTOKENS];

  goto yynewstate;


/*--------------------------------------.
| yyerrlab -- here on detecting error.  |
`--------------------------------------*/
yyerrlab:
  /* Make sure we have latest lookahead translation.  See comments at
     user semantic actions for why this is necessary.  */
  yytoken = yychar == YYEMPTY ? YYEMPTY : YYTRANSLATE (yychar);

  /* If not already recovering from an error, report this error.  */
  if (!yyerrstatus)
    {
      ++yynerrs;
#if ! YYERROR_VERBOSE
      yyerror (YY_("syntax error"));
#else
# define YYSYNTAX_ERROR yysyntax_error (&yymsg_alloc, &yymsg, \
                                        yyssp, yytoken)
      {
        char const *yymsgp = YY_("syntax error");
        int yysyntax_error_status;
        yysyntax_error_status = YYSYNTAX_ERROR;
        if (yysyntax_error_status == 0)
          yymsgp = yymsg;
        else if (yysyntax_error_status == 1)
          {
            if (yymsg != yymsgbuf)
              YYSTACK_FREE (yymsg);
            yymsg = (char *) YYSTACK_ALLOC (yymsg_alloc);
            if (!yymsg)
              {
                yymsg = yymsgbuf;
                yymsg_alloc = sizeof yymsgbuf;
                yysyntax_error_status = 2;
              }
            else
              {
                yysyntax_error_status = YYSYNTAX_ERROR;
                yymsgp = yymsg;
              }
          }
        yyerror (yymsgp);
        if (yysyntax_error_status == 2)
          goto yyexhaustedlab;
      }
# undef YYSYNTAX_ERROR
#endif
    }



  if (yyerrstatus == 3)
    {
      /* If just tried and failed to reuse lookahead token after an
         error, discard it.  */

      if (yychar <= YYEOF)
        {
          /* Return failure if at end of input.  */
          if (yychar == YYEOF)
            YYABORT;
        }
      else
        {
          yydestruct ("Error: discarding",
                      yytoken, &yylval);
          yychar = YYEMPTY;
        }
    }

  /* Else will try to reuse lookahead token after shifting the error
     token.  */
  goto yyerrlab1;


/*---------------------------------------------------.
| yyerrorlab -- error raised explicitly by YYERROR.  |
`---------------------------------------------------*/
yyerrorlab:

  /* Pacify compilers like GCC when the user code never invokes
     YYERROR and the label yyerrorlab therefore never appears in user
     code.  */
  if (/*CONSTCOND*/ 0)
     goto yyerrorlab;

  /* Do not reclaim the symbols of the rule whose action triggered
     this YYERROR.  */
  YYPOPSTACK (yylen);
  yylen = 0;
  YY_STACK_PRINT (yyss, yyssp);
  yystate = *yyssp;
  goto yyerrlab1;


/*-------------------------------------------------------------.
| yyerrlab1 -- common code for both syntax error and YYERROR.  |
`-------------------------------------------------------------*/
yyerrlab1:
  yyerrstatus = 3;      /* Each real token shifted decrements this.  */

  for (;;)
    {
      yyn = yypact[yystate];
      if (!yypact_value_is_default (yyn))
        {
          yyn += YYTERROR;
          if (0 <= yyn && yyn <= YYLAST && yycheck[yyn] == YYTERROR)
            {
              yyn = yytable[yyn];
              if (0 < yyn)
                break;
            }
        }

      /* Pop the current state because it cannot handle the error token.  */
      if (yyssp == yyss)
        YYABORT;


      yydestruct ("Error: popping",
                  yystos[yystate], yyvsp);
      YYPOPSTACK (1);
      yystate = *yyssp;
      YY_STACK_PRINT (yyss, yyssp);
    }

  YY_IGNORE_MAYBE_UNINITIALIZED_BEGIN
  *++yyvsp = yylval;
  YY_IGNORE_MAYBE_UNINITIALIZED_END


  /* Shift the error token.  */
  YY_SYMBOL_PRINT ("Shifting", yystos[yyn], yyvsp, yylsp);

  yystate = yyn;
  goto yynewstate;


/*-------------------------------------.
| yyacceptlab -- YYACCEPT comes here.  |
`-------------------------------------*/
yyacceptlab:
  yyresult = 0;
  goto yyreturn;

/*-----------------------------------.
| yyabortlab -- YYABORT comes here.  |
`-----------------------------------*/
yyabortlab:
  yyresult = 1;
  goto yyreturn;

#if !defined yyoverflow || YYERROR_VERBOSE
/*-------------------------------------------------.
| yyexhaustedlab -- memory exhaustion comes here.  |
`-------------------------------------------------*/
yyexhaustedlab:
  yyerror (YY_("memory exhausted"));
  yyresult = 2;
  /* Fall through.  */
#endif

yyreturn:
  if (yychar != YYEMPTY)
    {
      /* Make sure we have latest lookahead translation.  See comments at
         user semantic actions for why this is necessary.  */
      yytoken = YYTRANSLATE (yychar);
      yydestruct ("Cleanup: discarding lookahead",
                  yytoken, &yylval);
    }
  /* Do not reclaim the symbols of the rule whose action triggered
     this YYABORT or YYACCEPT.  */
  YYPOPSTACK (yylen);
  YY_STACK_PRINT (yyss, yyssp);
  while (yyssp != yyss)
    {
      yydestruct ("Cleanup: popping",
                  yystos[*yyssp], yyvsp);
      YYPOPSTACK (1);
    }
#ifndef yyoverflow
  if (yyss != yyssa)
    YYSTACK_FREE (yyss);
#endif
#if YYERROR_VERBOSE
  if (yymsg != yymsgbuf)
    YYSTACK_FREE (yymsg);
#endif
  return yyresult;
}
#line 456 "parse.y" /* yacc.c:1906  */


static struct stoken {    /* all opcodes we recognize */
  char *opstr ;                 /* op code as a string      */
  int token ;                   /* token to return to yacc  */
  int opcode ;                  /* opcode value associated with the token */
 } tokens[] =  {                /* the special tokens, longer ones 1st! */
    {">>=",T_ASSIGN, OP_RSH},
    {"<<=",T_ASSIGN, OP_LSH},
    {"-->",T_DFS,    OP_DFS},
    {"->>",T_BFS,    OP_BFS},
    {"==?", T_EQQ,   OP_EQQ},
    {"!=?", T_NEQ,   OP_NEQ},
    {"<=?", T_LEQ,   OP_LEQ},
    {">=?", T_GEQ,   OP_GEQ},
    {"&&/", T_ANDL,  OP_AND},
    {"||/", T_ORL,   OP_OR},

    {"<?", T_LSQ,    OP_LSQ},
    {">?", T_GTQ,    OP_GTQ},
    {"#/", T_COUNT,  '#' },
    {"%/", T_COUNT,  '#' }, /* gdb insists to recognize # as start of comma!*/
    {"%%", '#',      '#' }, /* same. so %/ for #/ and %% for #. not doc!*/
    {"+=", T_ASSIGN,  '+'},
    {"-=", T_ASSIGN,  '-'},
    {"*=", T_ASSIGN,  '*'},
    {"/=", T_ASSIGN,  '/'},
    {"%=", T_ASSIGN,  '%'},
    {"|=", T_ASSIGN,  '|'},
    {"&=", T_ASSIGN,  '&'},
    {"^=", T_ASSIGN,  '^'},
    {":=", T_DEFVAR,OP_DEF},
    {"++", T_INC,   OP_INC },
    {"--", T_DEC,   OP_DEC },
    {"->", T_ARROW, OP_ARR },
    {"&&", T_AND,   OP_AND },
    {"||", T_OR,    OP_OR  },
    {"<<", T_LSH,   OP_LSH },
    {">>", T_RSH,   OP_RSH },
    {"==", T_EQ,    OP_EQ  },
    {"!=", T_NE,    OP_NE  },
    {"<=", T_LE,    OP_LE  },
    {">=", T_GE,    OP_GE  },
    {"..", T_TO,    OP_TO  },
    {"=>", T_IMP,   OP_IMP },
    {"[[", T_OSEL,  OP_SEL },
    {"]]", T_CSEL,  OP_SEL },
  };

static struct skeyword {  /* all keywords we recognize */
  char *keyword_str ;           /* keyword as a string       */
  int token ;                   /* token to return to yacc   */
  topcode opcode ;              /* opcode associated w/keyword */
 } keywords[] = {
    {"if",	T_IF	   , OP_IF},
    {"else",	T_ELSE	   },
    {"for",	T_FOR	   , OP_FOR},
    {"while",	T_WHILE	   , OP_WHILE},
    {"sizeof",  T_SIZEOF   , OP_SIZ},
    {"frame",	T_FRAME	   , OP_FRAME},

    {"T", 	T_TYPEDEF_INDICATOR  },
    {"struct",  T_STRUCT   },
    {"union",   T_UNION    },
    {"enum",    T_ENUM     },

    {"unsigned",T_UNSIGNED },
    {"signed",  T_SIGNED   },
    {"short",   T_SHORT    },
    {"long",    T_LONG     },
    {"char",    T_CHAR     },
    {"int",     T_INT      },
    {"double",  T_DOUBLE   },
    {"float",   T_FLOAT    },
    {"void",	T_VOID	   },
   } ;


LFUNC tnode* duel_lex_int(void)    /* parse next token as integer num */
{
   tnode *n ;
   tulonglong val=0 ;
   char *p=lexptr ;
   bool is_l=0,is_u=0 ;
   int base=10 ;
   int src_pos=lexptr-inputstr ;

   if(*p=='0') {                        /* figure out the base */
      p++ ;
      if(*p=='x' || *p=='X') base=16,p++ ;
      else
      if(isdigit(*p)) base=8 ; /* avoid having '0' as a base 8 (uint) */
   }

   while(isdigit(*p) || base==16 && isxdigit(*p)) {  /* get the value */
      val*=base ;
      if(isupper(*p)) val+= *p-'A'+10 ;
      else if(islower(*p)) val+= *p-'a'+10 ;
      else val+= *p-'0' ;
      p++ ;
   }
   for (;*p;p++) {          /* yuk. figure 0L etc */
     if (*p == 'l' || *p == 'L') is_l++;
     else if(*p == 'u' || *p == 'U') is_u++;
     else break;
   }
   is_u=is_u || base!=10 ;

   if((is_l>1 && is_u) || (long long) val < 0 || ((tulong) val != val && is_u)) {
        n=mknode_const(src_pos,ctype_ulonglong);
        n->cnst.u.rval_ulonglong=val ;
   }
   else
   if(is_l>1 || (tulong) val != val) {
        n=mknode_const(src_pos,ctype_longlong) ;
        n->cnst.u.rval_longlong=(long long) val ;
   }
   else
   if((is_l && is_u) || (long) val < 0 || ((tuint) val != val && is_u)) {
        n=mknode_const(src_pos,ctype_ulong);
        n->cnst.u.rval_ulong=val ;
   }
   else
   if(is_l || (tuint) val != val) {
        n=mknode_const(src_pos,ctype_long) ;
        n->cnst.u.rval_long=(long) val ;
   }
   else
   if(is_u || (int) val < 0) {
        n=mknode_const(src_pos,ctype_uint) ;
        n->cnst.u.rval_uint=(tuint) val ;
   }
   else {
        n=mknode_const(src_pos,ctype_int) ;
        n->cnst.u.rval_int=(int) val ;
   }
   strncpyz(n->cnst.symb_val,lexptr,p-lexptr); /* save the symbolic val*/
   lexptr=p ;
   return n ;
}

LFUNC tnode* duel_lex_float(void)    /* parse next token as float num */
{
  tnode *n=0 ;
  char *p=lexptr ;
  double val ;
  char c,tmpc ;
  bool ok=TRUE;
  int src_pos = lexptr - inputstr ;

  /* this is disgusting.. why isnt there a lib call to recognize floats?! */
  while(isdigit(*p)) p++ ;
  if(*p=='.') p++ ;
  while(isdigit(*p)) p++ ;
  if(*p=='e' || *p=='E') {
     p++ ;
     if(*p=='+' || *p=='-') p++ ;
     if(!isdigit(*p)) ok=FALSE ;     /* force digit (scanf allows 1e-.2 ?!) */
     while(isdigit(*p)) p++ ;
  }
  tmpc= *p ; *p=0 ;
  ok=ok && sscanf(lexptr,"%lf%c",&val,&c)==1 ;
  *p=tmpc ;
  if(!ok) yyerror("Invalid float constant.");

  n=mknode_const(src_pos,ctype_double);
  n->cnst.u.rval_double=val ;
  strncpyz(n->cnst.symb_val,lexptr,p-lexptr); /* save the symbolic val*/
  lexptr=p ;
  return(n);
}

/* parse_escaped_char -- parse an escaped char (e.g. '\n').
 * lexptr expected to point to text right after the '\'.
 * return: actual char value (e.g. 012 if 'n' or '012' is found.)
 *         lexptr is advanced after the espaced char.
 */

LFUNC char parse_escaped_char(void)
{
  char retc ;
  switch(lexptr[0]) {
   /*case 'a': retc='\a' ; break ;	/* some compilers don't support it. */
   case 'b': retc='\b' ; break ;
   case 'f': retc='\f' ; break ;
   case 'n': retc='\n' ; break ;
   case 'r': retc='\r' ; break ;
   case 't': retc='\t' ; break ;
   case 'v': retc='\v' ; break ;
   case 'x': yyerror("hex char const not yet suppported");
   case '0': case '1': case '2': case '3':
	     retc= lexptr[0] - '0' ;
	     if(lexptr[1]>='0' && lexptr[1]<='7')
		retc= retc* 010 +  *++lexptr - '0' ;
	     if(lexptr[1]>='0' && lexptr[1]<='7')
		retc= retc* 010 +  *++lexptr - '0' ;
             break ;
   default:  retc=lexptr[0] ;     /* default also takes care of '\'' '\\' */
  }
  lexptr++ ;
  return retc ;
}

/* FUNC yylex -- return the next token to yacc.
 * GLOBALS: lexptr point to the string we are parsing next. it is updated.
 */

LFUNC int yylex (void)
{
  int c,i,src_pos ;
  char *p ;

  for(c= *lexptr; c==' ' || c=='\t' || c=='\n' ; c= *++lexptr); /* skip blank*/

  src_pos = lexptr - inputstr ;	/* current char being parsed */
  yylval.opinfo.src_pos = src_pos ;

  if(*lexptr=='\0' || strncmp(lexptr,"|>",2)==0) return 0 ; /* end of expr */

  for (i = 0;  i < sizeof(tokens)/sizeof(struct stoken) ; i++) {
    int l=strlen(tokens[i].opstr) ;             /* check next token vs table */
    if(strncmp(lexptr,tokens[i].opstr,l)==0) {
	lexptr+=l ;
	yylval.opinfo.opcode = tokens[i].opcode;
	return tokens[i].token ;
    }
  }

  switch (c = *lexptr) {
    case '\'':                /* char constant, but stored as int (ansi-c) */
      p=lexptr++ ;
      c = *lexptr++ ;
      if (c == '\\') c=parse_escaped_char();
      if( *lexptr++ != '\'') yyerror("Invalid character constant.");
      yylval.node=mknode_const(src_pos,ctype_int) ;
      yylval.node->cnst.u.rval_int=c ;
      strncpyz(yylval.node->cnst.symb_val,p,lexptr-p); /*save the symbol. val*/
      return T_CONST ;

    case '0':                           /* chk hex  */
        if(lexptr[1]=='x' || lexptr[1]=='X') {
           yylval.node=duel_lex_int();
           return T_CONST ;
        }
        /* fall thru for other numbers */
    case '1': case '2': case '3':      /* decimal or floating point number */
    case '4': case '5': case '6': case '7': case '8': case '9':
          for(p=lexptr ; *p>='0' && *p<='9' ; p++ ) ;  /*find next non digit*/
          if(*p=='.' && p[1]!='.' || *p=='e' || *p=='E')
               yylval.node=duel_lex_float();
          else yylval.node=duel_lex_int();
          return T_CONST ;

    case '(':  case ')':
    case '<':  case '>':
    case '[':  case ']':
    case '{':  case '}':
    case '+':  case '-':  case '*':  case '/':  case '%':
    case '|':  case '&':  case '^':  case '~':  case '!':
    case ',':  case '?':  case ':':  case '=':
    case '.':  case '@':  case '$':  case '#':  case '`': case '\\':
      lexptr++;
      yylval.opinfo.opcode=c ;
      return c;
    case ';': { /* hack, ignore ';' before '}' and else. for C compatability*/
	        char *save_lexptr= ++lexptr ;
		int tok=yylex()	;	/* hack, call myself for next token */
		if(tok=='}' || tok==T_ELSE) {
		    duel_printf("warning: useless ';' ignored\n");
		    return tok ;
		}
		/* else restore position and return the ';' */
		lexptr=save_lexptr ;
		yylval.opinfo.opcode=';' ;
		yylval.opinfo.src_pos = src_pos ;
		return ';';
    }
    case '"': {
          char s[512] ;
	  size_t len=0 ;
	  ttarget_ptr dptr ;
	  tnode *n ;

	  p=lexptr++ ;
	  while((c= *lexptr++)!='"') {
	       if (c == '\\') c=parse_escaped_char();
	       s[len++]=c ;
	  }
	  s[len++]=0 ;
	  dptr=duel_alloc_target_space(len);
	  duel_put_target_bytes(dptr,s,len);

	  n=mknode_const(src_pos,ctype_charptr);
	  n->cnst.u.rval_ptr=dptr ;
	  len=lexptr-p ;
	  if(len>60) len=60 ;
	  strncpyz(n->cnst.symb_val,p,len); /* save the symbolic val*/
          yylval.node=n ;
          return T_CONST ;
      }
    }

  if(c != '_' && !isalpha(c))
     yyerror ("Invalid character in expression.");

  p=lexptr ;
  do { c= *++lexptr ; } while(c=='_' || isalnum(c));

  for (i = 0;  i < sizeof(keywords)/sizeof(struct skeyword) ; i++) {
    int l=strlen(keywords[i].keyword_str) ;   /* check next token vs keywords*/
    if(l==lexptr-p && strncmp(p,keywords[i].keyword_str,l)==0) {
        yylval.opinfo.opcode=keywords[i].opcode ;
	return keywords[i].token ;
    }
  }

  /* the symbol/name found is not a reserved word, so return it as a T_SYM
   */

  i=lexptr-p ;          /* length of string found (symbol/name) */
  yylval.nameinfo.src_pos=src_pos ;
  yylval.nameinfo.name=duel_malloc(i+1);
  strncpyz(yylval.nameinfo.name,p,i);
  return T_SYM;
}

LPROC yyerror(char *msg)
{
  int i,n=lexptr-inputstr ;
  duel_printf("%s\n",inputstr);
  for(i=0 ; i<n ; i++) duel_printf("-");
  duel_printf("^ %s\n",msg);
  duel_abort();		/* terminate parsing. some callers depend on this*/
}

/*************************************************************************/
/* utility functions used to parse the expression and build it as a tree */
/*************************************************************************/

/* mknode_op -- make a tree node of type op with given opcode and kids
 */

LFUNC tnode* mknode_op(top_kind op_kind,topinfo opinfo,
                       tnode *k1,tnode *k2,tnode *k3,tnode *k4)
{
   tnode *n ;
   duel_assert(opinfo.opcode>' ');
   n=(tnode *) duel_malloc(sizeof(tnode));
   duel_bzero((char*) n,sizeof(tnode));
   n->node_kind=NK_OP ;
   n->op_kind=op_kind ;
   n->op=opinfo.opcode ;
   n->src_pos=opinfo.src_pos ;
   n->kids[0]=k1 ;  n->kids[1]=k2 ;  n->kids[2]=k3 ; n->kids[3]=k4 ;
   return n ;
}


 /* mknode_const -- make a constant node for the given type.
  */

LFUNC tnode* mknode_const(int src_pos,tctype *ctype)
{
   tnode *n ;
   n=(tnode *) duel_malloc(sizeof(tnode));
   duel_bzero((char*) n,sizeof(tnode));
   n->node_kind=NK_CONST ;
   n->src_pos=src_pos ;
   n->cnst.val_kind=VK_RVALUE ;
   n->cnst.ctype=ctype ;
   return n ;
}

 /* mknode_ctype -- make a node of the given c-type.
  */

LFUNC tnode* mknode_ctype(tctype *ctype)
{
   tnode *n ;
   n=(tnode *) duel_malloc(sizeof(tnode));
   duel_bzero((char*) n,sizeof(tnode));
   n->node_kind=NK_CTYPE ;
   n->ctype=ctype ;
   return n ;
}

 /* mknode_name -- make a node of the given name/symbol.
  * input is pointer to the saved name (on heap)
  */

LFUNC tnode* mknode_name(tnameinfo nameinfo)
{
   tnode *n ;
   n=(tnode *) duel_malloc(sizeof(tnode));
   duel_bzero((char*) n,sizeof(tnode));
   n->node_kind=NK_NAME ;
   n->name=nameinfo.name ;
   n->src_pos=nameinfo.src_pos ;
   return n ;
}

/* In order to parse C types, which are 'reversed' in the parser, a stack
 * is used to push abstract declarators, e.g. in (*)() we first push a func
 * indicator '(' and then push a pointer indicator '*'. for arrays we push
 * a '[' and the array size.
 * This stack is popped and a ctype is constructed at the end of the
 * abstract type parsing. The following functions implement the stack
 */

typedef struct stype_desc {  /* stack of type descriptors is made of these */
        char desc ;
        int size ;
        struct stype_desc *next ;       /* next on stack */
      } ttype_desc ;

ttype_desc *top = 0 ;


LPROC push_type(char desc)     /* put desc on the types stack */
{
    ttype_desc *p = (ttype_desc* ) duel_malloc(sizeof(ttype_desc));
    p->desc=desc ;
    p->size=0 ;
    p->next=top ;
    top=p ;
}

/* push_type_int -- same as push_type but also set the size parameter, which
 *                  is given as a constant node (which is expected to be int)
 */

LPROC push_type_int(char desc,tnode *n)
{
   duel_assert(n->node_kind==NK_CONST);
   if(n->cnst.ctype != ctype_int ||
      n->cnst.u.rval_int <=0 ) duel_gen_error("Illegal array size",0);
   push_type(desc);
   top->size=n->cnst.u.rval_int ;
}

LFUNC bool pop_type(char *desc,int *size)  /* pop item from stack. */
{
   ttype_desc *p = top ;
   if(p==0) return FALSE ;
   *desc=p->desc ;
   *size=p->size ;
   top=p->next ;
   duel_free(p) ;
   return TRUE ;
}


/* abstract type-modifiers were pushed on a stack. Retrieve
 * them (reversed) creating type nodes as we go
 * input: base type (e.g. 'long').
 * returns: node of the modified type.
 * modification is based on the stack of things pushed while parsing.
 */

LFUNC tnode* mknode_modified_ctype(tctype *base)
{
    int size;
    char tdesc ;           /* descriptor of abs decl eg '*' */
    tctype *t=base ;       /* type under construction       */

    while(pop_type(&tdesc,&size))    /* pop next abs decl    */
	switch (tdesc) {
	  case '*':  t=duel_mkctype_ptr(t);         break ;
	  case '(':  t=duel_mkctype_func(t);        break ;
	  case '[':  t=duel_mkctype_array(t,size);  break ;
	}
    return mknode_ctype(t) ;
}

/* entry point for parsing. the given expression is parsed into the given
 * node as root.
 */

FUNC tnode* duel_parse(char *s)
{
  lexptr=inputstr=s ;
  top=0 ; 				/* reset the types stack */
  if(duel_yyparse()) root=NULL ;
  return root ;
}
