typedef union {
    int		ival;
    double	dval;
    char	*sval;
    XftExpr	*eval;
    XftPattern	*pval;
    XftValue	vval;
    XftEdit	*Eval;
    XftOp	oval;
    XftQual	qval;
    XftTest	*tval;
} YYSTYPE;
#define	INTEGER	257
#define	DOUBLE	258
#define	STRING	259
#define	NAME	260
#define	ANY	261
#define	ALL	262
#define	DIR	263
#define	INCLUDE	264
#define	INCLUDEIF	265
#define	MATCH	266
#define	EDIT	267
#define	TOK_TRUE	268
#define	TOK_FALSE	269
#define	TOK_NIL	270
#define	EQUAL	271
#define	SEMI	272
#define	QUEST	273
#define	COLON	274
#define	OROR	275
#define	ANDAND	276
#define	EQEQ	277
#define	NOTEQ	278
#define	LESS	279
#define	LESSEQ	280
#define	MORE	281
#define	MOREEQ	282
#define	PLUS	283
#define	MINUS	284
#define	TIMES	285
#define	DIVIDE	286
#define	NOT	287


extern YYSTYPE KfiXftConfiglval;
